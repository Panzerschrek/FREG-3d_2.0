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

/**\file CursedScreen.cpp
 * \brief This file is related to curses screen for freg. */

#include <QDir>
#include <QLocale>
#include <QMutexLocker>
#include "screens/CursedScreen.h"
#include "screens/IThread.h"
#include "Shred.h"
#include "blocks/Block.h"
#include "blocks/Inventory.h"
#include "Player.h"

const char OBSCURE_BLOCK = ' ';
const int QUICK_INVENTORY_X_SHIFT = 36;

void Screen::Arrows(WINDOW * const window, const int x, const int y,
        const bool show_dir)
const {
    if ( show_dir ) {
        wcolor_set(window, WHITE_BLACK, nullptr);
        mvwaddstr(window, 0, x-2, qPrintable(tr("N    N")));
        mvwaddstr(window, SCREEN_SIZE+1, x-2, qPrintable(tr("S    S")));
    }
    wcolor_set(window, WHITE_RED, nullptr);
    mvwaddstr(window, 0, x, qPrintable(arrows[SOUTH]));
    waddstr  (window,       qPrintable(arrows[SOUTH]));
    mvwaddstr(window, SCREEN_SIZE+1, x, qPrintable(arrows[NORTH]));
    waddstr  (window, qPrintable(arrows[NORTH]));
    HorizontalArrows(window, y, show_dir);
    (void)wmove(window, y, x);
}

void Screen::HorizontalArrows(WINDOW * const window, const int y,
        const bool show_dir)
const {
    wcolor_set(window, WHITE_BLACK, nullptr);
    if ( show_dir ) {
        mvwaddstr(window, y-1, 0, qPrintable(tr("W")));
        mvwaddstr(window, y+1, 0, qPrintable(tr("W")));
        mvwaddstr(window, y-1, SCREEN_SIZE*2+1, qPrintable(tr("E")));
        mvwaddstr(window, y+1, SCREEN_SIZE*2+1, qPrintable(tr("E")));
    }
    wcolor_set(window, WHITE_RED, nullptr);
    mvwaddstr(window, y, 0, qPrintable(arrows[EAST]));
    mvwaddstr(window, y, SCREEN_SIZE*2+1, qPrintable(arrows[WEST]));
}

void Screen::RePrint() {
    clear();
    mvwaddstr(actionWin, 0, 0, qPrintable(tr("Use")));
    mvwaddstr(actionWin, 1, 0, qPrintable(tr("Throw")));
    mvwaddstr(actionWin, 2, 0, qPrintable(tr("Obtain")));
    mvwaddstr(actionWin, 3, 0, qPrintable(tr("iNscribe")));
    mvwaddstr(actionWin, 4, 0, qPrintable(tr("Build")));
    mvwaddstr(actionWin, 5, 0, qPrintable(tr("Craft")));
    mvwaddstr(actionWin, 6, 0, qPrintable(tr("Equipment")));
    refresh();
    wrefresh(actionWin);
    updated = false;
}

void Screen::Update(int, int, int) { updated = false; }
void Screen::UpdatePlayer() { updated = false; }
void Screen::UpdateAround(int, int, int, int) { updated = false; }
void Screen::Move(int) { updated = false; }
bool Screen::IsScreenWide() { return COLS >= (SCREEN_SIZE*2+2)*2; }

void Screen::UpdateAll() {
    CleanFileToShow();
    updated = false;
}

void Screen::PassString(QString & str) const {
    inputActive = true;
    wstandend(notifyWin);
    waddch(notifyWin, ':');
    char temp_str[MAX_NOTE_LENGTH + 1];
    echo();
    wgetnstr(notifyWin, temp_str, MAX_NOTE_LENGTH);
    inputActive = false;
    noecho();
    lastNotification = str;
    fprintf(notifyLog, "%lu: Command: %s\n", w->Time(), temp_str);
    str = QString::fromUtf8(temp_str);
}

char Screen::CharNumber(const int z) const {
    if ( HEIGHT-1 == z ) { // sky
        return ' ';
    }
    const int z_dif = ( UP==player->GetDir() ) ?
        z - player->Z() : player->Z() - z;
    return ( z_dif == 0 ) ?
        ' ' : ( z_dif<0 ) ?
            '-' : ( z_dif<10 ) ?
                z_dif+'0' : '+';
}

char Screen::CharNumberFront(const int i, const int j) const {
    const int dist = (( NORTH==player->GetDir() || SOUTH==player->GetDir() ) ?
        abs(player->Y()-j) :
        abs(player->X()-i)) -1;
    return ( dist>9 ) ?
        '+' : ( dist>0 ) ?
            dist+'0' : ' ';
}

int  Screen::RandomBlink() const { return RandomBit() ? 0 : A_REVERSE; }
bool Screen::RandomBit()   const { return (randomBlink >>= 1) & blinkOn; }

