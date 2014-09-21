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

#ifndef ACTIVE_H
#define ACTIVE_H

#include <QObject>
#include "Block.h"
#include "Xyz.h"

/// Frequency can be "never", "rare", "rare & first", "rare & second".
enum active_frequency {
    FREQUENT_NEVER  = 0,
    FREQUENT_RARE   = 1,
    FREQUENT_FIRST  = 2,
    FREQUENT_SECOND = 4,
};

/// See Shred::PhysEventsRare() for details.
enum inner_actions {
    INNER_ACTION_ONLY,
    INNER_ACTION_NONE,
    INNER_ACTION_EXPLODE, // like in Fallout
    INNER_ACTION_MESSAGE
};

class Shred;
class World;

class Active : public QObject, public Block, protected Xyz {
    Q_OBJECT
public:
    Active(int sub, int id, int transp = UNDEF);
    Active(QDataStream & str, int sub, int id, int transp = UNDEF);

    int X() const;
    int Y() const;
    using Xyz::Z;
    using Xyz::SetXyz;

    void Move(dirs dir) override;
    void ReceiveSignal(QString) override;
    void Damage(int dmg, int dmg_kind) override;
    QString  FullName() const override = 0;
    Active * ActiveBlock() override;

    Shred * GetShred() const;
    World * GetWorld() const;

    virtual void ActFrequent();
    void ActRare();

    virtual inner_actions ActInner();
    virtual int ShouldAct() const;

    void SetShred(Shred *);
    void Farewell();
    void Unregister();

signals:
    void Moved(int);
    void Updated();
    void ReceivedText(const QString);

protected:
    void SendSignalAround(QString) const;
    void DamageAround() const;
    /// Damages block and destroys it if it is broken.
    void TryDestroy(int x, int y, int z) const;
    /// Returns true if there is at least 1 block of substance sub around.
    bool IsSubAround(int sub) const;
    bool Gravitate(int range, int down, int up, int calmness);

    virtual void DoRareAction();
    virtual int  Attractive(int sub) const;

private:
    Active(Active &) = delete;
    Active & operator=(Active &) = delete;
    void ReRegister(dirs);

    Shred * shred = nullptr;
}; // Active

class Falling : public Active {
    Q_OBJECT
public:
    Falling(int sub, int id, int transp = UNDEF);
    Falling(QDataStream & str, int sub, int id, int transp = UNDEF);

    void Move(dirs dir) override;
    QString FullName() const override;
    Falling * ShouldFall() override final;
    push_reaction PushResult(dirs) const override;

    void FallDamage();
    bool IsFalling() const;
    void SetFalling(bool set);

protected:
    void SaveAttributes(QDataStream & out) const override;

private:
    quint8 fallHeight;
    bool falling;
}; // Falling

#endif // ACTIVE_H
