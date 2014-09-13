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

/**\file TextScreen.cpp
 * \brief This file is related to text screen for freg. */

#include <QTimer>
#include <QSettings>
#include <QDir>
#include <QMutex>
#include <QLocale>
#include "TextScreen.h"
#include "IThread.h"
#include "world.h"
#include "blocks/Block.h"
#include "blocks/Inventory.h"
#include "Player.h"

void Screen::Update(int, int, int) {}
void Screen::UpdatePlayer() {}
void Screen::UpdateAll() {}
void Screen::UpdateAround(int, int, int, int) {}
void Screen::Move(int) {}

void Screen::PassString(QString & str) const {
    char temp_str[MAX_NOTE_LENGTH+1];
    fgets(temp_str, MAX_NOTE_LENGTH, stdin);
    fprintf(notifyLog, "%lu: Command: %s\n", w->Time(), temp_str);
    str = QString::fromUtf8(temp_str);
}

int  Screen::GetChar() const { return getchar(); }
void Screen::FlushInput() const {}

void Screen::MovePlayer(const int dir) const {

}

void Screen::MovePlayerDiag(const int dir1, const int dir2) const {

}

void Screen::ControlPlayer(const int ch) {

}

void Screen::SetActionMode(const actions mode) { actionMode = mode; }

void Screen::InventoryAction(const ushort num) const {

}

void Screen::ActionXyz(int * x, int * y, int * z) const {

}

void Screen::Print() {}

void Screen::CleanFileToShow() {

}

bool Screen::PrintFile(QString const & file_name) {

}

void Screen::DisplayFile(QString path) {

}

void Screen::Notify(const QString str) const {

}

void Screen::DeathScreen() {

}

Screen::Screen(
        World  * const wor,
        Player * const pl,
        int &, // error
        bool _ascii)
    :
        VirtScreen(wor, pl),
        input(new IThread(this)),
        timer(new QTimer(this)),
        notifyLog(fopen("texts/messages.txt", "at")),
        fileToShow(nullptr),
        ascii(_ascii)
{

}

void Screen::CleanAll() {


}

Screen::~Screen() { CleanAll(); }