int Screen::Color(const int kind, const int sub) const {
    const int color = COLOR_PAIR(VirtScreen::Color(kind, sub));
    switch ( kind ) { // foreground_background
    case TELEGRAPH: return color | A_BOLD;
    case LIQUID: switch ( sub ) {
        case H_MEAT:
        case A_MEAT:
        case SUB_CLOUD: return color;
        default:        return color | RandomBlink();
        case ACID:      return color | RandomBlink() | A_BOLD;
        } break;
    default: switch ( sub ) {
        case GOLD:       return color | RandomBlink();
        case IRON:
        case NULLSTONE:
        case DIAMOND:    return color | A_BOLD;
        case ACID:
        case SUB_DUST:   return color | A_BOLD  | A_REVERSE;
        case FIRE:       return color | A_BLINK | RandomBlink();
        case SKY:
        case STAR:
            if ( w->GetEvernight() ) return COLOR_PAIR(BLACK_BLACK);
            switch ( w->PartOfDay() ) {
            case TIME_NIGHT: return
                COLOR_PAIR(WHITE_BLACK) | ( RandomBit() ? A_BOLD : 0 );
            case TIME_MORNING: return COLOR_PAIR(WHITE_BLUE);
            case TIME_NOON:    return COLOR_PAIR( CYAN_CYAN);
            case TIME_EVENING: return COLOR_PAIR(WHITE_CYAN);
            } break;
        default: return color;
        } break;
    }
} // color_pairs Screen::Color(int kind, int sub)

int Screen::ColorShred(const shred_type type) const {
    switch ( type ) { // foreground_background
    case SHRED_NORMAL_UNDERGROUND:
    case SHRED_TESTSHRED:
    case SHRED_EMPTY:
    case SHRED_CHAOS:       return COLOR_PAIR( WHITE_BLACK);
    case SHRED_DEAD_FOREST: return COLOR_PAIR(YELLOW_BLACK);
    case SHRED_DEAD_HILL:   return COLOR_PAIR( BLACK_WHITE) | A_BOLD;
    case SHRED_PYRAMID:
    case SHRED_WASTE:
    case SHRED_CASTLE:
    case SHRED_MOUNTAIN:  return Color(BLOCK,  STONE);
    case SHRED_PLAIN:     return Color(BLOCK,  GREENERY);
    case SHRED_DESERT:    return Color(BLOCK,  SAND);
    case SHRED_WATER:     return Color(LIQUID, WATER);
    case SHRED_LAVA_LAKE: return Color(LIQUID, STONE);
    case SHRED_ACID_LAKE: return Color(LIQUID, ACID);
    case SHRED_CRATER:    return COLOR_PAIR( WHITE_WHITE) | A_BOLD;
    case SHRED_FOREST:    return COLOR_PAIR(YELLOW_GREEN);
    case SHRED_HILL:      return COLOR_PAIR( WHITE_GREEN);
    case SHRED_NULLMOUNTAIN: return Color(BLOCK, NULLSTONE);
    }
    Q_UNREACHABLE();
    return COLOR_PAIR(WHITE_BLACK);
}

void Screen::MovePlayer(const dirs dir) {
    if ( player->GetDir() == dir ) {
        player->Move(dir);
    } else {
        player->SetDir(dir);
    }
}

void Screen::MovePlayerDiag(const dirs dir1, const dirs dir2) const {
    player->SetDir(dir1);
    static bool step_trigger = true;
    player->Move(step_trigger ? dir1 : dir2);
    step_trigger = !step_trigger;
}

int Screen::GetChar() const { return getch(); }
void Screen::FlushInput() const { flushinp(); }

