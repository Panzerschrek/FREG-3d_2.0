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

#include "Xyz.h"

Xy::Xy(const short i, const short j) : x_self(i), y_self(j) {}
Xy::Xy()                             : x_self(),  y_self()  {}

Xyz::Xyz(const short i, const short j, const short k) : Xy(i, j), z_self(k) {}
Xyz::Xyz()                                            : Xy(),     z_self()  {}

short Xy ::X() const { return x_self; }
short Xy ::Y() const { return y_self; }
short Xyz::Z() const { return z_self; }

void Xyz::SetXyz(const short x, const short y, const short z) {
    x_self = x;
    y_self = y;
    z_self = z;
}
