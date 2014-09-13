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

#ifndef BUCKET_H
#define BUCKET_H

#include "Block.h"
#include "Inventory.h"

class Bucket : public Block, public Inventory {
public:
    Bucket(int sub, int id);
    Bucket(QDataStream & str, int sub, int id);

    int  Weight() const override;
    void ReceiveSignal(QString str) override;
    bool Get(Block * block, int start) override;
    void Damage(int dmg, int dmg_kind) override;
    QString FullName() const override;
    usage_types Use(Block *) override;
    Inventory * HasInventory() override final;

protected:
    void SaveAttributes(QDataStream & out) const override;
};

#endif // BUCKET_H
