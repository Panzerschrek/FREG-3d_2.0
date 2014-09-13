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

/**\file Block.h
 * \brief Provides definition for class Block. */

#ifndef BLOCK_H
#define BLOCK_H

#include "header.h"

enum wearable {
    WEARABLE_NOWHERE,
    WEARABLE_HEAD,
    WEARABLE_BODY,
    WEARABLE_LEGS,
    WEARABLE_OTHER,
    WEARABLE_VESSEL
};

enum damage_kinds {
    // Put new damage kinds before DAMAGE_PUSH_UP.
    DAMAGE_MINE,    ///<  0
    DAMAGE_DIG,     ///<  1
    DAMAGE_CUT,     ///<  2
    DAMAGE_THRUST,  ///<  3
    DAMAGE_CRUSH,   ///<  4
    DAMAGE_HEAT,    ///<  5
    DAMAGE_FREEZE,  ///<  6
    DAMAGE_ELECTRO, ///<  7
    DAMAGE_HUNGER,  ///<  8
    DAMAGE_BREATH,  ///<  9
    DAMAGE_BITE,    ///< 10
    DAMAGE_TIME,    ///< 11
    DAMAGE_NO,      ///< 12
    DAMAGE_HANDS,   ///< 13
    DAMAGE_ACID,    ///< 14
    DAMAGE_RADIATION,
    DAMAGE_ULTIMATE,
    DAMAGE_PUSH_UP,
    DAMAGE_PUSH_DOWN,
    DAMAGE_PUSH_NORTH,
    DAMAGE_PUSH_SOUTH,
    DAMAGE_PUSH_EAST,
    DAMAGE_PUSH_WEST
};

enum weights { ///< weights in measures - mz (mezuro)
    WEIGHT_NULLSTONE = 1000,
    WEIGHT_WATER     =  500,
    WEIGHT_IRON      =  300,
    WEIGHT_STONE     =  200,
    WEIGHT_GLASS     =  150,
    WEIGHT_SAND      =  100,
    WEIGHT_GREENERY  =    3,
    WEIGHT_MINIMAL   =    1,
    WEIGHT_AIR       =    0,
};

enum sub_groups {
    GROUP_NONE,
    GROUP_AIR,
    GROUP_MEAT,
};

class Inventory;
class Active;
class Falling;
class Animal;

class Block {
    /**\class Block Block.h
     * \brief Block without special physics and attributes. */
public:
    Block(int sub, int kind, int transp = UNDEF);
    Block(QDataStream &, int sub, int kind, int transp = UNDEF);
    virtual ~Block();

    Block & operator=(Block &) = delete;
    Block(Block &) = delete;

    virtual QString FullName() const;
    virtual bool Catchable() const;
    /// Returns true on success.
    virtual bool Inscribe(QString str);
    virtual void Move(dirs direction);
    virtual void Damage(int dmg, int dmg_kind);
    virtual usage_types Use(Block * user);
    virtual usage_types UseOnShredMove(Block * user);
    virtual push_reaction PushResult(dirs) const;
    /// Should return dropped block.
    /** It can be pile(BOX, DIFFERENT) containing all dropped blocks, or
     *  block itself.
     *  Set delete_self false if this block itself should not be deleted.
     *  (by default block is deleted, beware). */
    virtual Block * DropAfterDamage(bool * delete_self);

    virtual Inventory * HasInventory();
    virtual Animal * IsAnimal();
    virtual Active * ActiveBlock();
    virtual Falling * ShouldFall();

    virtual wearable Wearable() const;
    virtual int DamageKind() const;
    virtual int DamageLevel() const;

    virtual int LightRadius() const;
    virtual int Weight() const;
    /// Receive text signal.
    virtual void ReceiveSignal(QString);

    /// Determines kind and sub, unique for every kind-sub pair.
    int GetId() const;
    /// Set maximum durability.
    void Restore();
    /// Set durability to null.
    void Break();
    /// Increase durability, no more than MAX_DURABILITY.
    void Mend(int plus);
    void SetDir(int dir);

    dirs GetDir() const;
    int GetDurability() const;
    QString GetNote() const;
    inline int Transparent() const { return transparent; }
    inline int Sub()  const { return sub; }
    inline int Kind() const { return kind; }

    bool operator==(const Block &) const;
    bool operator!=(const Block &) const;

    /// Important! If block will be used after save,
    /// call RestoreDurabilityAfterSave.
    void SaveToFile(QDataStream & out);
    void SaveNormalToFile(QDataStream & out) const;
    /// Importart! Use it if block won't be deleted after SaveToFile.
    void RestoreDurabilityAfterSave();

    /// Returns translated substance name.
    static QString SubName(int sub);
    /// Returns translated substance name with first upper letter.
    static QString SubNameUpper(int sub);
    static sub_groups GetSubGroup(int sub);
    static dirs MakeDirFromDamage(int damage_kind);
    static QString DirString(dirs);

protected:
    virtual void SaveAttributes(QDataStream &) const;
    /// To convert DAMAGE_PUSH_UP...WEST to corresponding direction.

    quint16 noteId;

private:
    int Transparency(int transp, int sub) const;

    qint16 durability;
    const quint8 transparent;
    const quint8 kind;
    const quint8 sub;
    quint8 direction;
};

#endif // BLOCK_H
