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

#ifndef FILTER_H
#define FILTER_H

#include "Containers.h"

class Filter : public Container {
    Q_OBJECT
public:
    Filter(int kind, int sub);
    Filter(QDataStream &, int kind, int sub);

    bool Get(Block * block, int start) override;
    Active * ActiveBlock() override;
    QString FullName() const override;
    QString InvFullName(int slot_number) const override;
};

#endif // FILTER_H
