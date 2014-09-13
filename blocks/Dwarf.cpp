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

#include "Dwarf.h"
#include "World.h"
#include "Shred.h"
#include "BlockManager.h"
#include <QDataStream>

int Dwarf::Weight() const {
    World * const world = GetWorld();
    const int bound = world->GetBound();
    return ( (X() < bound && world->GetBlock(X()+1, Y(), Z())->Catchable()) ||
            ( X() > 0     && world->GetBlock(X()-1, Y(), Z())->Catchable()) ||
            ( Y() < bound && world->GetBlock(X(), Y()+1, Z())->Catchable()) ||
            ( Y() > 0     && world->GetBlock(X(), Y()-1, Z())->Catchable()) ) ?
        0 : Inventory::Weight()+Block::Weight();
}

Block * Dwarf::DropAfterDamage(bool * const delete_block) {
    Block * const cadaver = Animal::DropAfterDamage(delete_block);
    cadaver->HasInventory()->Get(BlockManager::NewBlock(WEAPON, BONE));
    return cadaver;
}

int  Dwarf::ShouldAct() const { return FREQUENT_FIRST | FREQUENT_RARE; }
int  Dwarf::Start() const { return IN_LEFT + 1; }
int  Dwarf::LightRadius() const { return lightRadius; }
bool Dwarf::Access() const { return false; }
Inventory * Dwarf::HasInventory() { return this; }
void Dwarf::ReceiveSignal(const QString str) { Active::ReceiveSignal(str); }

QString Dwarf::FullName() const {
    return ( DIFFERENT == Sub() ) ?
        tr("Creator") :
        tr("Rational creature");
}

void Dwarf::UpdateLightRadius() {
    Block * const in_left  = ShowBlock(IN_LEFT);
    Block * const in_right = ShowBlock(IN_RIGHT);
    const int left_rad  = in_left  ? in_left ->LightRadius() : 0;
    const int right_rad = in_right ? in_right->LightRadius() : 0;
    lightRadius = qMax(MIN_DWARF_LIGHT_RADIUS, qMax(left_rad, right_rad));
}

int Dwarf::DamageKind() const {
    return ( DIFFERENT == Sub() ) ?
        DAMAGE_ULTIMATE : ( Number(IN_RIGHT) ) ?
            ShowBlock(IN_RIGHT)->DamageKind() : DAMAGE_HANDS;
}

int Dwarf::DamageLevel() const {
    int level = 1;
    if ( Number(IN_RIGHT) ) {
        level += ShowBlock(IN_RIGHT)->DamageLevel();
    }
    if ( Number(IN_LEFT) ) {
        level += ShowBlock(IN_LEFT)->DamageLevel();
    }
    return level;
}

void Dwarf::Damage(const int dmg, const int dmg_kind) {
    if ( dmg_kind >= DAMAGE_PUSH_UP ) {
        Active::Damage(dmg, DAMAGE_PUSH_UP);
        return;
    }
    int damage_to_self = dmg;
    const int places[] = { ON_HEAD, ON_BODY, ON_LEGS };
    for (const int i : places) {
        if ( Number(i) == 0 ) continue;
        Block * const armour = ShowBlock(i);
        const int dur_before_damage = armour->GetDurability();
        const int damage_divider = (i == ON_BODY) ? 2 : 4;
        armour->Damage(dmg/damage_divider, dmg_kind);
        if ( armour->GetDurability() < dur_before_damage ) {
            damage_to_self -= dmg/damage_divider;
            if ( armour->GetDurability() <= 0 ) {
                delete armour;
                Pull(i);
            }
        }
    }
    Active::Damage(damage_to_self, dmg_kind);
}

void Dwarf::Move(const dirs dir) {
    Shred * const last_shred = GetShred();
    Falling::Move(dir);
    if ( last_shred != GetShred() ) {
        for (int i=0; i<Size();    ++i)
        for (int j=0; j<Number(i); ++j) {
            ShowBlockInSlot(i, j)->UseOnShredMove(this);
        }
    }
}

int Dwarf::NutritionalValue(const subs sub) const {
    switch ( sub ) {
    case HAZELNUT: return SECONDS_IN_HOUR/2;
    case H_MEAT:   return SECONDS_IN_HOUR*2.5f;
    case A_MEAT:   return SECONDS_IN_HOUR*2;
    default:       return 0;
    }
}

bool Dwarf::GetExact(Block * const block, const int to) {
    if ( block==nullptr ) return true;
    if ( (to > IN_LEFT || ( Number(to) == 0 && (
                     IN_RIGHT==to
                ||   IN_LEFT ==to
                || ( ON_HEAD ==to && WEARABLE_HEAD==block->Wearable() )
                || ( ON_BODY ==to && WEARABLE_BODY==block->Wearable() )
                || ( ON_LEGS ==to && WEARABLE_LEGS==block->Wearable() ))))
            && Inventory::GetExact(block, to) )
    {
        UpdateLightRadius();
        if ( lightRadius == 0 ) {
            GetWorld()->GetShred(X(), Y())->RemShining(this);
        } else {
            GetWorld()->GetShred(X(), Y())->AddShining(this);
            GetWorld()->Shine(X(), Y(), Z(), lightRadius, true);
        }
        return true;
    } else {
        return false;
    }
}

void Dwarf::SaveAttributes(QDataStream & out) const {
    Animal::SaveAttributes(out);
    Inventory::SaveAttributes(out);
}

bool Dwarf::Inscribe(QString) {
    SendSignalAround(tr("Don't touch me!"));
    return false;
}

QString Dwarf::InvFullName(const int slot_number) const {
    if ( Number(slot_number) == 0 ) {
        switch ( slot_number ) {
        case ON_HEAD:  return tr("-head-");
        case ON_BODY:  return tr("-body-");
        case ON_LEGS:  return tr("-legs-");
        case IN_RIGHT: return tr("-right hand-");
        case IN_LEFT:  return tr("-left  hand-");
        }
    }
    return Inventory::InvFullName(slot_number);
}

Dwarf::Dwarf(const int kind, const int sub) :
        Animal(kind, sub),
        Inventory(),
        lightRadius(MIN_DWARF_LIGHT_RADIUS)
{
    Block::Inscribe("Urist");
}

Dwarf::Dwarf(QDataStream & str, const int kind, const int sub) :
        Animal(str, kind, sub),
        Inventory(str),
        lightRadius()
{
    UpdateLightRadius();
}
