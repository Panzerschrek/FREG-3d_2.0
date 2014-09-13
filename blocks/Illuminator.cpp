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

#include "blocks/Illuminator.h"
#include "BlockManager.h"
#include "Inventory.h"

Illuminator::Illuminator(const int kind, const int sub) :
        Active(kind, sub),
        fuelLevel(MAX_FUEL)
{}

Illuminator::Illuminator(QDataStream & str, const int kind, const int sub) :
        Active(str, kind, sub),
        fuelLevel()
{
    str >> fuelLevel;
}

int  Illuminator::DamageKind() const {
    return (Sub()==GLASS) ? DAMAGE_NO : DAMAGE_HEAT;
}

QString Illuminator::FullName() const {
    switch ( Sub() ) {
    case WOOD:  return tr("Torch, fuel: %1"        ).arg(fuelLevel);
    case STONE: return tr("Flint, charges: %1"     ).arg(fuelLevel);
    case GLASS: return tr("Flashlight, battery: %1").arg(fuelLevel);
    default:    return tr("Lantern (%1), fuel: %2" ).
        arg(SubName(Sub())).arg(fuelLevel);
    }
}

Block * Illuminator::DropAfterDamage(bool * const delete_block) {
    *delete_block = false;
    Block * const pile = BlockManager::NewBlock(BOX, DIFFERENT);
    pile->HasInventory()->Get(this);
    return pile;
}

usage_types Illuminator::Use(Block *) {
    if ( Sub() == GLASS ) { // flashlight
        return USAGE_TYPE_NO;
    } else {
        fuelLevel = ( fuelLevel >= 10 ) ?
            fuelLevel - 10 : 0;
        return USAGE_TYPE_SET_FIRE;
    }
}

int Illuminator::LightRadius() const {
    if ( fuelLevel == 0 ) return 0;
    switch ( Sub() ) {
    default:
    case STONE: return 0;
    case WOOD:  return 4;
    case IRON:  return 5;
    case GLASS: return 7;
    }
}

int  Illuminator::ShouldAct() const { return FREQUENT_RARE; }
void Illuminator::DoRareAction() { ActInner(); }
wearable Illuminator::Wearable() const { return WEARABLE_OTHER; }

inner_actions Illuminator::ActInner() {
    if ( fuelLevel == 0 ) {
        if ( Sub() == WOOD ) {
            Break();
        }
    } else {
        if ( Sub() != STONE ) {
            --fuelLevel;
        }
    }
    return INNER_ACTION_NONE;
}

void Illuminator::SaveAttributes(QDataStream & out) const {
    Active::SaveAttributes(out);
    out << fuelLevel;
}
