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

/**\file Lighting-inertia.cpp
    * \brief This file provides simple (also mad) lighting for freg.
    *
    * It has light inertia, meaning that block, enlightened by outer
    * source, will remain enlightened when light source is removed.
    * Light is divided to sunlight and other light. Sunlight is
    * changing over the day.
    *
    * This lighting can be used to develop more intelligent one.
    * Intended to be fast (and is!).
    *
    * LightMap is uchar:
    * & 0xF0 bits are for non-sun light,
    * & 0x0F bits for sun. */

#include "World.h"
#include "Shred.h"
#include "blocks/Block.h"
#include "blocks/Active.h"

const int FIRE_LIGHT_FACTOR = 4;

/// Use Enlightened instead, which is smart wrapper of this.
int World::LightMap(const int x, const int y, const int z) const {
    return GetShred(x, y)->
        Lightmap(Shred::CoordInShred(x), Shred::CoordInShred(y), z);
}

bool World::SetFireLightMap(const int level,
        const int x, const int y, const int z)
{
    return GetShred(x, y)->SetFireLight(
        Shred::CoordInShred(x), Shred::CoordInShred(y), z, level);
}

void World::AddFireLight(int, int, int, int) {}
void World::RemoveFireLight(int, int, int) {}

/// Makes block emit shining.
/** Receives only non-sun light as level, from 1 to F. */
void World::Shine(const int x, const int y, const int z, int level,
        const bool init)
{
    const int transparent = GetBlock(x, y, z)->Transparent();
    if ( SetFireLightMap(level << 4, x, y, z) ) {
        if ( INVISIBLE != transparent && not initial_lighting ) {
            emit Updated(x, y, z);
        }
        if ( not init ) {
            return;
        }
    }
    if ( (transparent != BLOCK_OPAQUE && level > 1) || init ) {
        --level;
        const int border = GetBound();
        if ( x > 0 )      Shine(x-1, y,   z, level, false);
        if ( x < border ) Shine(x+1, y,   z, level, false);
        if ( y > 0 )      Shine(x,   y-1, z, level, false);
        if ( y < border ) Shine(x,   y+1, z, level, false);
        Shine(x, y, z-1, level, false);
        Shine(x, y, z+1, level, false);
    }
}

void World::SunShineVertical(const int x, const int y, int z, int light_lev) {
    /* 2 1 3
     *   *   First, light goes down, then divides to 4 branches
     * ^ | ^ to N-S-E-W, and goes up.
     * | | |
     * | | |
     * |<v>|
     *   #     */
    Shred * const shred = GetShred(x, y);
    const int x_in = Shred::CoordInShred(x);
    const int y_in = Shred::CoordInShred(y);
    shred->SetLightmap(x_in, y_in, HEIGHT-1, 0xFF);
    for ( ; ; --z) {
        shred->SetSunLight(x_in, y_in, z, light_lev);
        switch ( shred->GetBlock(x_in, y_in, z)->Transparent() ) {
        case BLOCK_OPAQUE: CrossUpShine(x, y, z); return;
        case BLOCK_TRANSPARENT:
            --light_lev;
            if ( not initial_lighting ) {
                emit Updated(x, y, z);
            }
            break;
        case INVISIBLE: break;
        }
    }
}

void World::CrossUpShine(const int x, const int y, const int z_bottom) {
    const int bound = GetBound();
    if ( initial_lighting ) {
        if ( x > 0     ) UpShineInit(x-1, y,   z_bottom);
        if ( x < bound ) UpShineInit(x+1, y,   z_bottom);
        if ( y > 0     ) UpShineInit(x,   y-1, z_bottom);
        if ( y < bound ) UpShineInit(x,   y+1, z_bottom);
    } else {
        if ( x > 0     ) UpShine(x-1, y,   z_bottom);
        if ( x < bound ) UpShine(x+1, y,   z_bottom);
        if ( y > 0     ) UpShine(x,   y-1, z_bottom);
        if ( y < bound ) UpShine(x,   y+1, z_bottom);
        emit Updated(x, y, z_bottom);
    }
}

void World::UpShineInit(int x, int y, int z_bottom) {
    Shred * const shred = GetShred(x, y);
    x = Shred::CoordInShred(x);
    y = Shred::CoordInShred(y);
    for ( ; shred->SetSunLight(x, y, z_bottom, 1) && z_bottom < HEIGHT-1;
            ++z_bottom);
}

void World::UpShine(const int x, const int y, int z_bottom) {
    Shred * const shred = GetShred(x, y);
    const int x_in = Shred::CoordInShred(x);
    const int y_in = Shred::CoordInShred(y);
    while (shred->SetSunLight(x_in, y_in, z_bottom, 1) && z_bottom<HEIGHT-1) {
        emit Updated(x, y, z_bottom++);
    }
}

