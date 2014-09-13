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

#include "blocks/Container.h"
#include "world.h"
#include "BlockManager.h"
#include "CraftManager.h"

// Container::
    void Container::Push(const int, Block * const who) {
        Inventory::Push(who);
        if ( Sub()==DIFFERENT && IsEmpty() ) {
            GetWorld()->DestroyAndReplace(X(), Y(), Z());
        }
    }

    void Container::DoRareAction() {
        if ( Sub() == DIFFERENT ) {
            Inventory * const inv =
                GetWorld()->GetBlock(X(), Y(), Z()-1)->HasInventory();
            if ( inv ) {
                inv->GetAll(this);
            }
            if ( IsEmpty() ) {
                GetWorld()->DestroyAndReplace(X(), Y(), Z());
            }
        }
    }

    int Container::ShouldAct() const {
        return ( Sub() == IRON || Sub() == WATER ) ?
            FREQUENT_NEVER : FREQUENT_RARE;
    }

    int Container::Kind() const { return CONTAINER; }
    int Container::Sub() const { return Block::Sub(); }
    Inventory * Container::HasInventory() { return Inventory::HasInventory(); }
    usage_types Container::Use(Block *) { return USAGE_TYPE_OPEN; }

    Block * Container::DropAfterDamage(bool * const delete_block) {
        if ( DIFFERENT == Sub() ) {
            *delete_block = true;
            return nullptr;
        } // else:
        Block * const pile = block_manager.NewBlock(CONTAINER, DIFFERENT);
        Inventory * const pile_inv = pile->HasInventory();
        GetAll(pile_inv);
        *delete_block = not pile_inv->Get(this);
        return pile;
    }

    Active * Container::ActiveBlock() {
        return ( Sub() == IRON || Sub() == WATER) ?
            nullptr : this;
    }

    int Container::Weight() const {
        return Block::Weight()*4 + Inventory::Weight();
    }

    void Container::ReceiveSignal(const QString str) {
        Block::ReceiveSignal(str);
    }

    QString Container::FullName() const {
        switch ( Sub() ) {
        case DIFFERENT: return tr("Pile");
        case WOOD:      return tr("Wooden chest");
        case STONE:     return tr("Stone chest");
        case IRON:      return tr("Locker");
        case WATER:     return tr("Fridge");
        default:
            fprintf(stderr, "Container::FullName: unlisted sub: %d\n", Sub());
            return tr("Unknown container");
        }
    }

    void Container::SaveAttributes(QDataStream & out) const {
        Active::SaveAttributes(out);
        Inventory::SaveAttributes(out);
    }

    Container::Container(const int sub, const int id, const int size) :
            Active(sub, id, NONSTANDARD),
            Inventory(size)
    {}

    Container::Container(QDataStream & str, const int sub, const int id,
            const int size)
        :
            Active(str, sub, id, NONSTANDARD),
            Inventory(str, size)
    {}

// Workbench::
    void Workbench::Craft() {
        for (int i=0; i<Start(); ++i) { // remove previous products
            while ( Number(i) ) {
                Block * const to_pull = ShowBlock(i);
                Pull(i);
                block_manager.DeleteBlock(to_pull);
            }
        }
        int materials_number = 0;
        for (int i=Start(); i<Size(); ++i) {
            if ( Number(i) ) {
                ++materials_number;
            }
        }
        CraftList list(materials_number, 0);
        for (int i=Start(); i<Size(); ++i) {
            if ( Number(i) ) {
                list << new CraftItem({Number(i), ShowBlock(i)->GetId()});
            }
        }
        CraftList * products = world->GetCraftManager()->Craft(&list, Sub());
        if ( products != nullptr ) {
            for (int i=0; i<products->GetSize(); ++i) {
                for (int n=0; n<products->GetItem(i)->num; ++n) {
                    int id = products->GetItem(i)->id;
                    GetExact(block_manager.NewBlock(
                        block_manager.KindFromId(id),
                        block_manager. SubFromId(id)), i);
                }
            }
            delete products;
        }
    }

    bool Workbench::Drop(const int src, const int dest,
            const int num, Inventory * const inv_to)
    {
        if ( inv_to == nullptr
                || src  >= Size()
                || dest >= inv_to->Size()
                || Number(src) == 0 )
        {
            return false;
        }
        for (int i=0; i<num; ++i) {
            if ( not inv_to->Get(ShowBlock(src), dest) ) return false;
            Pull(src);
            if ( src < Start() ) {
                // remove materials:
                for (int i=Start(); i<Size(); ++i) {
                    while ( Number(i) ) {
                        Block * const to_pull = ShowBlock(i);
                        Pull(i);
                        block_manager.DeleteBlock(to_pull);
                    }
                }
            } else {
                Craft();
            }
        }
        return true;
    }

    QString Workbench::FullName() const {
        switch ( Sub() ) {
        case WOOD: return QObject::tr("Workbench");
        case IRON: return QObject::tr("Iron anvil");
        default:
            fprintf(stderr, "Workbench::FullName: sub (?): %d\n", Sub());
            return "Strange workbench";
        }
    }

    int Workbench::Kind() const { return WORKBENCH; }
    int Workbench::Start() const { return 2; }

    bool Workbench::Get(Block * const block, const int start) {
        if ( Inventory::Get(block, start) ) {
            Craft();
            return true;
        } else {
            return false;
        }
    }

    bool Workbench::GetAll(Inventory * const from) {
        if ( Inventory::GetAll(from) ) {
            Craft();
            return true;
        } else {
            return false;
        }
    }

    Workbench::Workbench(const int sub, const int id) :
            Container(sub, id, WORKBENCH_SIZE)
    {}
    Workbench::Workbench(QDataStream & str, const int sub, const int id) :
            Container(str, sub, id, WORKBENCH_SIZE)
    {}
