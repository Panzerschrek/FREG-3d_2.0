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

#ifndef DWARF_H
#define DWARF_H

#include "blocks/Animal.h"
#include "Inventory.h"

const int MIN_DWARF_LIGHT_RADIUS = 2;

class Dwarf : public Animal, public Inventory {
    Q_OBJECT
public:
    Dwarf(int sub, int id);
    Dwarf(QDataStream & str, int sub, int id);

    int ShouldAct() const override;
    int DamageKind() const override;
    void Move(dirs direction) override;
    int Start() const override;
    int Weight() const override;
    int DamageLevel() const override;
    void Damage(int dmg, int dmg_kind) override;
    QString FullName() const override;
    QString InvFullName(int slot_number) const override;

    bool Access() const override;
    bool Inscribe(QString str) override;
    bool GetExact(Block *, int to) override;
    void ReceiveSignal(QString) override;
    int  LightRadius() const override;
    int  NutritionalValue(subs) const override;
    Block * DropAfterDamage(bool * delete_block) override;
    Inventory * HasInventory() override;

protected:
    void SaveAttributes(QDataStream & out) const override;

private:
    enum {
        ON_HEAD,
        ON_BODY,
        ON_LEGS,
        IN_RIGHT,
        IN_LEFT
    };

    void UpdateLightRadius();

    int lightRadius;
};

#endif // DWARF_H
