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

#ifndef CONTAINER_H
#define CONTAINER_H

#include "blocks/Active.h"
#include "blocks/Inventory.h"

class Container : public Active, public Inventory {
    /** \class Container Container.h
     *  \brief Container is multi-purpose container for blocks.
     *
     *  Behaviour depends on substance:
     *  - container from DIFFERENT is a pile, and will dissapear if empty;
     *  - container from IRON (locker) and WATER (fridge) prevents all activity
     *    inside;
     *  - container from other substances are different chests. */
    Q_OBJECT
public:
    Container(int sub, int id, int size = INV_SIZE);
    Container(QDataStream & str, int sub, int id, int size = INV_SIZE);

    int  Sub() const override;
    void ReceiveSignal(QString) override;
    void DoRareAction() override;
    int  ShouldAct() const override;
    void Push(int, Block * who) override;
    int  Weight() const override;
    int  Kind() const override;
    Block * DropAfterDamage(bool * delete_block) override;
    QString FullName() const override;
    Active * ActiveBlock() override;
    Inventory * HasInventory() override;
    usage_types Use(Block * who = 0) override;

protected:
    void SaveAttributes(QDataStream & out) const override;
}; // Container

class Workbench : public Container {
    /** \class Workbench Container.h
     *  \brief Workbench allows craft from multiple sources. There can be up to
     *  2 products. Also can be used as container of smaller size. */
    Q_OBJECT
public:
    Workbench(int sub, int id);
    Workbench(QDataStream & str, int sub, int id);

    int  Kind() const override;
    bool Drop(int src, int dest, int num, Inventory * inv) override;
    bool Get(Block * block, int start = 0) override;
    bool GetAll(Inventory * from) override;
    int Start() const override;
    QString FullName() const override;

private:
    void Craft();

    static const int WORKBENCH_SIZE = 10;
}; // Workbench

#endif // CONTAINER_H