void Screen::ControlPlayer(const int ch) {
    CleanFileToShow();
    /// \todo: ctrl-z (to background) support
    // Q, ctrl-c, ctrl-d, ctrl-q, ctrl-x
    if ( 'Q' == ch
            || 3 == ch
            || 4 == ch
            || 17 == ch
            || 24 == ch
            || 'X' == ch
            || KEY_F(10) == ch )
    {
        emit ExitReceived();
        return;
    } // else:
    if ( 'a'<=ch && ch<='z' ) { // actions with inventory
        InventoryAction(ch-'a');
        return;
    } // else:
    switch ( ch ) { // interactions with world
    default:
        Notify(tr("Unknown key. Press 'H' for help."));
        #ifndef QT_NO_DEBUG
        Notify(QString("Pressed key code: %1.").arg(ch));
        #endif
        break;
    case 'W': case KEY_UP:    case '8': MovePlayer(NORTH);  break;
    case 'S': case KEY_DOWN:  case '2': MovePlayer(SOUTH);  break;
    case 'D': case KEY_RIGHT: case '6': MovePlayer(EAST);   break;
    case 'A': case KEY_LEFT:  case '4': MovePlayer(WEST);   break;
    case KEY_END:   case '5': player->Move(DOWN); break;
    case '7': MovePlayerDiag(NORTH, WEST); break;
    case '9': MovePlayerDiag(NORTH, EAST); break;
    case '1': MovePlayerDiag(SOUTH, WEST); break;
    case '3': MovePlayerDiag(SOUTH, EAST); break;
    case '=':
    case '0': MovePlayer(player->GetDir()); break;
    case ' ': player->Jump(); break;

    case '>': player->SetDir(World::TurnRight(player->GetDir())); break;
    case '<': player->SetDir(World::TurnLeft (player->GetDir())); break;
    case 'V':
    case KEY_NPAGE: player->SetDir(DOWN); break;
    case '^':
    case KEY_PPAGE: player->SetDir(UP);   break;

    case 'I':
    case KEY_HOME: player->Backpack(); break;
    case 8:
    case KEY_F(8):
    case KEY_DC: // delete
    case KEY_BACKSPACE: player->Damage(); break;
    case '\n':
    case 13:
    case KEY_F(2):
    case 'F': player->Use();      break;
    case '*':
    case KEY_F(3):
    case '?': player->Examine();  break;
    case KEY_F(4):
    case '~': player->Inscribe(); break;
    case 27: /* esc */ player->StopUseAll(); break;

    case KEY_IC: player->Build(1); break; // insert
    case 'B': SetActionMode(ACTION_BUILD);    break;
    case 'C': SetActionMode(ACTION_CRAFT);    break;
    case 'T': SetActionMode(ACTION_THROW);    break;
    case 'N': SetActionMode(ACTION_INSCRIBE); break;
    case 'G':
    case 'O': SetActionMode(ACTION_OBTAIN);   break;
    case 'E': SetActionMode(ACTION_WIELD);    break;
    case 'U': SetActionMode(ACTION_USE);      break;
    case 'Z':
        if ( player->PlayerInventory() ) {
              player->PlayerInventory()->Shake();
        }
        break;
    case KEY_HELP:
    case KEY_F(1):
    case 'H': ProcessCommand("help"); break;
    case KEY_F(5):
    case 'R':
    case 'L': RePrint(); break;

    case '-': shiftFocus = -!shiftFocus; break; // move focus down
    case '+': shiftFocus =  !shiftFocus; break; // move focus up

    case KEY_F(12):
    case '!': player->SetCreativeMode( not player->GetCreativeMode() ); break;
    case KEY_F(9):
    case ':':
    case '/': PassString(previousCommand); // no break
    case '.': ProcessCommand(previousCommand); break;
    }
    updated = false;
} // void Screen::ControlPlayer(int ch)

void Screen::ProcessCommand(const QString command) {
    if ( command.length()==1 && command.at(0)!='.' ) {
        ControlPlayer(command.at(0).toLatin1());
        return;
    }
    switch ( Player::UniqueIntFromString(qPrintable(command)) ) {
    case Player::UniqueIntFromString("size"):
        Notify(QString("Terminal height: %1 lines, width: %2 chars.").
            arg(LINES).arg(COLS));
        break;
    case Player::UniqueIntFromString("moo"):
        Notify("^__^");
        Notify("(oo)\\_______");
        Notify("(__)\\       )\\/\\");
        Notify("    ||----w |");
        Notify("    ||     ||");
        break;
    default: player->ProcessCommand(command); break;
    }
}

void Screen::SetActionMode(const actions mode) {
    mvwchgat(actionWin, actionMode,      0,20, A_NORMAL, WHITE_BLACK, nullptr);
    mvwchgat(actionWin, actionMode=mode, 0,20, A_NORMAL, BLACK_WHITE, nullptr);
    wrefresh(actionWin);
    updated = false;
}

void Screen::InventoryAction(const int num) const {
    switch ( actionMode ) {
    case ACTION_USE:      player->Use     (num); break;
    case ACTION_THROW:    player->Throw   (num); break;
    case ACTION_OBTAIN:   player->Obtain  (num); break;
    case ACTION_INSCRIBE: player->Inscribe(num); break;
    case ACTION_BUILD:    player->Build   (num); break;
    case ACTION_CRAFT:    player->Craft   (num); break;
    case ACTION_WIELD:    player->Wield   (num); break;
    }
}

void Screen::ActionXyz(int * x, int * y, int * z) const {
    VirtScreen::ActionXyz(x, y, z);
    if (
            DOWN != player->GetDir() &&
            UP   != player->GetDir() &&
            ( AIR==w->GetBlock(*x, *y, *z)->Sub() || AIR==w->GetBlock(
                player->X(),
                player->Y(),
                player->Z()+shiftFocus)->Sub() ))
    {
        *z += shiftFocus;
    }
}

