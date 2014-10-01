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

/**\file VirtScreen.cpp
 * \brief This file provides definitions of code common to all freg screens. */

#include "VirtScreen.h"
#include "Player.h"

void VirtScreen::UpdatesEnd() {}
void VirtScreen::DeathScreen() {}

VirtScreen::VirtScreen(World * const world_, Player * const player_) :
        w(world_),
        player(player_),
        settings(home_path + "freg.ini", QSettings::IniFormat),
        previousCommand(settings.value("last_command", "moo").toString())
{
    connect(     w, SIGNAL(Notify(QString)), SLOT(Notify(QString)),
        Qt::DirectConnection);
    connect(player, SIGNAL(Notify(QString)), SLOT(Notify(QString)),
        Qt::DirectConnection);
    connect(player, SIGNAL(ShowFile(QString)), SLOT(DisplayFile(QString)));
    connect(player, SIGNAL(GetFocus(int *, int *, int *)),
        SLOT(ActionXyz(int *, int *, int *)), Qt::DirectConnection);

    connect(     w, SIGNAL(GetString(QString &)),
        SLOT(PassString(QString &)), Qt::DirectConnection);
    connect(player, SIGNAL(GetString(QString &)),
        SLOT(PassString(QString &)), Qt::DirectConnection);

    connect(player, SIGNAL(Updated()), SLOT(UpdatePlayer()),
        Qt::DirectConnection);
    connect(w, SIGNAL(UpdatedAll()), SLOT(UpdateAll()),
        Qt::DirectConnection);
    connect(w, SIGNAL(Moved(int)), SLOT(Move(int)),
        Qt::DirectConnection);
    connect(w, SIGNAL(Updated(int, int, int)), SLOT(Update(int, int, int)),
        Qt::DirectConnection);
    connect(w, SIGNAL(UpdatedAround(int, int, int, int)),
        SLOT(UpdateAround(int, int, int, int)),
        Qt::DirectConnection);
    connect(w, SIGNAL(UpdatesEnded()), SLOT(UpdatesEnd()),
        Qt::DirectConnection);

    connect(player, SIGNAL(Destroyed()), SLOT(DeathScreen()),
        Qt::DirectConnection );
}

VirtScreen::~VirtScreen() {}

int  VirtScreen::GetChar() const { return 0; }
void VirtScreen::FlushInput() const {}
void VirtScreen::ControlPlayer(int) {}
void VirtScreen::DisplayFile(QString /* path */) {}

void VirtScreen::ActionXyz(int * x, int * y, int * z) const {
    w->Focus(player->X(), player->Y(), player->Z(), x, y, z, player->GetDir());
}

World * VirtScreen::GetWorld() const { return w; }

int VirtScreen::Color(const int kind, const int sub) {
    switch ( kind ) { // foreground_background
    case LIQUID: switch ( sub ) {
        case WATER:     return CYAN_BLUE;
        case SUB_CLOUD: return BLACK_WHITE;
        case ACID:      return GREEN_GREEN;
        case H_MEAT:
        case A_MEAT:    return BLACK_RED;
        default:        return RED_YELLOW;
        } break;
    case FALLING: switch ( sub ) {
        case WATER: return   CYAN_WHITE;
        case SAND:  return YELLOW_WHITE;
        } // no break;
    default: switch ( sub ) {
        default:         return WHITE_BLACK;
        case STONE:      return BLACK_WHITE;
        case GREENERY:   return BLACK_GREEN;
        case WOOD:
        case HAZELNUT:
        case SOIL:       return   BLACK_YELLOW;
        case SAND:       return   WHITE_YELLOW;
        case COAL:       return   BLACK_WHITE;
        case IRON:       return   BLACK_BLACK;
        case A_MEAT:     return   WHITE_RED;
        case H_MEAT:     return   BLACK_RED;
        case WATER:      return   WHITE_CYAN;
        case GLASS:      return    BLUE_WHITE;
        case NULLSTONE:  return MAGENTA_BLACK;
        case MOSS_STONE: return   GREEN_WHITE;
        case ROSE:       return     RED_GREEN;
        case CLAY:       return   WHITE_RED;
        case PAPER:      return MAGENTA_WHITE;
        case GOLD:       return   WHITE_YELLOW;
        case BONE:       return MAGENTA_WHITE;
        case EXPLOSIVE:  return   WHITE_RED;
        case DIAMOND:    return    CYAN_WHITE;
        case ADAMANTINE: return    CYAN_BLACK;
        case SKY:
        case STAR:       return   WHITE_BLACK;
        case SUB_DUST:   return   BLACK_BLACK;
        case FIRE:       return     RED_YELLOW;
        case ACID:       return   GREEN_GREEN;
        }
    case DWARF: switch ( sub ) {
        case ADAMANTINE: return  CYAN_BLACK;
        default:         return WHITE_BLUE ;
        }
    case RABBIT:    return  RED_WHITE;
    case PREDATOR:  return  RED_BLACK;
    case TELEGRAPH: return BLUE_BLUE;
    case MEDKIT:    return  RED_WHITE;
    }
} // color_pairs Screen::Color(int kind, int sub)

char VirtScreen::CharName(const int kind, const int sub) {
    switch ( static_cast<kinds>(kind) )  {
    case BUSH:   return ';';
    case DWARF:  return '@';
    case LIQUID: return '~';
    case GRASS:  return ( FIRE == sub ) ? 'f' : '.';
    case RABBIT: return 'r';
    case CLOCK:  return 'c';
    case PLATE:  return '_';
    case LADDER: return '^';
    case PICK:   return '\\';
    case SHOVEL: return '|';
    case HAMMER: return 'T';
    case AXE:    return '/';
    case BELL:   return 'b';
    case BUCKET: return 'u';
    case MAP:
    case KIND_TEXT: return '?';
    case PREDATOR:  return '!';
    case WORKBENCH: return '*';
    case CONVERTER: return 'V';
    case BOX:       return '&';
    case CONTAINER: return '&';
    case DOOR:        return ( STONE == sub ) ? '#' : '\'';
    case ILLUMINATOR: return 'i';
    case WEAPON: switch ( sub ) {
        default:    return '/';
        case STONE: return '.';
        case SKY:   return ' ';
    } break;
    case ARMOUR: return 'A';
    case HELMET: return 'H';
    case BOOTS:  return 'B';
    case TELEGRAPH: return 't';
    case MEDKIT:    return '+';
    case FILTER:    return 'F';
    case INFORMER:  return 'I';
    case RAIN_MACHINE: return 'R';
    case FALLING: switch ( sub ) {
        case SAND:  return '.';
        case WATER: return '*';
        case STONE: return ':';
    } // no break;
    case BLOCK: switch ( sub ) {
        default:    return '#';
        case SOIL:  return '.';
        case WATER: return '~';
        case GREENERY: return '%';
        case A_MEAT:
        case H_MEAT:
        case HAZELNUT: return ',';
        case GLASS: return 'g';
        case ROSE:  return ';';
        case COAL:  return '*';
        case STAR:  return '.';
        case SKY:
        case AIR:   return ' ';
        } break;
    case LAST_KIND: break;
    }
    Q_UNREACHABLE();
} // char VirtScreen::CharName(int kind, int sub)
