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

#include "Block.h"
#include "World.h"
#include "Inventory.h"
#include "CraftManager.h"
#include "BlockManager.h"

int  Inventory::Start() const { return 0; }
int  Inventory::Size()  const { return size; }
int  Inventory::Number(const int i) const { return inventory[i].size(); }
bool Inventory::Access() const { return true; }
QString Inventory::NumStr(const int num) { return QString(" (%1x)").arg(num); }

bool Inventory::Drop(const int src, int dest, int num,
        Inventory * const inv_to)
{
    dest = qMax(inv_to->Start(), dest);
    bool ok_flag = false;
    while ( num-- ) {
        if ( src < Size()
                && dest < inv_to->Size()
                && not inventory[src].isEmpty()
                && inv_to->Get(inventory[src].top(), dest) )
        {
            ok_flag = true;
            Pull(src);
        }
    }
    return ok_flag;
}

bool Inventory::GetAll(Inventory * const from) {
    bool flag = false;
    for (int i=0; i<from->Size(); ++i) {
        if ( from->Drop(i, 0, from->Number(i), this) ) {
            flag = true;
        }
    }
    return flag;
}

void Inventory::Pull(const int num) {
    if ( not inventory[num].isEmpty() ) {
        inventory[num].pop();
    }
}

void Inventory::SaveAttributes(QDataStream & out) const {
    for (int i=0; i<Size(); ++i) {
        out << quint8(Number(i));
        if ( not inventory[i].isEmpty() ) {
            Block * const to_save = inventory[i].top();
            for (int j=0; j<Number(i); ++j) {
                to_save->SaveToFile(out);
                to_save->RestoreDurabilityAfterSave();
            }
        }
    }
}

bool Inventory::Get(Block * const block, const int start) {
    if ( block == nullptr ) return true;
    if ( block->Wearable() == WEARABLE_VESSEL ) {
        for (int i=0; i<Size(); ++i) {
            if ( Number(i)==1 && ShowBlock(i) ) {
                Inventory * const inner = ShowBlock(i)->HasInventory();
                if ( inner && inner->Get(block) ) {
                    return true;
                }
            }
        }
    } else {
        for (int i=start; i<Size(); ++i) {
            if ( GetExact(block, i) ) {
                return true;
            }
        }
    }
    ReceiveSignal(QObject::tr("No room."));
    return false;
}

bool Inventory::GetExact(Block * const block, const int num) {
    if ( block == nullptr) return true;
    if ( inventory[num].isEmpty() ||
            (*block == *inventory[num].top() && Number(num)<MAX_STACK_SIZE) )
    {
        inventory[num].push(block);
        return true;
    } else {
        return false;
    }
}

void Inventory::MoveInside(const int from, const int num_to, const int num) {
    for (int i=0; i<num; ++i) {
        if ( GetExact(ShowBlock(from), num_to) ) {
            Pull(from);
        }
    }
}

bool Inventory::InscribeInv(const int num, const QString str) {
    const int number = Number(num);
    if ( number == 0 ) {
        ReceiveSignal(QObject::tr("Nothing here."));
        return false;
    }
    const int sub = inventory[num].top()->Sub();
    if ( inventory[num].top() == block_manager.Normal(sub) ) {
        for (int i=0; i<number; ++i) {
            inventory[num].replace(i, block_manager.Normal(sub));
        }
    }
    for (int i=0; i<number; ++i) {
        if ( not inventory[num].at(i)->Inscribe(str) ) {
            ReceiveSignal(QObject::tr("Cannot inscribe this."));
            return false;
        }
    }
    ReceiveSignal(QObject::tr("Inscribed."));
    return true;
}

QString Inventory::InvFullName(const int num) const {
    return inventory[num].isEmpty() ?
        QObject::tr("-empty-") : inventory[num].top()->FullName();
}

int Inventory::GetInvWeight(const int i) const {
    return inventory[i].isEmpty() ?
        0 : inventory[i].top()->Weight()*Number(i);
}

int Inventory::GetInvSub(const int i) const {
    return inventory[i].isEmpty() ?
        AIR : inventory[i].top()->Sub();
}

int Inventory::GetInvKind(const int i) const {
    return inventory[i].isEmpty() ?
        BLOCK : int(inventory[i].top()->Kind());
}

int Inventory::Weight() const {
    int sum = 0;
    for (int i=0; i<Size(); ++i) {
        sum += GetInvWeight(i);
    }
    return sum / MAX_STACK_SIZE;
}

Block * Inventory::ShowBlockInSlot(const int slot, const int index) const {
    return ( slot >= Size() || index >= Number(slot) ) ?
        nullptr : inventory[slot].at(index);
}

Block * Inventory::ShowBlock(const int slot) const {
    return ( slot >= Size() || inventory[slot].isEmpty() ) ?
        nullptr : inventory[slot].top();
}

bool Inventory::IsEmpty() const {
    for (int i=Start(); i<Size(); ++i) {
        if ( not inventory[i].isEmpty() ) {
            return false;
        }
    }
    return true;
}

void Inventory::Push(const int x, const int y, const int z,
        const int push_direction)
{
    int x_targ, y_targ, z_targ;
    world->Focus(x, y, z, &x_targ, &y_targ, &z_targ,
        World::Anti(Block::MakeDirFromDamage(push_direction)));
    Inventory * const inv =
        world->GetBlock(x_targ, y_targ, z_targ)->HasInventory();
    if ( inv != nullptr ) {
        inv->GetAll(this);
    }
}

bool Inventory::MiniCraft(const int num) {
    if ( Number(num) == 0 ) {
        ReceiveSignal(QObject::tr("Nothing here."));
        return false;
    } // else:
    CraftItem * crafted =
        new CraftItem({Number(num), GetInvKind(num), GetInvSub(num)});
    if ( craft_manager->MiniCraft(&crafted) ) {
        while ( not inventory[num].isEmpty() ) {
            block_manager.DeleteBlock(ShowBlock(num));
            Pull(num);
        }
        for (int i=0; i<crafted->num; ++i) {
            GetExact(BlockManager::NewBlock(crafted->kind, crafted->sub), num);
        }
        ReceiveSignal(QObject::tr("Craft successful."));
        delete crafted;
        return true;
    } else {
        ReceiveSignal(QObject::tr("You don't know how to craft this."));
        return false;
    }
}

void Inventory::Shake() {
    for (int i=Start(); i<Size()-1; ++i) {
        if ( Number(i) != MAX_STACK_SIZE ) {
            for (int j=i; j<Size(); ++j) {
                MoveInside(j, i, Number(j));
            }
        }
    }
}

Inventory::Inventory(const int sz) :
        size(sz),
        inventory(new QStack<Block *>[sz])
{}

Inventory::Inventory(QDataStream & str, const int sz) :
        Inventory(sz)
{
    for (int i=0; i<Size(); ++i) {
        quint8 num;
        str >> num;
        while ( num-- ) {
            quint8 kind, sub;
            inventory[i].push(BlockManager::KindSubFromFile(str, &kind, &sub) ?
                block_manager.Normal(sub) :
                BlockManager::BlockFromFile(str, kind, sub));
        }
    }
}

Inventory::~Inventory() {
    const int size = Size();
    for (int i=0; i<size; ++i) {
        while ( not inventory[i].isEmpty() ) {
            block_manager.DeleteBlock(inventory[i].pop());
        }
    }
    delete [] inventory;
}
