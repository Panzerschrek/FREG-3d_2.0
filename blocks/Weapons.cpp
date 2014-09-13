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

#include "blocks/Weapons.h"
#include "blocks/Inventory.h"
#include "BlockManager.h"
#include "Shred.h"

// Weapon::
    QString Weapon::FullName() const {
        switch ( Sub() ) {
        case STONE: return QObject::tr("Pebble");
        case IRON:  return QObject::tr("Spike");
        case BONE:  return QObject::tr("Bone");
        case SKY:   return QObject::tr("Air");
        default:    return QObject::tr("Stick (%1)").arg(SubName(Sub()));
        }
    }

    int  Weapon::Weight() const { return Block::Weight()/4; }
    wearable Weapon::Wearable() const { return WEARABLE_OTHER; }
    push_reaction Weapon::PushResult(dirs) const { return DAMAGE; }

    int Weapon::DamageLevel() const {
        switch ( Sub() ) {
        default:    return  1;
        case WOOD:  return  4;
        case BONE:  return  5;
        case STONE: return  7;
        case IRON:  return 10;
        case STEEL: return 12;
        case ADAMANTINE: return 20;
        }
    }

    int  Weapon::DamageKind() const {
        switch ( Sub() ) {
        case IRON: return DAMAGE_THRUST;
        case SKY:  return DAMAGE_ULTIMATE;
        default:   return DAMAGE_CRUSH;
        }
    }

// Pick::
    int Pick::DamageKind() const { return DAMAGE_MINE; }

    QString Pick::FullName() const {
        return QObject::tr("Pick (%1)").arg(SubName(Sub()));
    }

// Shovel::
    int Shovel::DamageKind() const { return DAMAGE_DIG; }

    QString Shovel::FullName() const {
        return QObject::tr("Shovel (%1)").arg(SubName(Sub()));
    }

// Hammer::
    int Hammer::DamageKind() const { return DAMAGE_CRUSH; }

    QString Hammer::FullName() const {
        return QObject::tr("Hammer (%1)").arg(SubName(Sub()));
    }

// Axe::
    int Axe::DamageKind() const { return DAMAGE_CUT; }

    QString Axe::FullName() const {
        return QObject::tr("Axe (%1)").arg(SubName(Sub()));
    }
