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

#include "blocks/Animal.h"
#include "blocks/Inventory.h"
#include "DeferredAction.h"
#include "BlockManager.h"
#include "World.h"

// Animal:: section
    inner_actions Animal::ActInner() {
        if ( GROUP_MEAT != GetSubGroup(Sub()) ) return INNER_ACTION_NONE;
        if ( satiation <= 0 ) {
            Damage(5, DAMAGE_HUNGER);
        } else {
            --satiation;
            Mend(1);
        }
        emit Updated();
        return INNER_ACTION_NONE;
    }

    void Animal::ActFrequent() {
        if ( deferredAction != nullptr ) {
            deferredAction->MakeAction();
            delete deferredAction;
            deferredAction = nullptr;
        }
    }

    void Animal::DoRareAction() {
        if ( GROUP_MEAT != GetSubGroup(Sub()) ) return;
        if ( not IsSubAround(AIR) ) {
            if ( breath <= 0 ) {
                Damage(10, DAMAGE_BREATH);
            } else {
                --breath;
            }
        } else if ( breath < MAX_BREATH ) {
            ++breath;
        }
        if ( GetDurability() <= 0 ) {
            GetWorld()->DestroyAndReplace(X(), Y(), Z());
        } else {
            emit Updated();
        }
    }

    int Animal::Breath() const { return breath; }
    int Animal::Satiation() const { return satiation; }
    int Animal::ShouldAct() const { return FREQUENT_SECOND | FREQUENT_RARE; }
    int Animal::DamageKind() const { return DAMAGE_BITE; }
    int Animal::NutritionalValue(subs) const { return 0; }
    Animal * Animal::IsAnimal() { return this; }

    bool Animal::Eat(const subs sub) {
        const int value = NutritionalValue(sub);
        if ( value ) {
            satiation += value;
            ReceiveSignal(tr("Ate."));
            if ( SECONDS_IN_DAY < satiation ) {
                satiation = 1.1 * SECONDS_IN_DAY;
                ReceiveSignal(tr("You have gorged yourself!"));
            }
            return true;
        } else {
            return false;
        }
    }

    void Animal::SaveAttributes(QDataStream & out) const {
        Falling::SaveAttributes(out);
        out << breath << satiation;
    }

    void Animal::EatGrass() {
        for (int x=X()-1; x<=X()+1; ++x)
        for (int y=Y()-1; y<=Y()+1; ++y) {
            if ( world->InBounds(x, y) &&
                    GREENERY == world->GetBlock(x, y, Z())->Sub() )
            {
                TryDestroy(x, y, Z());
                Eat(GREENERY);
                return;
            }
        }
    }

    Block * Animal::DropAfterDamage(bool *) {
        Block * const cadaver = BlockManager::NewBlock(CONTAINER, Sub());
        cadaver->HasInventory()->Get(BlockManager::NewBlock(WEAPON, BONE));
        return cadaver;
    }

    void Animal::SetDeferredAction(DeferredAction * const action) {
        delete deferredAction;
        deferredAction = action;
    }

    Animal::Animal(const int kind, const int sub) :
            Falling(kind, sub, NONSTANDARD),
            breath(MAX_BREATH),
            satiation(SECONDS_IN_DAY)
    {}

    Animal::Animal(QDataStream & str, const int kind, const int sub) :
            Falling(str, kind, sub, NONSTANDARD),
            breath(),
            satiation()
    {
        str >> breath >> satiation;
    }

    Animal::~Animal() {
        delete deferredAction;
        deferredAction = nullptr;
    }

// Predator:: section
    int Predator::DamageLevel() const { return 10; }
    QString Predator::FullName() const { return "Predator"; }

    int Predator::NutritionalValue(const subs sub) const {
        return Attractive(sub) * SECONDS_IN_HOUR;
    }

    void Predator::ActFrequent() {
        if ( Gravitate(5, 1, 2, 0) ) {
            world->Move(X(), Y(), Z(), GetDir());
        }
    }

    void Predator::DoRareAction() {
        const Xyz coords[] = {
            Xyz(X()-1, Y(), Z()),
            Xyz(X()+1, Y(), Z()),
            Xyz(X(), Y()-1, Z()),
            Xyz(X(), Y()+1, Z()),
            Xyz(X(), Y(), Z()-1)
        };
        World * const world = GetWorld();
        for (const Xyz xyz : coords) {
            if ( not world->InBounds(xyz.X(), xyz.Y()) ) {
                continue;
            }
            Block * const block = world->GetBlock(xyz.X(), xyz.Y(), xyz.Z());
            if ( Attractive(block->Sub()) ) {
                block->ReceiveSignal(tr("Predator bites you!"));
                world->Damage(xyz.X(), xyz.Y(), xyz.Z(),
                    DamageLevel(), DamageKind());
                Eat(static_cast<subs>(block->Sub()));
            }
        }
        if ( SECONDS_IN_DAY/4 > Satiation() ) {
            EatGrass();
        }
        Animal::DoRareAction();
    }

    int Predator::Attractive(const int sub) const {
        return ( GROUP_MEAT == GetSubGroup(sub) ) ?
            10 : ( sub == GREENERY ) ?
                1 : 0;
    }