Block * Screen::GetFocusedBlock() const {
    int x, y, z;
    ActionXyz(&x, &y, &z);
    return GetWorld()->GetBlock(x, y, z);
}

char Screen::PrintBlock(const Block & block, WINDOW * const window) const {
    const int kind = block.Kind();
    const int sub  = block.Sub();
    wattrset(window, Color(kind, sub));
    return CharName(kind, sub);
}

void Screen::Print() {
    if ( updated ) return;
    updated = true;
    w->Lock();
    PrintHUD();
    const dirs dir = player->GetDir();
    if ( player->UsingSelfType() != USAGE_TYPE_OPEN ) { // left window
        PrintNormal(leftWin, (UP==dir || DOWN==dir) ? NORTH : dir);
    } else {
        PrintInv(leftWin, player->GetBlock(), player->PlayerInventory());
    }
    if ( fileToShow == nullptr ) { // right window
        switch ( player->UsingType() ) {
        default:
            if ( UP==dir || DOWN==dir ) {
                PrintNormal(rightWin, dir);
            } else if ( rightWin != nullptr ) {
                PrintFront(dir);
            }
            break;
        case USAGE_TYPE_READ_IN_INVENTORY:
            DisplayFile(QString(home_path + w->WorldName() + "/texts/"
                + player->PlayerInventory()->ShowBlock(
                    player->GetUsingInInventory())->GetNote()));
            player->SetUsingTypeNo();
            break;
        case USAGE_TYPE_READ:
            DisplayFile(QString(home_path + w->WorldName()
                + "/texts/" + GetFocusedBlock()->GetNote()));
            player->SetUsingTypeNo();
            break;
        case USAGE_TYPE_OPEN: {
                Block * const block = GetFocusedBlock();
                PrintInv(rightWin, block, block->HasInventory());
            } break;
        }
    }
    w->Unlock();
} // void Screen::Print()

void Screen::PrintHUD() {
    werase(hudWin);
    if ( player->GetCreativeMode() ) {
        mvwaddstr (hudWin, 0, 0,
            qPrintable(tr("Creative Mode\nxyz: %1, %2, %3. XY: %4, %5.")
            .arg(player->GlobalX()).arg(player->GlobalY()).arg(player->Z())
            .arg(player->GetLatitude()).arg(player->GetLongitude())) );
    } else {
        const int dur = player->GetBlock()->GetDurability();
        if ( dur > 0 ) { // HitPoints line
            PrintBar(0,
                (dur > MAX_DURABILITY/5) ?
                    COLOR_PAIR(RED_BLACK) : (COLOR_PAIR(BLACK_RED) | A_BLINK),
                ascii ? '@' : 0x2665, dur*100/MAX_DURABILITY);
        }
        const int breath = player->BreathPercent();
        if ( breath != 100 ) {
            PrintBar(16, COLOR_PAIR(BLUE_BLACK), ascii ? 'o' : 0x00b0, breath);
        }
        switch ( player->SatiationPercent()/25 ) { // satiation status
        default: break;
        case  0:
            wcolor_set(hudWin, RED_BLACK, nullptr);
            mvwaddstr(hudWin, 1, 1, qPrintable(tr("Hungry")));
            break;
        case  3:
            wcolor_set(hudWin, GREEN_BLACK, nullptr);
            mvwaddstr(hudWin, 1, 1, qPrintable(tr("Full")));
            break;
        case  4:
            wcolor_set(hudWin, BLUE_BLACK, nullptr);
            mvwaddstr(hudWin, 1, 1, qPrintable(tr("Gorged")));
            break;
        }
    }
    Block * const focused = GetFocusedBlock();
    if ( Block::GetSubGroup(focused->Sub()) != GROUP_AIR ) {
        const int left_border = IsScreenWide() ?
            (SCREEN_SIZE*2+2) * 2 :
            SCREEN_SIZE*2+2 - 15;
        PrintBar(left_border - 15,
            Color(focused->Kind(), focused->Sub()),
            (focused->IsAnimal() == nullptr) ? '+' : '*',
            focused->GetDurability()*100/MAX_DURABILITY,
            false);
        const QString name = focused->FullName();
        mvwaddstr(hudWin, 1, left_border-name.length(),
            qPrintable(focused->FullName()));
        const QString note = focused->GetNote();
        if ( not note.isEmpty() ) {
            const int width = qMin(36, note.length() + 2);
            (void)wmove(hudWin, 2, left_border - width);
            if ( note.length()+2 <= width ) {
                wprintw(hudWin, "~:%s", qPrintable(note));
            } else {
                wprintw(hudWin, "~:%s ...", qPrintable(note.left(width - 6)));
            }
        }
    }
    PrintQuickInventory();
    wrefresh(hudWin);
    PrintMiniMap();
} // void Screen::PrintHUD()

