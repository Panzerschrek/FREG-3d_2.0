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

/**\file CursedScreen.h
 * \brief Provides curses (text-based graphics interface) screen for freg.*/

#ifndef CURSEDSCREEN_H
#define CURSEDSCREEN_H

#define NOX

#include <QMutex>

#ifdef Q_OS_WIN32
#include "pdcurses/curses.h"
#else
#define _X_OPEN_SOURCE_EXTENDED
#include <ncursesw/ncurses.h>
#endif
#include "screens/VirtScreen.h"

const int SCREEN_SIZE = 30;

enum actions {
    ACTION_USE,
    ACTION_THROW,
    ACTION_OBTAIN,
    ACTION_INSCRIBE,
    ACTION_BUILD,
    ACTION_CRAFT,
    ACTION_WIELD,
};

enum screen_errors {
    SCREEN_NO_ERROR = 0,
    HEIGHT_NOT_ENOUGH,
    WIDTH_NOT_ENOUGH
};

class IThread;
class Inventory;
class QFile;
class Block;

class Screen final : public VirtScreen {
    Q_OBJECT
public:
     Screen(World *, Player *, int & error, bool ascii);
    ~Screen() override;

    int  GetChar() const override;
    void FlushInput() const override;
    void ControlPlayer(int command) override;

public slots:
    void Notify(QString) const override;
    void PassString(QString &) const override;
    void Update(int, int, int) override;
    void UpdateAll() override;
    void UpdatePlayer() override;
    void UpdateAround(int, int, int, int) override;
    void Move(int) override;
    void DeathScreen();
    void DisplayFile(QString path) override;
    void ActionXyz(int * x, int * y, int * z) const override;

private slots:
    void Print() override;

private:
    Screen(const Screen &) = delete;
    Screen & operator=(const Screen &) = delete;
    char CharNumber(int z) const;
    char CharNumberFront(int x, int y) const;
    void Arrows(WINDOW *, int x, int y, bool show_dir) const;
    void HorizontalArrows(WINDOW *, int y, bool show_dir) const;
    void PrintNormal(WINDOW *, dirs) const;
    void PrintFront(dirs) const;
    void PrintInv(WINDOW *, const Block *, const Inventory *) const;
    /// Can print health, breath and other bars on hudWin.
    void PrintBar(int x, int color, int ch, int percent,
            bool value_position_right = true);
    /// Returns false when file does not exist, otherwise true.
    bool PrintFile(WINDOW *, QString const & file_name);
    void PrintHUD();
    void PrintMiniMap();
    void PrintQuickInventory();
    void CleanFileToShow();
    void RePrint();
    void InventoryAction(int num) const;
    int  Color(int kind, int sub) const;
    int  ColorShred(shred_type)   const;
    char PrintBlock(const Block &, WINDOW *) const;
    void SetActionMode(actions mode);
    void ProcessCommand(QString command);
    void PrintTitle(WINDOW *, dirs) const;
    void MovePlayer(dirs dir);
    void MovePlayerDiag(dirs dir1, dirs dir2) const;
    static bool IsScreenWide();
    int  RandomBlink() const;
    bool RandomBit() const;
    Block * GetFocusedBlock() const;

    WINDOW * windows[6];
    WINDOW *& leftWin    = windows[0];
    WINDOW *& rightWin   = windows[1];
    WINDOW *& notifyWin  = windows[2];
    WINDOW *& hudWin     = windows[3]; // head-up display
    WINDOW *& miniMapWin = windows[4];
    WINDOW *& actionWin  = windows[5];
    mutable QString lastNotification;
    IThread * const input;
    volatile bool updated;
    FILE * const notifyLog;
    actions actionMode;
    /// Can be -1, 0, 1 for low, normal, and high focus.
    int shiftFocus;
    QFile * fileToShow;
    const bool beepOn, flashOn;
    const bool ascii;
    const bool blinkOn;
    const QChar arrows[WEST+1];
    SCREEN * const screen;
    mutable bool inputActive = false;
    mutable int randomBlink;
};

#endif // CURSEDSCREEN_H
