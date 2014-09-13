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

#include <QPainter>
#include <QKeyEvent>
#include <QMouseEvent>

#include "3dSoftScreen.h"
#include "IThread.h"
#include "world.h"
#include "blocks/Block.h"
#include "blocks/Inventory.h"
#include "Player.h"

#include "3d_soft_screen/psr/rasterization.h"
#include "3d_soft_Screen/s_world_renderer.h"

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


void Screen::OnKeyEvent( QKeyEvent* e, bool is_pressed)
{
    if( e->key() == Qt::Key_Escape )
        emit ExitReceived();
}


void Screen::Repaint()
{
    renderer->Draw();


    QPainter p( window );
    p.drawImage( QPoint(0,0), *image );
}
void Screen::OnMouseEvent( QMouseEvent* e, bool is_pressed)
{
    printf( "mouse\n");
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
    screen_size_x= 640;
    screen_size_y= 480;

    window= new FregMainWindow(this);
    window->setFixedSize( screen_size_x, screen_size_y );
    window->show();


    image= new QImage(screen_size_x, screen_size_y, QImage::Format_RGB32 );

    PRast_Init( screen_size_x, screen_size_y );
    PRast_SetFramebuffer( (unsigned char*) image->constBits() );

    renderer= new s_WorldRenderer(wor);
}


FregMainWindow::FregMainWindow(Screen* s):
screen(s)
{
}
FregMainWindow::~FregMainWindow()
{
}

void FregMainWindow::keyPressEvent( QKeyEvent* e)
{


    screen->OnKeyEvent(e,true);
}
void FregMainWindow::mousePressEvent(QMouseEvent * e)
{
    screen->OnMouseEvent(e,true);
}

void FregMainWindow::keyReleaseEvent( QKeyEvent* e)
{
    screen->OnKeyEvent(e,false);
}
void FregMainWindow::mouseReleaseEvent(QMouseEvent * e)
{
    screen->OnMouseEvent(e,false);
}

void FregMainWindow::paintEvent( QPaintEvent* event)
{
    screen->Repaint();
    QWidget::update();
    QThread::currentThread()->usleep( 5000 );
}
void Screen::CleanAll()
{
}

Screen::~Screen()
{
    delete renderer;
    delete image;
    delete window;

}