void Screen::PrintQuickInventory() {
    Inventory * const inv = player->PlayerInventory();
    if ( inv!=nullptr && IsScreenWide() ) {
        for (int i=inv->Size()-1; i>=0; --i) {
            wstandend(hudWin);
            const int x = QUICK_INVENTORY_X_SHIFT+i*2;
            mvwaddch(hudWin, 0, x, 'a'+i);
            switch ( inv->Number(i) ) {
            case  0: break;
            default: mvwaddch(hudWin, 2, x, inv->Number(i)+'0'); // no break;
            case  1: mvwaddch(hudWin, 1, x,
                    PrintBlock(*inv->ShowBlock(i), hudWin));
                break;
            }
        }
    }
}

void Screen::PrintMiniMap() {
    (void)wmove(miniMapWin, 1, 0);
    const int x_center = Shred::CoordOfShred(player->X());
    const int y_center = Shred::CoordOfShred(player->Y());
    const int j_start = x_center - 2;
    const int j_end   = x_center + 2;
    const int i_end   = y_center + 2;
    for (int i=y_center-2; i<=i_end; ++i, waddch(miniMapWin, '\n'))
    for (int j=j_start;    j<=j_end; ++j) {
        if ( i<0 || j<0 || i>=w->NumShreds() || j>=w->NumShreds() ) {
            wstandend(miniMapWin);
            waddstr  (miniMapWin, "  ");
        } else {
            Shred * const shred = w->GetShredByPos(j, i);
            wattrset(miniMapWin, ColorShred(shred->GetTypeOfShred()));
            wprintw (miniMapWin, " %c", shred->GetTypeOfShred());
        }
    }
    wstandend(miniMapWin);
    box(miniMapWin, 0, 0);
    wrefresh(miniMapWin);
}

void Screen::PrintNormal(WINDOW * const window, const dirs dir) const {
    int k_start, k_step;
    if ( UP == dir ) {
        k_start = player->Z() + 1;
        k_step  = 1;
    } else {
        k_start = player->Z() - ( DOWN==dir );
        k_step  = -1;
    }
    (void)wmove(window, 1, 1);
    const int start_x = ( player->X()/SHRED_WIDTH )*SHRED_WIDTH +
        ( SHRED_WIDTH-SCREEN_SIZE )/2;
    const int start_y = ( player->Y()/SHRED_WIDTH )*SHRED_WIDTH +
        ( SHRED_WIDTH-SCREEN_SIZE )/2;
    for (int j=start_y; j<SCREEN_SIZE+start_y; ++j, waddstr(window, "\n_"))
    for (int i=start_x; i<SCREEN_SIZE+start_x; ++i ) {
        randomBlink = qrand();
        Shred * const shred = w->GetShred(i, j);
        const int i_in = Shred::CoordInShred(i);
        const int j_in = Shred::CoordInShred(j);
        int k = k_start;
        for ( ; INVISIBLE == shred->GetBlock(i_in, j_in, k)->Transparent();
            k += k_step);
        if ( player->Visible(i, j, k) ) {
            waddch(window, PrintBlock(*shred->GetBlock(i_in,j_in,k), window));
            waddch(window, CharNumber(k));
        } else {
            wstandend(window);
            waddch(window, OBSCURE_BLOCK);
            waddch(window, ' ');
        }
    }

    if ( dir > DOWN ) {
        const Block * const block = player->GetBlock();
        wattrset(window, Color(block->Kind(), block->Sub()));
        mvwaddstr(window, player->Y()-start_y+1, (player->X()-start_x)*2+2,
            qPrintable(arrows[player->GetDir()]));
    }

    PrintTitle(window, UP==dir ? UP : DOWN);
    Arrows(window, (player->X()-start_x)*2+1, player->Y()-start_y+1, true);
    wrefresh(window);
} // void Screen::PrintNormal(WINDOW * window, int dir)

