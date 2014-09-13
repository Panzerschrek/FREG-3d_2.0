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

#ifndef ITHREAD_H
#define ITHREAD_H

/** \class IThread IThread.h
 *  \brief Keyboard input thread for curses screen for freg.
 *
 * This class is thread, with IThread::run containing input loop.
 * Can be used in console-based freg screens. */

#include <QThread>

class VirtScreen;

class IThread final : public QThread {
    Q_OBJECT
public:
    IThread(VirtScreen * const);
    void Stop();

protected:
    void run() override;

private:
    IThread(const IThread &) = delete;
    IThread & operator=(const IThread &) = delete;

    VirtScreen * const screen;
    volatile bool stopped;
};

#endif // ITHREAD_H