/// Called when one block is moved, built or destroyed.
void World::ReEnlighten(const int x, const int y, const int z) {
    if ( not GetEvernight() ) {
        SunShineVertical(x, y);
    }
    const int radius = GetBlock(x, y, z)->LightRadius();
    if ( radius != 0 ) {
        Shine(x, y, z, radius, true);
    }
}

void World::ReEnlightenTime() {
    for (int i=0; i<NumShreds()*NumShreds(); ++i) {
        shreds[i]->SetAllLightMapNull();
    }
    sunMoonFactor = ( TIME_NIGHT==PartOfDay() ) ?
        MOON_LIGHT_FACTOR : SUN_LIGHT_FACTOR;
    ReEnlightenAll();
}

void World::ReEnlightenAll() {
    initial_lighting = true;
    for (int i=0; i<NumShreds()*NumShreds(); ++i) {
        shreds[i]->ShineAll();
    }
    initial_lighting = false;
}

void World::ReEnlightenMove(const dirs dir) {
    initial_lighting = true;
    switch ( dir ) {
    case NORTH:
        for (int i=0; i<NumShreds(); ++i) {
            shreds[i            ]->ShineAll();
            shreds[i+NumShreds()]->ShineAll();
        }
        break;
    case SOUTH:
        for (int i=0; i<NumShreds(); ++i) {
            shreds[i+NumShreds()*(NumShreds()-1)]->ShineAll();
            shreds[i+NumShreds()*(NumShreds()-2)]->ShineAll();
        }
        break;
    case EAST:
        for (int i=0; i<NumShreds(); ++i) {
            shreds[NumShreds()*i+NumShreds()-1]->ShineAll();
            shreds[NumShreds()*i+NumShreds()-2]->ShineAll();
        }
        break;
    case WEST:
        for (int i=0; i<NumShreds(); ++i) {
            shreds[NumShreds()*i  ]->ShineAll();
            shreds[NumShreds()*i+1]->ShineAll();
        }
        break;
    default: Q_UNREACHABLE(); break;
    }
    initial_lighting = false;
}

int World::Enlightened(const int x, const int y, const int z) const {
    const int light = LightMap(x, y, z);
    return   (light & 0x0F) * sunMoonFactor +
           ( (light & 0xF0) >> 4 ) * FIRE_LIGHT_FACTOR;
}

/// Provides lighting of block side, not all block.
int World::Enlightened(const int i, const int j, const int k, const dirs dir)
const {
    int x, y, z;
    Focus(i, j, k, &x, &y, &z, dir);
    return qMin(Enlightened(i, j, k), Enlightened(x, y, z));
}

int World::SunLight(const int i, const int j, const int k) const {
    return (LightMap(i, j, k) & 0x0F) * sunMoonFactor;
}

int World::FireLight(const int x, const int y, const int z) const {
    return (LightMap(x, y, z) & 0xF0) * FIRE_LIGHT_FACTOR;
}

// Shred methods

int Shred::Lightmap(const int x, const int y, const int z) const {
    return lightMap[x][y][z];
}

int Shred::FireLight(const int x, const int y, const int z) const {
    return ((lightMap[x][y][z] & 0xF0) >> 4) * FIRE_LIGHT_FACTOR;
}

int Shred::SunLight(const int x, const int y, const int z) const {
    return (lightMap[x][y][z] & 0x0F);
}

bool Shred::SetSunLight(const int x, const int y, const int z, const int level)
{
    if ( ( lightMap[x][y][z] &  0x0F ) <  level ) {
         ( lightMap[x][y][z] &= 0xF0 ) |= level;
        return true;
    } else {
        return false;
    }
}

bool Shred::SetFireLight(const int x, const int y, const int z,
        const int level)
{
    if ( ( lightMap[x][y][z] &  0xF0 ) <  level ) {
         ( lightMap[x][y][z] &= 0x0F ) |= level;
        return true;
    } else {
        return false;
    }
}

void Shred::SetLightmap(const int x, const int y, const int z, const int level)
{
    lightMap[x][y][z] = level;
}

void Shred::SetAllLightMapNull() { memset(lightMap, 0, sizeof(lightMap)); }

/// Makes all shining blocks of shred shine.
void Shred::ShineAll() {
    for (auto i=ShiningBegin(); i!=ShiningEnd(); ++i) {
        GetWorld()->Shine((*i)->X(), (*i)->Y(), (*i)->Z(),
            (*i)->LightRadius(), true);
    }
    if ( not GetWorld()->GetEvernight() ) {
        for (int i=shredX*SHRED_WIDTH; i<SHRED_WIDTH*(shredX+1); ++i)
        for (int j=shredY*SHRED_WIDTH; j<SHRED_WIDTH*(shredY+1); ++j) {
            GetWorld()->SunShineVertical(i, j);
        }
    }
}