void Screen::PrintFront(const dirs dir) const {
    const int pX = player->X();
    const int begin_x = ( pX/SHRED_WIDTH )*SHRED_WIDTH +
        ( SHRED_WIDTH-SCREEN_SIZE )/2;
    const int pY = player->Y();
    const int begin_y = ( pY/SHRED_WIDTH )*SHRED_WIDTH +
        ( SHRED_WIDTH-SCREEN_SIZE )/2;
    int x_step,  z_step,
        x_start, z_start,
        x_end,   z_end;
    int * x, * z;
    int i, j;
    int arrow_X;
    switch ( dir ) {
    case NORTH:
        x = &i;
        x_step  = 1;
        x_start = begin_x;
        x_end   = x_start + SCREEN_SIZE;
        z = &j;
        z_step  = -1;
        z_start = pY - 1;
        z_end   = qMax(0, pY - SHRED_WIDTH*2);
        arrow_X = (pX - begin_x)*2 + 1;
        break;
    case SOUTH:
        x = &i;
        x_step  = -1;
        x_start = SCREEN_SIZE - 1 + begin_x;
        x_end   = begin_x - 1;
        z = &j;
        z_step  = 1;
        z_start = pY + 1;
        z_end   = qMin(pY+SHRED_WIDTH*2, w->GetBound());
        arrow_X = (SCREEN_SIZE - pX + begin_x)*2 - 1;
        break;
    case EAST:
        x = &j;
        x_step  = 1;
        x_start = begin_y;
        x_end   = SCREEN_SIZE + begin_y;
        z = &i;
        z_step  = 1;
        z_start = pX + 1;
        z_end   = qMin(pX + SHRED_WIDTH*2, w->GetBound());
        arrow_X = (pY - begin_y)*2 + 1;
        break;
    case WEST:
        x = &j;
        x_step  = -1;
        x_start = SCREEN_SIZE - 1 + begin_y;
        x_end   = begin_y - 1;
        z = &i;
        z_step  = -1;
        z_start = pX - 1;
        z_end   = qMax(0, pX - SHRED_WIDTH*2);
        arrow_X = (SCREEN_SIZE - pY + begin_y)*2 - 1;
        break;
    default:
        Q_UNREACHABLE();
        return;
    }
    const int k_start =
        qBound(SCREEN_SIZE-1, player->Z()+SCREEN_SIZE/2, HEIGHT-1);
    const int sky_color = Color(BLOCK, SKY);
    (void)wmove(rightWin, 1, 1);
    for (int k=k_start; k>k_start-SCREEN_SIZE; --k, waddstr(rightWin, "\n_")) {
        for (*x=x_start; *x!=x_end; *x+=x_step) {
            randomBlink = qrand();
            for (*z=z_start; *z!=z_end && w->GetBlock(i, j, k)->
                        Transparent()==INVISIBLE;
                    *z += z_step);
            if ( *z == z_end ) {
                static const int sky_char = CharName(BLOCK, SKY);
                wattrset(rightWin, sky_color);
                waddch(rightWin, sky_char);
                waddch(rightWin, ' ');
            } else if ( player->Visible(i, j, k) ) {
                const Block * const block = w->GetBlock(i, j, k);
                waddch(rightWin, PrintBlock(*block, rightWin));
                waddch(rightWin, CharNumberFront(i, j));
            } else {
                wattrset(rightWin, COLOR_PAIR(WHITE_BLACK));
                waddch(rightWin, OBSCURE_BLOCK);
                waddch(rightWin, ' ');
            }
        }
    }
    PrintTitle(rightWin, dir);
    const int arrow_Y = k_start + 1 - player->Z();
    if ( shiftFocus ) {
        wattrset(rightWin, COLOR_PAIR(WHITE_BLUE));
        const int ch = ( shiftFocus == 1 ) ? '^' : 'v';
        for (int q=arrow_Y-shiftFocus; 0<q && q<=SCREEN_SIZE; q-=shiftFocus) {
            mvwaddch(rightWin, q,               0, ch);
            mvwaddch(rightWin, q, SCREEN_SIZE*2+1, ch);
        }
    }
    Arrows(rightWin, arrow_X, arrow_Y, false);
    wrefresh(rightWin);
} // void Screen::PrintFront(dirs)

void Screen::PrintTitle(WINDOW * const window, const dirs dir) const {
    QString dir_string = QString("%1 %2 %1").
        arg(arrows[dir]).
        arg(Block::DirString(dir));
    wstandend(window);
    box(window, 0, 0);
    wcolor_set(window, BLACK_WHITE, nullptr);
    mvwaddstr(window, 0, 1, qPrintable(dir_string));
}

void Screen::PrintInv(WINDOW * const window,
        const Block * const block, const Inventory * const inv)
