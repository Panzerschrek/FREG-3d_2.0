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

#ifndef XYZ_H
#define XYZ_H

class Xy {
public:
    Xy(short x, short y);
    Xy();

    short X() const;
    short Y() const;

protected:
    short x_self, y_self;
};

class Xyz : public Xy {
public:
    Xyz(short x, short y, short z);
    Xyz();

    short Z() const;
    void SetXyz(short x, short y, short z);

protected:
    short z_self;
};

#endif
