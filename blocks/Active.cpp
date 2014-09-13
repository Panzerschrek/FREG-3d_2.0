    /* freg, Free-Roaming Elementary Game with open and interactive world
    *  Copyright (C) 2012-2014 Alexander 'mmaulwurff' Kromm
    *  mmaulwurff@gmail.com
    *
    * This file is part of FREG.
    *
    * FREG is free software: you can redistribute it and/or modify
    * it under the terms of the GNU General Public License as published by
    * the Free Software Foundation, either version 3 of the License, or
    * (at your option) any later version.
    *
    * FREG is distributed in the hope that it will be useful,
    * but WITHOUT ANY WARRANTY; without even the implied warranty of
    * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    * GNU General Public License for more details.
    *
    * You should have received a copy of the GNU General Public License
    * along with FREG. If not, see <http://www.gnu.org/licenses/>. */

#include "Active.h"
#include "Shred.h"
#include "World.h"
#include "Xyz.h"
#include "blocks/Inventory.h"

// Active section

Active * Active::ActiveBlock() { return this; }
int  Active::ShouldAct() const { return FREQUENT_NEVER; }
inner_actions Active::ActInner() { return INNER_ACTION_ONLY; }

void Active::ActFrequent() {
    fprintf(stderr, "Active::ActFrequent called, check ShouldAct\
 return value or add ActFrequent implementation.\nKind: %d, sub: %d.\n",
        Kind(), Sub());
    Q_UNREACHABLE();
}

void Active::DoRareAction() {
    fprintf(stderr, "Active::DoRareAction called, check ShouldAct and ActInner\
 return values or add DoRareAction implementation.\nKind: %d, sub: %d.\n",
        Kind(), Sub());
    Q_UNREACHABLE();
}

void Active::ActRare() {
    Inventory * const inv = HasInventory();
    if ( inv == nullptr ) {
        DoRareAction();
        return;
    } // else:
    for (int i=inv->Size()-1; i; --i) {
        const int number = inv->Number(i);
        if ( number == 0 ) continue;
        Active * const top_active = inv->ShowBlock(i)->ActiveBlock();
        if ( top_active == nullptr ) continue;
        for (int j=0; j<number; ++j) {
            Active * const active = inv->ShowBlockInSlot(i, j)->ActiveBlock();
            if ( active->ActInner() == INNER_ACTION_MESSAGE ) {
                ReceiveSignal( tr("%1 in slot '%2': %3").
                    arg(inv->InvFullName(i)).
                    arg(char('a'+i)).
                    arg(inv->ShowBlockInSlot(i, j)->GetNote()) );
            }
        }
    }
    DoRareAction();
}

void Active::Unregister() {
    if ( shred != nullptr ) {
        shred->Unregister(this);
        shred = nullptr;
    }
}

void Active::Move(const dirs dir) {
    switch ( dir ) {
    case UP:    ++z_self; break;
    case DOWN:  --z_self; break;
    case NORTH: --y_self; break;
    case SOUTH: ++y_self; break;
    case EAST:  ++x_self; break;
    case WEST:  --x_self; break;
    }
    emit Moved(dir);
    if ( dir > DOWN ) {
        Shred * const new_shred = GetWorld()->GetShred(X(), Y());
        if ( shred != new_shred ) {
            shred->Unregister(this);
            new_shred->Register(this);
        }
    }
}

void Active::SendSignalAround(const QString signal) const {
    if ( shred == nullptr ) return; // for blocks inside inventories
    World * const world = GetWorld();
    const int bound = world->GetBound();
    if ( X() > 0 )     world->GetBlock(X()-1, Y(), Z())->ReceiveSignal(signal);
    if ( X() < bound ) world->GetBlock(X()+1, Y(), Z())->ReceiveSignal(signal);
    if ( Y() > 0 )     world->GetBlock(X(), Y()-1, Z())->ReceiveSignal(signal);
    if ( Y() < bound ) world->GetBlock(X(), Y()+1, Z())->ReceiveSignal(signal);
    world->GetBlock(X(), Y(), Z()-1)->ReceiveSignal(signal);
    world->GetBlock(X(), Y(), Z()+1)->ReceiveSignal(signal);
}

void Active::DamageAround() const {
    const int bound = GetWorld()->GetBound();
    int x_temp = X()-1;
    int y_temp = Y();
    int z_temp = Z();
    if (   x_temp     >= 0     ) TryDestroy(  x_temp, y_temp, z_temp);
    if (  (x_temp+=2) <= bound ) TryDestroy(  x_temp, y_temp, z_temp);
    if ( --y_temp     >= 0     ) TryDestroy(--x_temp, y_temp, z_temp);
    if (  (y_temp+=2) <= bound ) TryDestroy(  x_temp, y_temp, z_temp);
    TryDestroy(x_temp, --y_temp, --z_temp);
    TryDestroy(x_temp,   y_temp,   z_temp+=2);
}

void Active::TryDestroy(const int x, const int y, const int z) const {
    World * const world = GetWorld();
    if ( world->Damage(x, y, z, DamageLevel(), DamageKind()) <= 0 ) {
        world->DestroyAndReplace(x, y, z);
    }
}

void Active::SetShred(Shred * const new_shred) { shred = new_shred; }
Shred * Active::GetShred() const { return shred; }
World * Active::GetWorld() const { return world; }