const {
    if ( inv == nullptr ) return;
    werase(window);
    const int start = inv->Start();
    int shift = 0; // to divide inventory sections
    for (int i=0; i<inv->Size(); ++i) {
        shift += ( start == i && i != 0 );
        wstandend(window);
        mvwprintw(window, 2+i+shift, 1, "%c) ", 'a'+i);
        const int number = inv->Number(i);
        if ( 0 == number ) {
            wattrset(window, COLOR_PAIR(BLACK_BLACK) | A_BOLD);
            waddstr(window, qPrintable(inv->InvFullName(i)));
            continue;
        }
        const Block * const block = inv->ShowBlock(i);
        wprintw(window, "[%c]%s",
            PrintBlock(*block, window), qPrintable(inv->InvFullName(i)) );
        if ( 1 < number ) {
            waddstr(window, qPrintable(Inventory::NumStr(number)));
        }
        if ( MAX_DURABILITY != block->GetDurability() ) {
            wprintw(window, "{%d}", block->GetDurability()*100/MAX_DURABILITY);
        }
        const QString str = block->GetNote();
        if ( not str.isEmpty() ) {
            const int x = getcurx(window);
            const int width = SCREEN_SIZE*2 - x - 3 - 8;
            if ( str.length() <= width ) {
                wprintw(window, " ~:%s", qPrintable(str));
            } else {
                wprintw(window, " ~:%s ...", qPrintable(str.left(width - 4)));
            }
        }
        wstandend(window);
        mvwprintw(window, 2+i+shift, 53, "%5hu mz", inv->GetInvWeight(i));
    }
    wstandend(window);
    QString full_weight = tr("Full weight: %1 mz").
        arg(inv->Weight(), 6, 10, QChar(' '));
    mvwprintw(window, 2+inv->Size()+shift,
        SCREEN_SIZE*2 + 1 - full_weight.length(), qPrintable(full_weight));
    wattrset(window, Color(block->Kind(), block->Sub()));
    box(window, 0, 0);
    if ( start != 0 ) {
        mvwhline(window, 2+start, 1, ACS_HLINE, SCREEN_SIZE*2);
    }
    mvwprintw(window, 0, 1, "[%c] %s", CharName( block->Kind(), block->Sub()),
        qPrintable((player->PlayerInventory() == inv) ?
            tr("Your inventory") : block->FullName()) );
    wrefresh(window);
} // void Screen::PrintInv(WINDOW *, const Block *, const Inventory *)

void Screen::CleanFileToShow() {
    delete fileToShow;
    fileToShow = 0;
}

bool Screen::PrintFile(WINDOW * const window, QString const & file_name) {
    CleanFileToShow();
    fileToShow = new QFile(file_name);
    if ( fileToShow->open(QIODevice::ReadOnly | QIODevice::Text) ) {
        werase(window);
        waddstr(window, qPrintable(
            QString::fromLocal8Bit(fileToShow->readAll().constData())) );
        wrefresh(window);
        return true;
    } else {
        CleanFileToShow();
        return false;
    }
}

void Screen::DisplayFile(QString path) {
    wstandend(rightWin);
    if ( not PrintFile(rightWin, path) ) {
        Notify(tr("There is no such file."));
    }
}

void Screen::Notify(const QString str) const {
    fputs(qPrintable(QString("%1 %2\n").arg(w->TimeOfDayStr()).arg(str)),
        notifyLog);
    if ( inputActive ) return;
    switch ( str.at(str.size()-1).unicode() ) {
    default: wstandend(notifyWin); break;
    case '!': wcolor_set(notifyWin, RED_BLACK, nullptr); // no break;
    case '*': if ( flashOn ) flash(); break;
    case '^': if (  beepOn ) beep();  break;
    }
    static int notification_repeat_count = 1;
    if ( str == lastNotification ) {
        ++notification_repeat_count;
        mvwprintw(notifyWin, getcury(notifyWin)-1, 0, "%s (%dx)\n",
            qPrintable(str), notification_repeat_count);
    } else {
        notification_repeat_count = 1;
        waddstr(notifyWin, qPrintable(lastNotification = str));
        waddch(notifyWin, '\n');
    }
    wrefresh(notifyWin);
}

void Screen::DeathScreen() {
    werase(rightWin);
    werase(hudWin);
    wcolor_set(leftWin, WHITE_RED, nullptr);
    if ( not PrintFile(leftWin, ":/texts/death.txt") ) {
        waddstr(leftWin, qPrintable(tr("You die.\nWaiting for respawn...")));
    }
    box(leftWin, 0, 0);
    wnoutrefresh(leftWin);
    wnoutrefresh(rightWin);
    wnoutrefresh(hudWin);
    doupdate();
    updated = true;
}

