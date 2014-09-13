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

/**\file Inventory.h
 * \brief Provides declaration for class Inventory for freg. */

#ifndef INVENTORY_H
#define INVENTORY_H

#include <QStack>

const int INV_SIZE = 26;
const int MAX_STACK_SIZE = 9;

class Block;

class Inventory {
    /**\class Inventory Inventory.h
     * \brief Provides block ability to contain other blocks inside. */
public:
    Inventory & operator=(Inventory &) = delete;
    Inventory(Inventory & inv) = delete;

    /// Returns true on success.
    virtual bool Drop(int src, int dest, int num, Inventory * to);
    /// Returns true on success.
    virtual bool GetAll(Inventory * from);
    virtual bool Access() const;
    /// Returns true on success.
    virtual bool Get(Block * block, int start = 0);
    virtual void ReceiveSignal(QString) = 0;
    virtual int Start() const;
    virtual int Weight() const;
    virtual QString FullName() const = 0;
    virtual Inventory * HasInventory() = 0;
    /// Returns true if block found its place.
    virtual bool GetExact(Block * block, int num);
    virtual QString InvFullName(int num) const;

    /// Removes block from inventory. Does not delete block.
    void Pull(int num);
    void MoveInside(int num_from, int num_to, int num);
    /// Returns true on success (something has been crafted).
    bool MiniCraft(int num);
    /// Returns true on success.
    bool InscribeInv(int num, QString str);
    /// Returns AIR if slot number i is empty.
    int  GetInvSub(int i) const;
    /// Returns BLOCK if slot number i is empty.
    int  GetInvKind(int i) const;
    int Size() const;
    int GetInvWeight(int i) const;
    int Number(int i) const;
    Block * ShowBlock(int slot) const;
    /// Don't move block shown by this function.
    Block * ShowBlockInSlot(int slot, int index) const;
    static QString NumStr(int num);

    bool IsEmpty() const;

    void Push(int x, int y, int z, int push_direction);
    /// Stacks items in inventory if possible.
    void Shake();

protected:
    /// It is not recommended to make inventory size more than 26.
    /** Because it will not be convenient to deal with inventory
     *  in console version. */
    explicit Inventory(int sz = INV_SIZE);
    Inventory(QDataStream & str, int size = INV_SIZE);
    virtual ~Inventory();

    virtual void SaveAttributes(QDataStream & out) const;

private:
    const quint8 size;
    QStack<Block *> * const inventory;
};

#endif // INVENTORY_H
