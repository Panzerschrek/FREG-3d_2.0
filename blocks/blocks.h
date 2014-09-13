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

#ifndef BLOCKS_H
#define BLOCKS_H

#include "Inventory.h"
#include "blocks/Animal.h"

class Plate : public Block {
public:
    using Block::Block;

    int Weight() const override;
    QString FullName() const override;
    push_reaction PushResult(dirs) const override;
};

class Ladder : public Block {
public:
    using Block::Block;

    bool Catchable() const override;
    int  Weight() const override;
    QString FullName() const override;
    Block * DropAfterDamage(bool * delete_block) override;
    push_reaction PushResult(dirs) const override;
};

class Liquid : public Falling {
    Q_OBJECT
public:
    using Falling::Falling;

    int ShouldAct() const override;
    int DamageKind() const override;
    int LightRadius() const override;
    bool Inscribe(QString) override;
    void Damage(int dmg, int dmg_kind) override;
    QString FullName() const override;
    Block * DropAfterDamage(bool * delete_block) override;
    wearable Wearable() const override;
    push_reaction PushResult(dirs) const override;
    inner_actions ActInner() override;

protected:
    void DoRareAction() override;
};

class Grass : public Active {
    Q_OBJECT
public:
    using Active::Active;

    int  ShouldAct() const override;
    int  DamageKind() const override;
    int  LightRadius() const override;
    QString FullName() const override;
    Block * DropAfterDamage(bool * delete_block) override;
    inner_actions ActInner() override;

protected:
    void DoRareAction() override;

private:
    static bool IsBase(int ownsub, int ground);
};

class Bush : public Active, public Inventory {
    Q_OBJECT

    static const int BUSH_SIZE = 3;
public:
    Bush(int sub, int id);
    Bush(QDataStream & str, int sub, int id);

    int  Weight() const override;
    int  ShouldAct() const override;
    void ReceiveSignal(QString) override;
    void Damage(int dmg, int dmg_kind) override;
    QString FullName() const override;
    Block * DropAfterDamage(bool * delete_block) override;
    Inventory * HasInventory() override;
    usage_types Use(Block * who) override;
    inner_actions ActInner() override;

protected:
    void DoRareAction() override;
    void SaveAttributes(QDataStream & out) const override;
};

class Rabbit : public Animal {
    Q_OBJECT
public:
    using Animal::Animal;

    QString FullName() const override;
    void ActFrequent() override;

protected:
    void DoRareAction() override;
    int  Attractive(int sub) const override;

private:
    int NutritionalValue(subs) const override;
};

class Door : public Active {
    Q_OBJECT
public:
    Door(int sub, int id);
    Door(QDataStream & str, int sub, int id);

    int  ShouldAct() const override;
    void ActFrequent() override;
    void Damage(int dmg, int dmg_kind) override;
    QString FullName() const override;
    usage_types Use(Block * who) override;
    push_reaction PushResult(dirs) const override;

protected:
    void SaveAttributes(QDataStream & out) const override;

private:
    bool shifted;
    bool locked;
    push_reaction movable = NOT_MOVABLE;
};

class Clock : public Active {
    Q_OBJECT
public:
    Clock(int sub, int id);
    Clock (QDataStream & str, int sub, int id);

    int  Weight() const override;
    int  ShouldAct() const override;
    bool Inscribe(QString) override;
    void Damage(int dmg, int dmg_kind) override;
    QString FullName() const override;
    wearable Wearable() const override;
    usage_types Use(Block * who) override;
    inner_actions ActInner() override;

protected:
    void SaveAttributes(QDataStream &) const override;

private:
    int alarmTime = -1;
    int timerTime = -1;
};

class Text : public Block {
public:
    using Block::Block;

    bool Inscribe(QString) override;
    QString FullName() const override;
    usage_types Use(Block * who) override;
};

class Map : public Text {
public:
    Map(int sub, int id);
    Map(QDataStream & str, int sub, int id);

    QString FullName() const override;
    wearable Wearable() const override;
    usage_types Use(Block * who) override;
    usage_types UseOnShredMove(Block * who) override;

protected:
    void SaveAttributes(QDataStream & out) const override;

private:
    /// coordinates map titled in. also ~center.
    qint64 longiStart, latiStart;
    quint16 savedShift;
    qint8 savedChar;
};

class Bell : public Active {
    Q_OBJECT
public:
    using Active::Active;

    void Damage(int dmg, int dmg_kind) override;
    QString FullName() const override;
    wearable Wearable() const override;
    usage_types Use(Block * who) override;
};

class Telegraph : public Active {
    Q_OBJECT
public:
    Telegraph(int sub, int id);
    Telegraph(QDataStream &, int sub, int id);

    int ShouldAct() const override;
    bool Inscribe(QString) override;
    void ReceiveSignal(QString) override;
    QString FullName() const override;
    wearable Wearable() const override;
    inner_actions ActInner() override;
    usage_types Use(Block * who) override;

protected:
    void SaveAttributes(QDataStream &) const override;

private:
    static QString sharedMessage;
    bool isReceiver;
};

class MedKit : public Block {
public:
    using Block::Block;
    QString FullName() const override;
    wearable Wearable() const override;
    usage_types Use(Block * user) override;
};

/** @brief The Informer class, provides various information.
 *
 * Can be used inside inventory and outside.
 * Iron informer is compass. */
class Informer : public Block {
public:
    using Block::Block;

    QString FullName() const override;
    wearable Wearable() const override;
    usage_types Use(Block * user) override;
};

#endif // BLOCKS_H
