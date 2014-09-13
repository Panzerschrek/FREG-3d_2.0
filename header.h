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

#ifndef HEADER_H
#define HEADER_H

#include <QtGlobal>
#include <QLocale>

#ifdef Q_OS_WIN32
#include <windows.h>
#endif

#ifdef QT_NO_DEBUG
const bool DEBUG = false;
#else
const bool DEBUG = true;
#endif

extern const QString home_path;

const int SHRED_WIDTH = 16;
const int HEIGHT = 128;

enum times {
    SECONDS_IN_HOUR = 60,
    SECONDS_IN_DAY = 24*SECONDS_IN_HOUR,
    END_OF_NIGHT   =  6*SECONDS_IN_HOUR,
    END_OF_MORNING = 12*SECONDS_IN_HOUR,
    END_OF_NOON    = 18*SECONDS_IN_HOUR,
    END_OF_EVENING =  0*SECONDS_IN_HOUR,
    SECONDS_IN_NIGHT = END_OF_NIGHT,
    SECONDS_IN_DAYLIGHT = SECONDS_IN_DAY-END_OF_NIGHT
};

#ifdef Q_OS_WIN32
const QString locale = "en";
#else
const QString locale = QLocale::system().name();
#endif

enum shred_type {
    SHRED_PLAIN     = '.',
    SHRED_TESTSHRED = 'T',
    SHRED_PYRAMID   = 'P',
    SHRED_HILL      = '+',
    SHRED_DESERT    = ':',
    SHRED_WATER     = '~',
    SHRED_FOREST    = '%',
    SHRED_MOUNTAIN  = '^',
    SHRED_EMPTY     = '_',
    SHRED_CHAOS     = '!',
    SHRED_CASTLE    = 'C',
    SHRED_WASTE     = '=',
    SHRED_ACID_LAKE = 'a',
    SHRED_LAVA_LAKE = 'l',
    SHRED_CRATER    = 'c',
    SHRED_DEAD_FOREST = 'f',
    SHRED_DEAD_HILL   = '*',
    SHRED_NULLMOUNTAIN = '#',
    SHRED_NORMAL_UNDERGROUND = '-',
};

const int DEFAULT_MAP_SIZE = 79;
const char DEFAULT_SHRED = SHRED_PLAIN;
const char OUT_BORDER_SHRED = SHRED_WATER;

enum dirs {
    ANYWHERE = 0,
    UP = 0, ///< 0
    DOWN,   ///< 1
    NORTH,  ///< 2
    SOUTH,  ///< 3
    EAST,   ///< 4
    WEST    ///< 5
};

enum push_reaction {
    MOVABLE,
    ENVIRONMENT,
    NOT_MOVABLE,
    MOVE_UP,
    JUMP,
    DAMAGE,
};

enum times_of_day {
    TIME_NIGHT,
    TIME_MORNING,
    TIME_NOON,
    TIME_EVENING
};

/// Kinds of atom
enum kinds {
    // add new kinds to bottom (before LAST_KIND).
    // changind kind codes will break file compatibility.
    BLOCK,       ///<  0
    BELL,        ///<  1
    CONTAINER,   ///<  2
    DWARF,       ///<  3
    PICK,        ///<  4
    LIQUID,      ///<  5
    GRASS,       ///<  6
    BUSH,        ///<  7
    RABBIT,      ///<  8
    FALLING,     ///<  9
    CLOCK,       ///< 10
    PLATE,       ///< 11
    WORKBENCH,   ///< 12
    WEAPON,      ///< 13
    LADDER,      ///< 14
    DOOR,        ///< 15
    BOX,         ///< 16
    KIND_TEXT,   ///< 17
    MAP,         ///< 18
    PREDATOR,    ///< 19
    BUCKET,      ///< 20
    SHOVEL,      ///< 21
    AXE,         ///< 22
    HAMMER,      ///< 23
    ILLUMINATOR, ///< 24
    RAIN_MACHINE, ///< 25
    CONVERTER,    ///< 26
    ARMOUR,       ///< 27
    HELMET,       ///< 28
    BOOTS,        ///< 29
    TELEGRAPH,    ///< 30
    MEDKIT,       ///< 31
    FILTER,       ///< 32
    INFORMER,     ///< 33
    /// Nothing is LAST_KIND.
    LAST_KIND // keep it last in this list.
}; // enum kinds

/// Substance block is made from.
/** Don't make blocks from SKY and STAR, they are special for shred loading
 *  and saving.
 *  Don't make non-BLOCK blocks from air, otherwise leaks are possible. */
enum subs {
    // do not change order, this will break file compatibility.
    // add new substances right before LAST_SUB.
    STONE,      ///<   0
    MOSS_STONE, ///<   1
    NULLSTONE,  ///<   2
    SKY,        ///<   3
    STAR,       ///<   4
    DIAMOND,    ///<   5
    SOIL,       ///<   6
    H_MEAT,     ///<   7 (hominid meat)
    A_MEAT,     ///<   8 (animal meat)
    GLASS,      ///<   9
    WOOD,       ///<  10
    DIFFERENT,  ///<  11
    IRON,       ///<  12
    WATER,      ///<  13
    GREENERY,   ///<  14
    SAND,       ///<  15
    HAZELNUT,   ///<  16
    ROSE,       ///<  17
    CLAY,       ///<  18
    AIR,        ///<  19
    PAPER,      ///<  20
    GOLD,       ///<  21
    BONE,       ///<  22
    STEEL,      ///<  23
    ADAMANTINE, ///<  24
    FIRE,       ///<  25
    COAL,       ///<  26
    EXPLOSIVE,  ///<  27
    ACID,       ///<  28
    SUB_CLOUD,  ///<  29
    SUB_DUST,   ///<  30
    /// Nothing is made from LAST_SUB.
    LAST_SUB // keep it last in this list
}; // enum subs

enum usage_types {
    USAGE_TYPE_NO,
    USAGE_TYPE_OPEN,
    USAGE_TYPE_READ,
    USAGE_TYPE_READ_IN_INVENTORY,
    USAGE_TYPE_POUR,
    USAGE_TYPE_SET_FIRE
};

enum transparency {
    BLOCK_OPAQUE = 0,
    BLOCK_TRANSPARENT,
    INVISIBLE,
    NONSTANDARD = 6,
    UNDEF // temporary, doesn't appear in world.
};

bool IsLikeAir(int sub);

/// For positive numbers only.
inline int Round(const float x) { return int(x + 0.5f); }

inline int Abs(const int x) {
    const int mask = x >> (sizeof(int)*8 - 1);
    return (x ^ mask) - mask;
}

#endif // HEADER_H
