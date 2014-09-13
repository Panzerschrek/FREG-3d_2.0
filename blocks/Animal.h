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

#ifndef ANIMAL_H
#define ANIMAL_H

#include "blocks/Active.h"

class DeferredAction;

class Animal : public Falling {
    Q_OBJECT
public:
     Animal(int sub, int id);
     Animal(QDataStream & str, int sub, int id);
    ~Animal();

    int  DamageKind() const override;
    int  ShouldAct() const override;
    void ActFrequent() override;
    void DoRareAction() override;
    bool Eat(subs);
    int  Breath() const;
    int  Satiation() const;
    QString FullName() const override = 0;
    Animal * IsAnimal() override;
    Block  * DropAfterDamage(bool * delete_block) override;
    inner_actions ActInner() override;

    void SetDeferredAction(DeferredAction *);

protected:
    void SaveAttributes(QDataStream & out) const override;
    void EatGrass();

    bool moved_in_this_turn = false;

private:
    Animal(Animal &) = delete;
    Animal & operator=(Animal &) = delete;
    virtual int NutritionalValue(subs) const;

    quint8  breath;
    quint16 satiation;
    DeferredAction * deferredAction = nullptr;
}; // class Animal

class Predator : public Animal {
    Q_OBJECT
public:
    using Animal::Animal;

    int DamageLevel() const override;
    void ActFrequent() override;
    QString FullName() const override;

protected:
    void DoRareAction() override;
    int  Attractive(int sub) const override;

private:
    int NutritionalValue(subs) const override;
}; // class Predator

#endif // ANIMAL_H
