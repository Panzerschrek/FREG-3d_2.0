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

#include <QTime>
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
    switch( e->key() )
    {
    case Qt::Key_Escape:
        emit ExitReceived();
        break;

    case Qt::Key_W:
        move_keys_pressed[ KEY_MOVE_FORWARD ]= is_pressed;
        break;
    case Qt::Key_S:
        move_keys_pressed[ KEY_MOVE_BACK ]= is_pressed;
        break;
    case Qt::Key_A:
        move_keys_pressed[ KEY_MOVE_LEFT ]= is_pressed;
        break;
    case Qt::Key_D:
        move_keys_pressed[ KEY_MOVE_RIGHT ]= is_pressed;
        break;
    case Qt::Key_Space:
        move_keys_pressed[ KEY_MOVE_UP ]= is_pressed;
        break;
    case Qt::Key_C:
        move_keys_pressed[ KEY_MOVE_DOWN ]= is_pressed;
        break;

    case Qt::Key_Left:
        move_keys_pressed[ KEY_TURN_LEFT ]= is_pressed;
        break;
    case Qt::Key_Right:
       move_keys_pressed[ KEY_TURN_RIGHT ]= is_pressed;
       break;
    case Qt::Key_Up:
        move_keys_pressed[ KEY_TURN_UP ]= is_pressed;
        break;
    case Qt::Key_Down:
       move_keys_pressed[ KEY_TURN_DOWN ]= is_pressed;
       break;


    default:
        break;
    };
}

void Screen::CalculateFPS()
{
    this_frame_time= QTime::currentTime();
    frame_time_delta= float( prev_frame_time.msecsTo( this_frame_time ) ) * 0.001f;
    prev_frame_time= this_frame_time;

    int fps_calc_freq= 2;

    if( prev_fps_time.msecsTo( this_frame_time ) > fps_calc_freq * 1000 )
    {
        current_fps= frame_count_in_last_second / fps_calc_freq;
        frame_count_in_last_second= 0;

        prev_fps_time= this_frame_time;
    }

    frame_count_in_last_second++;
}
void Screen::CamMove()
{
    float move_speed= 10.0f;
    float turn_speed= 0.5f;

    float dt= frame_time_delta;

    if( move_keys_pressed[ KEY_TURN_LEFT ] )
    {
        cam_ang.z-= dt * turn_speed;
    }
    if( move_keys_pressed[ KEY_TURN_RIGHT ] )
    {
        cam_ang.z+= dt * turn_speed;
    }
    if( cam_ang.z > m_Math::FM_2PI ) cam_ang.z-= m_Math::FM_2PI;
    else if( cam_ang.z < 0.0f ) cam_ang.z+= m_Math::FM_2PI;

    if( move_keys_pressed[ KEY_TURN_UP ] )
    {
        cam_ang.x-= dt * turn_speed;
    }
    if( move_keys_pressed[ KEY_TURN_DOWN ] )
    {
        cam_ang.x+= dt * turn_speed;
    }
    if( cam_ang.x > m_Math::FM_PI )
        cam_ang.x= m_Math::FM_PI;
    else if( cam_ang.x < 0.0f)
        cam_ang.x= 0.0f;

    if( move_keys_pressed[ KEY_MOVE_FORWARD ] )
    {
        cam_pos.y+= dt*move_speed * m_Math::Cos( cam_ang.z );
        cam_pos.x+= dt*move_speed * m_Math::Sin( cam_ang.z );
    }
    if( move_keys_pressed[ KEY_MOVE_BACK ] )
    {
        cam_pos.y-= dt*move_speed * m_Math::Cos( cam_ang.z );
        cam_pos.x-= dt*move_speed * m_Math::Sin( cam_ang.z );
    }

    if( move_keys_pressed[ KEY_MOVE_LEFT ] )
    {
        cam_pos.y-= dt*move_speed * m_Math::Cos( cam_ang.z + m_Math::FM_PI2 );
        cam_pos.x-= dt*move_speed * m_Math::Sin( cam_ang.z + m_Math::FM_PI2 );
    }
    if( move_keys_pressed[ KEY_MOVE_RIGHT ] )
    {
        cam_pos.y-= dt*move_speed * m_Math::Cos( cam_ang.z - m_Math::FM_PI2 );
        cam_pos.x-= dt*move_speed * m_Math::Sin( cam_ang.z - m_Math::FM_PI2 );
    }

    if( move_keys_pressed[ KEY_MOVE_UP ] )
    {
        cam_pos.z+= dt*move_speed;
    }
    if( move_keys_pressed[ KEY_MOVE_DOWN ] )
    {
        cam_pos.z-= dt*move_speed;
    }
}
void Screen::Repaint()
{
    CalculateFPS();

    CamMove();
    renderer->SetViewportSize( screen_size_x, screen_size_y );
    renderer->SetCamPos( cam_pos );
    renderer->SetCamAng( cam_ang );
    renderer->Draw();

    QPainter p( window );
    p.drawImage( QPoint(0,0), *image );


    QString txt( "fps: " );
    txt+= QVariant( current_fps ).toString();
    p.setFont( QFont( "courier new", 12 ) );
    p.setPen( QColor( Qt::white ) );
    p.drawText( 10, 30, txt );
}
void Screen::OnMouseEvent( QMouseEvent* e, bool is_pressed)
{
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
        ascii(_ascii),

      move_keys_pressed{false},
      cam_pos(0.0f,0.0f,64.0f), cam_ang(m_Math::FM_PI2,0.0f,0.0f),
      prev_frame_time( QTime::currentTime() ),
      prev_fps_time( QTime::currentTime() ),
      frame_count_in_last_second(0)
{
    screen_size_x= 1024;
    screen_size_y= 768;

    window= new FregMainWindow(this);
    window->setFixedSize( screen_size_x, screen_size_y );
    window->show();


    image= new QImage(screen_size_x, screen_size_y, QImage::Format_RGB32 );

    PRast_Init( screen_size_x, screen_size_y );
    PRast_SetFramebuffer( (unsigned char*) image->constBits() );

    renderer= new s_WorldRenderer(wor);

    cam_pos.x= pl->GlobalX();
    cam_pos.y= pl->GlobalY();
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
    QThread::currentThread()->usleep( 1000 );
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