void Active::Damage(const int dmg, const int dmg_kind) {
    const int last_dur = GetDurability();
    Block::Damage(dmg, dmg_kind);
    if ( last_dur != GetDurability() ) {
        switch ( dmg_kind ) {
        case DAMAGE_HUNGER: ReceiveSignal(tr("You weaken from hunger!"));break;
        case DAMAGE_HEAT:   ReceiveSignal(tr("You burn!"));              break;
        case DAMAGE_BREATH: ReceiveSignal(tr("You choke withot air!"));  break;
        default:            ReceiveSignal(tr("Received damage!"));
        }
        emit Updated();
    }
}

void Active::ReceiveSignal(const QString str) { emit ReceivedText(str); }

void Active::ReloadToNorth() { y_self += SHRED_WIDTH; }
void Active::ReloadToSouth() { y_self -= SHRED_WIDTH; }
void Active::ReloadToWest()  { x_self += SHRED_WIDTH; }
void Active::ReloadToEast()  { x_self -= SHRED_WIDTH; }

Active::Active(const int kind, const int sub, const int transp) :
        Block(kind, sub, transp),
        Xyz()
{}

Active::Active(QDataStream & str, const int kind, const int sub,
        const int transp)
    :
        Block(str, kind, sub, transp),
        Xyz()
{}

bool Active::Gravitate(const int range, int bottom, int top,
        const int calmness)
{
    World * const world = GetWorld();
    const int bound = world->GetBound();
    // analyse world around
    int for_north = 0, for_west = 0;
    const int y_start = qMax(Y()-range, 0);
    const int y_end   = qMin(Y()+range, bound);
    const int x_end   = qMin(X()+range, bound);
    bottom = qMax(Z()-bottom,  0);
    top    = qMin(Z()+top, HEIGHT-1);
    for (int x=qMax(X()-range, 0); x<=x_end; ++x)
    for (int y=y_start; y<=y_end; ++y) {
        Shred * const shred = world->GetShred(x, y);
        const int x_in = Shred::CoordInShred(x);
        const int y_in = Shred::CoordInShred(y);
        for (int z=bottom; z<=top; ++z) {
            const int attractive =
                Attractive(shred->GetBlock(x_in, y_in, z)->Sub());
            if ( attractive != 0
                    && world->DirectlyVisible(X(), Y(), Z(), x, y, z) )
            {
                if ( y!=Y() ) for_north += attractive/(Y()-y);
                if ( x!=X() ) for_west  += attractive/(X()-x);
            }
        }
    }
    // make direction and move there
    if ( qAbs(for_north)>calmness || qAbs(for_west)>calmness ) {
        SetDir( ( qAbs(for_north)>qAbs(for_west) ) ?
            ( ( for_north>0 ) ? NORTH : SOUTH ) :
            ( ( for_west >0 ) ? WEST  : EAST  ) );
        return true;
    } else {
        return false;
    }
}

int Active::Attractive(int) const { return 0; }

bool Active::IsSubAround(const int sub) const {
    const World * const world = GetWorld();
    const int bound = world->GetBound();
    return (sub == world->GetBlock(X(), Y(), Z()-1)->Sub() ||
            sub == world->GetBlock(X(), Y(), Z()+1)->Sub() ||
            (X() > 0     && sub == world->GetBlock(X()-1, Y(), Z())->Sub()) ||
            (X() < bound && sub == world->GetBlock(X()+1, Y(), Z())->Sub()) ||
            (Y() > 0     && sub == world->GetBlock(X(), Y()-1, Z())->Sub()) ||
            (Y() < bound && sub == world->GetBlock(X(), Y()+1, Z())->Sub()) );
}

// Falling section

Falling::Falling(const int kind, const int sub, const int transp) :
        Active(kind, sub, transp),
        fallHeight(0),
        falling(false)
{}

Falling::Falling(QDataStream & str, const int kind, const int sub,
        const int transp)
    :
        Active(str, kind, sub, transp),
        fallHeight(),
        falling()
{
    str >> fallHeight >> falling;
}

void Falling::SaveAttributes(QDataStream & out) const {
    out << fallHeight << falling;
}

QString Falling::FullName() const {
    switch ( Sub() ) {
    default:    return SubNameUpper(Sub());
    case WATER: return tr("Snow");
    case STONE: return tr("Masonry");
    }
}

bool Falling::IsFalling() const { return falling; }
Falling * Falling::ShouldFall() { return this; }
push_reaction Falling::PushResult(dirs) const { return MOVABLE; }

void Falling::FallDamage() {
    const int SAFE_FALL_HEIGHT = 5;
    if ( fallHeight > SAFE_FALL_HEIGHT ) {
        const int dmg = (fallHeight - SAFE_FALL_HEIGHT)*10;
        World * const world = GetWorld();
        Block * const block_under = world->GetBlock(X(), Y(), Z()-1);
        world->Damage(X(), Y(), Z()-1, dmg, DamageKind());
        if ( block_under->GetDurability() <= 0 ) {
            world->DestroyAndReplace(X(), Y(), Z()-1);
        }
        Damage(dmg, block_under->DamageKind());
        if ( GetDurability() <= 0 ) {
            world->DestroyAndReplace(X(), Y(), Z());
            return;
        }
    }
    falling = false;
    fallHeight = 0;
}

void Falling::Move(const dirs dir) {
    Active::Move(dir);
    if ( DOWN == dir ) {
        ++fallHeight;
    }
}

void Falling::SetFalling(const bool set) {
    if ( not (falling=set) ) {
        fallHeight = 0;
    }
}
