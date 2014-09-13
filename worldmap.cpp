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

#include <qmath.h>
#include <QSettings>
#include "worldmap.h"
#include "header.h"

const float PI = 3.141592f;

WorldMap::WorldMap(const QString world_name) :
        mapSize(),
        map(home_path + world_name + "/map.txt"),
        spawnLongitude(),
        spawnLatitude()
{
    if ( map.open(QIODevice::ReadOnly | QIODevice::Text) ) {
        mapSize = int(qSqrt(1+4*map.size())-1)/2;
    } else {
        GenerateMap(home_path + world_name, DEFAULT_MAP_SIZE, SHRED_WATER, 0);
        mapSize = ( map.open(QIODevice::ReadOnly | QIODevice::Text) ) ?
            DEFAULT_MAP_SIZE : 1;
    }
    MakeAndSaveSpawn(world_name, mapSize, &spawnLongitude, &spawnLatitude);
}

void WorldMap::MakeAndSaveSpawn(const QString world_name, const int size,
        long * longitude, long * latitude)
{
    QSettings map_info(home_path+world_name+"/map.ini", QSettings::IniFormat);
    *longitude = map_info.value("spawn_longitude", GetSpawnCoordinate(size)).
        toLongLong();
    *latitude  = map_info.value("spawn_latitude",  GetSpawnCoordinate(size)).
        toLongLong();
    map_info.setValue("spawn_longitude", qlonglong(*longitude));
    map_info.setValue("spawn_latitude" , qlonglong(*latitude ));
}

int WorldMap::GetSpawnCoordinate(int size) {
    size = qMax(size/4, 1);
    return (qrand()%size) + size;
}

long WorldMap::GetSpawnLongitude() const { return spawnLongitude; }
long WorldMap::GetSpawnLatitude()  const { return spawnLatitude;  }

char WorldMap::TypeOfShred(const long longi, const long lati) const {
    //return '-'; // for testing purposes
    if (
            longi >= mapSize || longi < 0 ||
            lati  >= mapSize || lati  < 0 )
    {
        return OUT_BORDER_SHRED;
    } else if ( not map.seek((mapSize+1)*longi+lati) ) {
        return DEFAULT_SHRED;
    }
    char c;
    map.getChar(&c);
    return c;
}

float WorldMap::Deg(const int x, const int y, const int size) {
    const float x_cent = x-size/2.f;
    const float y_cent = y-size/2.f;
    float fi;
    if ( x_cent > 0 && y_cent >= 0 ) {
        fi = atanf(y_cent/x_cent);
    } else if ( x_cent>0 && y_cent<0 ) {
        fi = atanf(y_cent/x_cent)+2*PI;
    } else if ( x_cent<0 ) {
        fi = atanf(y_cent/x_cent)+PI;
    } else if ( qFuzzyCompare(x_cent, 0.f) && y_cent>0 ) {
        fi = PI/2;
    } else if ( qFuzzyCompare(x_cent, 0.f) && y_cent<0 ) {
        fi = 3*PI/2;
    } else {
        fi = 0;
    }
    return 360*fi / 2 / PI;
}

float WorldMap::R(const int x, const int y, const int size) {
    return sqrtf( (x-size/2.f)*(x-size/2.f)+(y-size/2.f)*(y-size/2.f) );
}

void WorldMap::Circle(
        const int min_rad,
        const int max_rad,
        const char ch,
        const int size,
        char * const map)
{
    Q_ASSERT(min_rad < max_rad);
    float maxs[360] = { float(qrand()%(max_rad - min_rad) + min_rad) };
    for (int x=1; x<360; ++x) {
        maxs[x] = ( x > 345 ) ? // connect beginning and end of circle
            maxs[x-1] + (maxs[0] - maxs[345]) / 15 :
            qBound(float(min_rad),
                maxs[x-1]+(qrand()%400-200)/200.f, float(max_rad));
    }
    for (int y=0; y<size; ++y)
    for (int x=0; x<size; ++x) {
        if ( R(x, y, size) < maxs[Round(Deg(x, y, size))] ) {
            map[x*size+y] = ch;
        }
    }
}

void WorldMap::GenerateMap(
        const QString world_name,
        int size,
        const char outer,
        const int seed)
{
    qsrand(seed);
    size = qMax(10, size);

    char * const map = new char[size*size];
    memset(map, outer, size*size);

    const float min_rad = size/3.0f;
    const float max_rad = size/2.0f;
    Circle(min_rad,   max_rad,     SHRED_WASTE,       size, map);
    Circle(min_rad/2, max_rad/2,   SHRED_DEAD_FOREST, size, map);
    Circle(min_rad/3, max_rad/3+1, SHRED_DEAD_HILL,   size, map);
    Circle(min_rad/4, max_rad/4+1, SHRED_MOUNTAIN,    size, map);

    int lakes_number = (qrand() % size) + 5;
    while ( lakes_number-- ) {
        char type = SHRED_WATER;
        switch ( qrand()%4 ) {
        case 1: type = SHRED_ACID_LAKE; break;
        case 2: type = SHRED_LAVA_LAKE; break;
        case 3: type = SHRED_CRATER;    break;
        }
        const float lake_size  = qrand() % (size/10) + 1;
        const int lake_start_x = qrand() % int(size-lake_size);
        const int lake_start_y = qrand() % int(size-lake_size);
        const int border = (lake_size-1)*(lake_size-1)/2/2;
        for (int x=lake_start_x; x<lake_start_x+lake_size; ++x)
        for (int y=lake_start_y; y<lake_start_y+lake_size; ++y) {
            if (      (x-lake_start_x-lake_size/2)*(x-lake_start_x-lake_size/2)
                    + (y-lake_start_y-lake_size/2)*(y-lake_start_y-lake_size/2)
                    < border )
            {
                map[x*size+y] = type;
            }
        }
    }

    long spawn_longitude, spawn_latitude;
    MakeAndSaveSpawn(world_name, size, &spawn_longitude, &spawn_latitude);
    PieceOfEden(spawn_latitude-1, spawn_longitude-1, map, size);

    FILE * const file =
        fopen(qPrintable(home_path + world_name + "/map.txt"), "wb");
    for (int y=0; y<size; ++y, fputc('\n', file))
    for (int x=0; x<size; ++x) {
        fputc(map[x*size+y], file);
    }
    delete [] map;
    fclose(file);
}

void WorldMap::PieceOfEden(const int x, const int y,
        char * const map, const size_t size)
{
    if ( (x+5)*size + y+5 > size*size) return;
    char eden[][7] = {
        "^~~~~^",
        "~~%%~~",
        "~%++%~",
        "~%++%~",
        "~~%%~~",
        "^~~~~^"
    };
    for (int j=0; j<6; ++j)
    for (int i=0; i<6; ++i) {
        map[(x+j)*size + y+i] = eden[i][j];
    }
}
