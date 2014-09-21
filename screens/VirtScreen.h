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

#ifndef VIRTSCREEN_H
#define VIRTSCREEN_H

#include <QObject>
#include <QSettings>
#include <QString>
#include "World.h"

class Player;

enum color_pairs { // do not change colors order! // foreground_background
    BLACK_BLACK = 1,
    BLACK_RED,
    BLACK_GREEN,
    BLACK_YELLOW,
    BLACK_BLUE,
    BLACK_MAGENTA,
    BLACK_CYAN,
    BLACK_WHITE,
    //
    RED_BLACK,
    RED_RED,
    RED_GREEN,
    RED_YELLOW,
    RED_BLUE,
    RED_MAGENTA,
    RED_CYAN,
    RED_WHITE,
    //
    GREEN_BLACK,
    GREEN_RED,
    GREEN_GREEN,
    GREEN_YELLOW,
    GREEN_BLUE,
    GREEN_MAGENTA,
    GREEN_CYAN,
    GREEN_WHITE,
    //
    YELLOW_BLACK,
    YELLOW_RED,
    YELLOW_GREEN,
    YELLOW_YELLOW,
    YELLOW_BLUE,
    YELLOW_MAGENTA,
    YELLOW_CYAN,
    YELLOW_WHITE,
    //
    BLUE_BLACK,
    BLUE_RED,
    BLUE_GREEN,
    BLUE_YELLOW,
    BLUE_BLUE,
    BLUE_MAGENTA,
    BLUE_CYAN,
    BLUE_WHITE,
    //
    MAGENTA_BLACK,
    MAGENTA_RED,
    MAGENTA_GREEN,
    MAGENTA_YELLOW,
    MAGENTA_BLUE,
    MAGENTA_MAGENTA,
    MAGENTA_CYAN,
    MAGENTA_WHITE,
    //
    CYAN_BLACK,
    CYAN_RED,
    CYAN_GREEN,
    CYAN_YELLOW,
    CYAN_BLUE,
    CYAN_MAGENTA,
    CYAN_CYAN,
    CYAN_WHITE,
    //
    WHITE_BLACK,
    WHITE_RED,
    WHITE_GREEN,
    WHITE_YELLOW,
    WHITE_BLUE,
    WHITE_MAGENTA,
    WHITE_CYAN,
    WHITE_WHITE
}; // enum color_pairs

class VirtScreen : public QObject {
    /** \class VirtScreen VirtScreen.h
     *  \brief This class provides base for all screens for freg.
     *
     * It provides interface for world-screen and player-screen
     * communications by its slots and signals. */
    Q_OBJECT

public:
    /// Constructor makes player and world connections.
    /** Constructor of non-virtual screen should contain this code
     *  to connect to player for sending input:
     *  connect(this, SIGNAL(InputReceived(int, int)),
     *     player, SLOT(Act(int, int)),
     *     Qt::DirectConnection); */
    VirtScreen(World *, Player *);
    VirtScreen(VirtScreen &) = delete;
    virtual ~VirtScreen();

    // Functions for text screens:
    virtual int  GetChar() const;
    virtual void FlushInput() const;
    virtual void ControlPlayer(int command);

signals:
    /// This is emitted when input receives exit key.
    /** This is connected to application exit. */
    void ExitReceived();

public slots:
    /// This is called for a notification to be displayed.
    virtual void Notify(QString) const = 0;

    /// This is called when string is needed to be received from input.
    /** It is connected to world in constructor. */
    virtual void PassString(QString &) const = 0;

    /// This is called when block at (x, y, z) should be updated in screen.
    /** When implemented, this should work fast.
     *  It is connected to world in constructor. */
    virtual void Update(int x, int y, int z) = 0;

    /// This is called when all world should be updated in sceen.
    /** When implemented, this should work fast.
     *  It is connected to world in constructor. */
    virtual void UpdateAll() = 0;

    /// Called when world loaded zone is moved to update world in screen.
    /** When implemented, this should work fast.
     *  It is connected to world in constructor. */
    virtual void Move(int) = 0;

    /// Called when some player property needs to be updated in screen.
    /** When implemented, this should work fast.
     *  It is connected to world in constructor. */
    virtual void UpdatePlayer() = 0;

    /// Called when area around xyz with range needs to be updated.
    /** When implemented, this should work fast.
     *  It is connected to world in constructor. */
    virtual void UpdateAround(int x, int y, int z, int rng) = 0;

    /// This is called when current group of updates is ended.
    /** This is called from world when pack of world changing is ended.
     *  ( Can be used in screen optimization. ) */
    virtual void UpdatesEnd();

    /// This is called when player is dead, and displayed until respawn.
    void DeathScreen(); // virtual ?

    /// Used to get player focus coordinates from screen.
    /** x, y, z are coordinates where player will make action.
     *  May be reimplemented in derivative class to get xyz other than
     *  world direction-based focus. */
    virtual void ActionXyz(int * x, int * y, int * z) const;

    /// This shows a file by path.
    /** Standard (non-reimpemented) version does nothing. */
    virtual void DisplayFile(QString path);

private slots:
    /// Prints world. Should not be called not within screen.
    virtual void Print() = 0;

protected:
    World * GetWorld() const;
    static char CharName(int kind, int sub);

    World  * const w;
    Player * const player;
    QSettings settings;
    QString previousCommand;

private:
    VirtScreen(const VirtScreen &) = delete;
    VirtScreen & operator=(const VirtScreen &) = delete;
};

#endif // VIRTSCREEN_H
