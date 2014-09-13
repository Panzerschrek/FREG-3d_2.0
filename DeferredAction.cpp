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

#include "World.h"
#include "blocks/Animal.h"
#include "blocks/Inventory.h"
#include "DeferredAction.h"
#include "BlockManager.h"

void DeferredAction::GhostMove() const {
    const dirs dir = static_cast<dirs>(num);
    if ( dir == DOWN && attachedBlock->Z() == 1        ) return;
    if ( dir == UP   && attachedBlock->Z() == HEIGHT-2 ) return;
    attachedBlock->Move(dir);
}

void DeferredAction::Move() const {
    attachedBlock->GetWorld()->Move(attachedBlock->X(), attachedBlock->Y(),
        attachedBlock->Z(), static_cast<dirs>(num));
}

void DeferredAction::Jump() const {
    attachedBlock->GetWorld()->Jump(
        attachedBlock->X(), attachedBlock->Y(), attachedBlock->Z(),
        attachedBlock->GetDir());
}

void DeferredAction::Build() const {
    int z = Z();
    World * const world = attachedBlock->GetWorld();
    if ( DOWN==attachedBlock->GetDir() &&
            AIR!=world->GetBlock(X(), Y(), Z())->Sub() )
    {
        if ( world->Move(attachedBlock->X(),
                attachedBlock->Y(), attachedBlock->Z(), UP) )
        {
            ++z;
        } else {
            return;
        }
    }
    const int id = material->GetId();
    if ( not world->Build(material, X(), Y(), z,
            World::TurnRight(attachedBlock->GetDir()), attachedBlock) )
    { // build not successful
        return;
    } // else:
    Inventory * const inv = attachedBlock->HasInventory();
    if ( inv == nullptr ) return;
    inv->Pull(srcSlot);
    // put more material in building inventory slot:
    if ( inv->Number(srcSlot) ) return;
    for (int i=srcSlot+1; i<inv->Size() &&
        inv->Number(srcSlot)<MAX_STACK_SIZE; ++i)
    {
        const Block * const block_i = inv->ShowBlock(i);
        if ( block_i && id==block_i->GetId() ) {
            inv->MoveInside(i, srcSlot, inv->Number(i));
        }
    }
} // void DeferredAction::Build()

void DeferredAction::Damage() const {
    if ( attachedBlock->GetWorld()->Damage(X(), Y(), Z(),
            attachedBlock->DamageLevel(),
            attachedBlock->DamageKind()) <= 0 ) // durability
    {
        attachedBlock->GetWorld()->DestroyAndReplace(X(), Y(), Z());
    }
}

void DeferredAction::Throw() const {
    attachedBlock->GetWorld()->Drop(attachedBlock, X(), Y(), Z(),
        srcSlot, destSlot, num);
}

void DeferredAction::Pour() const {
    Inventory * const attached_inv = attachedBlock->HasInventory();
    if ( attached_inv == nullptr ) return;

    Block * const vessel = attached_inv->ShowBlock(srcSlot);
    if ( vessel == nullptr ) return;

    Inventory * const vessel_inv = vessel->HasInventory();
    if ( vessel_inv == nullptr ) return;

    Block * const liquid = vessel_inv->ShowBlock(0);
    if ( liquid == nullptr ) return;

    if ( attachedBlock->GetWorld()->Build(liquid, X(), Y(), Z()) ) {
        vessel_inv->Pull(0);
    }
}

void DeferredAction::SetFire() const {
    World * const world = attachedBlock->GetWorld();
    if ( world->GetBlock(X(), Y(), Z())->Sub() == AIR ) {
         world->Build(BlockManager::NewBlock(GRASS, FIRE), X(), Y(), Z());
    }
}

void DeferredAction::SetGhostMove(const int dir) {
    type = DEFERRED_GHOST_MOVE;
    num  = dir;
}

void DeferredAction::SetMove(const int dir) {
    type = DEFERRED_MOVE;
    num  = dir;
}

void DeferredAction::SetJump() { type = DEFERRED_JUMP; }

void DeferredAction::SetBuild(const int x, const int y, const int z,
        Block * const mat, const int builder_slot)
{
    SetXyz(x, y, z);
    material = mat;
    srcSlot  = builder_slot;
    type     = DEFERRED_BUILD;
}

void DeferredAction::SetDamage(const int x, const int y, const int z) {
    SetXyz(x, y, z);
    type = DEFERRED_DAMAGE;
}

void DeferredAction::SetThrow(const int x, const int y, const int z,
        const int src, const int dest, const int n)
{
    SetXyz(x, y, z);
    srcSlot  = src;
    destSlot = dest;
    num  = n;
    type = DEFERRED_THROW;
}

void DeferredAction::SetPour(const int x, const int y, const int z,
        const int src)
{
    SetXyz(x, y, z);
    srcSlot = src;
    type = DEFERRED_POUR;
}

void DeferredAction::SetSetFire(const int x, int y, int z) {
    SetXyz(x, y, z);
    type = DEFERRED_SET_FIRE;
}

void DeferredAction::MakeAction() const {
    switch ( type ) {
    case DEFERRED_NOTHING: break;
    case DEFERRED_GHOST_MOVE: GhostMove(); break;
    case DEFERRED_MOVE:   Move();   break;
    case DEFERRED_JUMP:   Jump();   break;
    case DEFERRED_BUILD:  Build();  break;
    case DEFERRED_DAMAGE: Damage(); break;
    case DEFERRED_THROW:  Throw();  break;
    case DEFERRED_POUR:   Pour();   break;
    case DEFERRED_SET_FIRE: SetFire(); break;
    }
}

DeferredAction::DeferredAction(Animal * const attached) :
        Xyz(),
        type(DEFERRED_NOTHING),
        attachedBlock(attached),
        material(),
        srcSlot(),
        destSlot(),
        num()
{}