Screen::Screen(
        World  * const wor,
        Player * const pl,
        int & error,
        bool _ascii)
    :
        VirtScreen(wor, pl),
        lastNotification(),
        input(new IThread(this)),
        updated(),
        notifyLog(fopen(qPrintable(home_path + "log.txt"), "at")),
        actionMode(ACTION_USE),
        shiftFocus(settings.value("focus_shift", 0).toInt()),
        fileToShow(nullptr),
        beepOn (settings.value("beep_on",  false).toBool()),
        flashOn(settings.value("flash_on", true ).toBool()),
        ascii(_ascii),
        blinkOn(settings.value("blink_on", true).toBool()),
        arrows{'.', 'x',
            ascii ? '^' : 0x2191,
            ascii ? 'v' : 0x2193,
            ascii ? '>' : 0x2192,
            ascii ? '<' : 0x2190
        },
        screen(newterm(nullptr, stdout, stdin)),
        randomBlink()
{
    #ifndef Q_OS_WIN32
        set_escdelay(10);
    #endif
    start_color();
    raw(); // send typed keys directly
    noecho(); // do not print typed symbols
    nonl();
    keypad(stdscr, TRUE); // use arrows
    memset(windows, 0, sizeof(windows));
    if ( LINES < 41 && IsScreenWide() ) {
        printf("Make your terminal height to be at least 41 lines.\n");
        error = HEIGHT_NOT_ENOUGH;
        return;
    } else if ( LINES < 39 ) {
        printf("Make your terminal height to be at least 39 lines.\n");
        error = HEIGHT_NOT_ENOUGH;
    }
    // all available color pairs (maybe some of them will not be used)
    const int colors[] = { // do not change colors order!
        COLOR_BLACK,
        COLOR_RED,
        COLOR_GREEN,
        COLOR_YELLOW,
        COLOR_BLUE,
        COLOR_MAGENTA,
        COLOR_CYAN,
        COLOR_WHITE
    };
    for (int i=BLACK_BLACK; i<=WHITE_WHITE; ++i) {
        init_pair(i, colors[(i-1)/8], colors[(i-1)%8]);
    }
    const int preferred_width = (SCREEN_SIZE*2+2)*2;
    if ( IsScreenWide() ) {
        const int left_border = COLS/2-SCREEN_SIZE*2-2;
        rightWin  = newwin(SCREEN_SIZE+2, SCREEN_SIZE*2+2, 0, COLS/2);
        leftWin   = newwin(SCREEN_SIZE+2, SCREEN_SIZE*2+2, 0, left_border);
        hudWin    = newwin(3, preferred_width, SCREEN_SIZE+2, left_border);
        notifyWin = newwin(0,preferred_width-13, SCREEN_SIZE+5,left_border+33);
        miniMapWin = newwin(7, 11, SCREEN_SIZE+5, left_border  );
        actionWin  = newwin(7, 20, SCREEN_SIZE+5, left_border+12);
    } else if ( COLS >= preferred_width/2 ) {
        const int left_border = COLS/2-SCREEN_SIZE-1;
        leftWin   = newwin(SCREEN_SIZE+2, SCREEN_SIZE*2+2, 0, left_border);
        hudWin    = newwin(2,SCREEN_SIZE*2+2-15, SCREEN_SIZE+2,left_border+15);
        notifyWin = newwin(21, SCREEN_SIZE*2+2-15,
            SCREEN_SIZE+2+2, left_border+15);
        actionWin = newwin(0, 15, SCREEN_SIZE+2, left_border);
    } else {
        puts(qPrintable(tr("Set your terminal width at least %1 chars.").
            arg(SCREEN_SIZE*2+2)));
        error = WIDTH_NOT_ENOUGH;
        return;
    }
    scrollok(notifyWin, TRUE);

    if ( not PrintFile(stdscr, ":/texts/splash.txt") ) {
        addstr("Free-Roaming Elementary Game\nby mmaulwurff\n");
    }
    addstr(qPrintable(tr("\nVersion %1.\n\nPress any key.").arg(VER)));
    qsrand(getch());
    CleanFileToShow();
    RePrint();
    SetActionMode(static_cast<actions>
        (settings.value("action_mode", ACTION_USE).toInt()));
    Print();
    Notify(tr("--- Game started. Press 'H' for help. ---"));
    if ( not IsScreenWide() ) {
        Notify(tr("For better gameplay set your"));
        Notify(tr("terminal width at least %1 chars.").arg(preferred_width));
    }

    input->start();
    connect(wor, SIGNAL(UpdatesEnded()), SLOT(Print()), Qt::DirectConnection);
} // Screen::Screen(World * wor, Player * pl)

Screen::~Screen() {
    w->Lock();
    disconnect(w, SIGNAL(UpdatesEnded()), this, SLOT(Print()));
    w->Unlock();

    input->Stop();
    input->wait();
    delete input;

    for (ulong i=0; i<sizeof(windows)/sizeof(windows[0]); ++i) {
        delwin(windows[i]);
    }
    endwin();
    delscreen(screen);
    if ( notifyLog ) {
        fclose(notifyLog);
    }
    delete fileToShow;
    settings.setValue("focus_shift", shiftFocus);
    settings.setValue("action_mode", actionMode);
    settings.setValue("last_command", previousCommand);
    settings.setValue("beep_on",  beepOn);
    settings.setValue("flash_on", flashOn);
    settings.setValue("blink_on", blinkOn);
}

void Screen::PrintBar(const int x, const int attr, const int ch,
        const int percent, const bool value_position_right)
{
    wstandend(hudWin);
    mvwprintw(hudWin, 0, x,
        value_position_right ? "[..........]%hd" : "%3hd[..........]",percent);
    wattrset(hudWin, attr);
    const QString str(10, QChar(ch));
    mvwaddstr(hudWin, 0, x + (not value_position_right ? 4 : 1),
        qPrintable(str.left(percent/10)));
}
