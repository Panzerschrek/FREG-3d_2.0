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

#include <QTextStream>
#include <QFile>
#include <QTime>
#include "blocks.h"
#include "World.h"
#include "Shred.h"
#include "BlockManager.h"

// Plate::
    QString Plate::FullName() const {
        switch ( Sub() ) {
        case WOOD:  return QObject::tr("Wooden board");
        case MOSS_STONE:
        case STONE: return QObject::tr("Stone slab");
        default:    return QObject::tr("Plate (%1)").arg(SubName(Sub()));
        }
    }

    int Plate::Weight() const { return Block::Weight()/4; }
    push_reaction Plate::PushResult(dirs) const { return JUMP; }

// Ladder::
    QString Ladder::FullName() const {
        switch ( Sub() ) {
        case MOSS_STONE:
        case STONE:    return QObject::tr("Rock with ledges");
        case GREENERY: return QObject::tr("Liana");
        default:       return QObject::tr("Ladder (%1)").arg(SubName(Sub()));
    }
    }

    bool Ladder::Catchable() const { return true; }
    int  Ladder::Weight() const { return Block::Weight()*3; }
    push_reaction Ladder::PushResult(dirs) const { return MOVE_UP; }

    Block * Ladder::DropAfterDamage(bool * const delete_block) {
        Block * const pile = BlockManager::NewBlock(BOX, DIFFERENT);
        if ( STONE==Sub() || MOSS_STONE==Sub() ) {
            pile->HasInventory()->Get(block_manager.Normal(Sub()));
        } else {
            pile->HasInventory()->Get(this);
            *delete_block = false;
        }
        return pile;
    }

// Liquid::
    void Liquid::DoRareAction() {
        if ( not IsSubAround(Sub()) || Sub()==SUB_CLOUD ) {
            Damage(MAX_DURABILITY*2/SECONDS_IN_NIGHT, DAMAGE_TIME);
            if ( GetDurability() <= 0 ) {
                GetWorld()->DestroyAndReplace(X(), Y(), Z());
                return;
            }
        }
        switch ( Sub() ) {
        case SUB_CLOUD: return;
        case ACID:
        case STONE: DamageAround(); // no break;
        default: switch ( qrand() % 20 ) {
            case 0: GetWorld()->Move(X(), Y(), Z(), NORTH); break;
            case 1: GetWorld()->Move(X(), Y(), Z(), SOUTH); break;
            case 2: GetWorld()->Move(X(), Y(), Z(), EAST ); break;
            case 3: GetWorld()->Move(X(), Y(), Z(), WEST ); break;
            } break;
        }
    }

    int Liquid::DamageKind() const {
        switch ( Sub() ) {
        default:    return DAMAGE_NO;
        case ACID:  return DAMAGE_ACID;
        case STONE: return DAMAGE_HEAT;
        }
    }

    int  Liquid::ShouldAct() const  { return FREQUENT_RARE; }
    int  Liquid::LightRadius() const { return ( STONE==Sub() ) ? 3 : 0; }
    bool Liquid::Inscribe(QString) { return false; }
    wearable Liquid::Wearable() const { return WEARABLE_VESSEL; }
    inner_actions Liquid::ActInner() { return INNER_ACTION_NONE; }
    push_reaction Liquid::PushResult(dirs) const { return ENVIRONMENT; }

    void Liquid::Damage(const int dmg, const int dmg_kind) {
        if ( dmg_kind < DAMAGE_PUSH_UP ) {
            Falling::Damage(dmg, dmg_kind);
        }
    }

    Block * Liquid::DropAfterDamage(bool *) {
        return block_manager.Normal( ( Sub() == STONE ) ?
            STONE : AIR);
    }

    QString Liquid::FullName() const {
        switch ( Sub() ) {
        case STONE:  return tr("Lava");
        case H_MEAT:
        case A_MEAT: return tr("Blood");
        default:     return SubNameUpper(Sub());
        }
    }

// Grass::
    void Grass::DoRareAction() {
        World * const world = GetWorld();
        if ( FIRE == Sub() ) {
            DamageAround();
            if ( qrand()%10 || IsSubAround(WATER) ) {
                Damage(2, DAMAGE_FREEZE);
            }
        }
        if ( not IsBase(Sub(), world->GetBlock(X(), Y(), Z()-1)->Sub()) ) {
            world->DestroyAndReplace(X(), Y(), Z());
            return;
        } // else:
        int i=X(), j=Y(), k=Z();
        // increase this if grass grows too fast
        switch ( qrand() % (FIRE==Sub() ? 4 : SECONDS_IN_HOUR*2) ) {
        case 0: ++i; break;
        case 1: --i; break;
        case 2: ++j; break;
        case 3: --j; break;
        default: return;
        }
        if ( not world->InBounds(i, j) ) return;
        const int sub_near = world->GetBlock(i, j, k)->Sub();
        if ( world->Enlightened(i, j, k) || FIRE == Sub() ) {
            if ( AIR == sub_near
                    && IsBase(Sub(), world->GetBlock(i, j, k-1)->Sub() ) )
            {
                world->Build(BlockManager::NewBlock(GRASS, Sub()), i, j, k);
            } else if ( IsBase(Sub(), sub_near)
                    && AIR == world->GetBlock(i, j, ++k)->Sub() )
            {
                world->Build(BlockManager::NewBlock(GRASS, Sub()), i, j, k);
            }
        }
    }

    bool Grass::IsBase(const int own_sub, const int ground) {
        return ( GREENERY==own_sub && SOIL==ground )
            || ( FIRE==own_sub && (
                WOOD==ground
                || GREENERY==ground
                || H_MEAT==ground
                || A_MEAT==ground
                || HAZELNUT==ground
                || ROSE==ground
                || PAPER==ground ) );
    }

    QString Grass::FullName() const {
        switch ( Sub() ) {
        case GREENERY: return tr("Grass");
        case FIRE:     return tr("Fire");
        default:       return tr("Plant (%1)").arg(SubName(Sub()));
        }
    }

    int  Grass::ShouldAct() const  { return FREQUENT_RARE; }
    int  Grass::LightRadius() const { return (FIRE == Sub()) ? 5 : 0; }
    inner_actions Grass::ActInner() { return INNER_ACTION_NONE; }
    Block * Grass::DropAfterDamage(bool*) { return block_manager.Normal(AIR); }

    int Grass::DamageKind() const {
        return (Sub() == FIRE) ? DAMAGE_HEAT : DAMAGE_NO;
    }

// Bush::
    int  Bush::ShouldAct() const  { return FREQUENT_RARE; }
    void Bush::ReceiveSignal(const QString str) { Active::ReceiveSignal(str); }
    int  Bush::Weight() const { return Inventory::Weight()+Block::Weight(); }
    QString Bush::FullName() const { return tr("Bush"); }
    usage_types Bush::Use(Block *) { return USAGE_TYPE_OPEN; }
    Inventory * Bush::HasInventory() { return this; }
    inner_actions Bush::ActInner() { return INNER_ACTION_NONE; }

    void Bush::DoRareAction() {
        if ( 0 == qrand()%(SECONDS_IN_HOUR*4) ) {
            Get(block_manager.Normal(HAZELNUT));
        }
    }

    void Bush::Damage(const int dmg, const int dmg_kind) {
        if ( dmg_kind >= DAMAGE_PUSH_UP ) {
            Push(X(), Y(), Z(), dmg_kind);
        } else {
            Block::Damage(dmg, dmg_kind);
        }
    }

    Block * Bush::DropAfterDamage(bool *) {
        Block * const pile = BlockManager::NewBlock(BOX, DIFFERENT);
        Inventory * const pile_inv = pile->HasInventory();
        pile_inv->Get(BlockManager::NewBlock(WEAPON, WOOD));
        pile_inv->Get(block_manager.Normal(HAZELNUT));
        return pile;
    }

    void Bush::SaveAttributes(QDataStream & out) const {
        Active   ::SaveAttributes(out);
        Inventory::SaveAttributes(out);
    }

    Bush::Bush(const int kind, const int sub) :
            Active(kind, sub),
            Inventory(BUSH_SIZE)
    {}

    Bush::Bush(QDataStream & str, const int kind, const int sub) :
            Active(str, kind, sub),
            Inventory(str, BUSH_SIZE)
    {}

// Rabbit::
    int Rabbit::Attractive(const int sub) const {
        switch ( sub ) {
        case GREENERY: return   1;
        case H_MEAT:   return -16;
        case A_MEAT:   return - 1;
        case SAND:     return - 1;
        default:       return   0;
        }
    }

    void Rabbit::DoRareAction() {
        // eat sometimes
        if ( SECONDS_IN_DAY/2 > Satiation() ) {
            EatGrass();
        }
        if ( not moved_in_this_turn ) {
            switch ( qrand()%60 ) {
            case 0: SetDir(NORTH); break;
            case 1: SetDir(SOUTH); break;
            case 2: SetDir(EAST);  break;
            case 3: SetDir(WEST);  break;
            default: if ( Gravitate(4, 1, 3, 4) ) {
                if ( qrand()%2 ) {
                    GetWorld()->Jump(X(), Y(), Z(), GetDir());
                } else {
                    GetWorld()->Move(X(), Y(), Z(), GetDir());
                }
                moved_in_this_turn = false; // for next turn
            } return;
            }
            GetWorld()->Move(X(), Y(), Z(), GetDir());
        }
        moved_in_this_turn = false; // for next turn
        Animal::DoRareAction();
    }

    QString Rabbit::FullName() const { return tr("Herbivore"); }

    void Rabbit::ActFrequent() {
        if ( Gravitate(2, 1, 2, 4) ) {
            if ( qrand()%2 ) {
                world->Jump(X(), Y(), Z(), GetDir());
            } else {
                world->Move(X(), Y(), Z(), GetDir());
            }
            moved_in_this_turn = true;
        }
    }

    int Rabbit::NutritionalValue(const subs sub) const {
        return ( GREENERY == sub ) ? SECONDS_IN_HOUR*4 : 0;
    }

// Door::
    void Door::Damage(const int dmg, const int dmg_kind) {
        if ( dmg_kind >= DAMAGE_PUSH_UP
                && not shifted
                && not locked
                && World::Anti(GetDir()) != MakeDirFromDamage(dmg_kind) )
        {
            movable = MOVABLE;
            shifted = GetWorld()->Move(X(), Y(), Z(), GetDir());
            movable = NOT_MOVABLE;
        }
        Block::Damage(dmg, dmg_kind);
    }

    void Door::ActFrequent() {
        if ( shifted ) {
            World * const world = GetWorld();
            movable = MOVABLE;
            shifted = !world->Move(X(), Y(), Z(), World::Anti(GetDir()));
            movable = NOT_MOVABLE;
        }
    }

    int  Door::ShouldAct() const { return FREQUENT_SECOND; }
    push_reaction Door::PushResult(dirs) const { return movable; }

    QString Door::FullName() const {
        return QString("%1 (%2)").
            arg(locked ? tr("Locked door") : tr("Door")).
            arg(SubName(Sub()));
    }

    usage_types Door::Use(Block *) {
        locked = !locked;
        return USAGE_TYPE_NO;
    }

    void Door::SaveAttributes(QDataStream & out) const {
        Active::SaveAttributes(out);
        out << shifted << locked;
    }

    Door::Door(const int kind, const int sub) :
            Active(kind, sub, ( STONE==sub || MOSS_STONE==sub ) ?
                BLOCK_OPAQUE : NONSTANDARD),
            shifted(false),
            locked(false)
    {}

    Door::Door(QDataStream & str, const int kind, const int sub) :
            Active(str, kind, sub, ( STONE==sub || MOSS_STONE==sub ) ?
                BLOCK_OPAQUE : NONSTANDARD),
            shifted(),
            locked()
    {
        str >> shifted >> locked;
    }

// Clock::
    usage_types Clock::Use(Block * const who) {
        if ( who != nullptr ) {
            who->ReceiveSignal( (GetNote().left(4) == "real") ?
                tr("Outer time is %1.").arg(QTime::currentTime().toString()) :
                GetWorld()->TimeOfDayStr() );
        } else {
            SendSignalAround(GetNote());
        }
        return USAGE_TYPE_NO;
    }

    QString Clock::FullName() const {
        return ( Sub() == EXPLOSIVE ) ?
            tr("Bomb") :
            tr("Clock (%1)").arg(SubName(Sub()));
    }

    void Clock::Damage(int, const int dmg_kind) {
        if ( dmg_kind >= DAMAGE_PUSH_UP ) {
            Use(nullptr);
        } else {
            alarmTime = timerTime = -1;
            Break();
        }
    }

    int Clock::ShouldAct() const  { return FREQUENT_RARE; }
    int Clock::Weight() const { return Block::Weight()/10; }
    wearable Clock::Wearable() const { return WEARABLE_OTHER; }

    inner_actions Clock::ActInner() {
        const int current_time = GetWorld()->TimeOfDay();
        int notify_flag = 1;
        if ( alarmTime == current_time ) {
            Block::Inscribe(tr("Alarm. %1").arg(GetWorld()->TimeOfDayStr()));
            ++notify_flag;
        } else if ( timerTime > 0 )  {
            --timerTime;
            Block::Inscribe(QString().setNum(timerTime));
        } else if ( timerTime == 0 ) {
            timerTime = -1;
            Block::Inscribe(tr("Timer fired. %1").
                arg(GetWorld()->TimeOfDayStr()));
            ++notify_flag;
        }
        if ( Sub()==EXPLOSIVE ) {
            return ( notify_flag>1 ) ?
                INNER_ACTION_EXPLODE : INNER_ACTION_ONLY;
        }
        switch ( current_time ) {
        default: --notify_flag; break;
        case END_OF_NIGHT:   Inscribe(tr("Morning has come.")); break;
        case END_OF_MORNING: Inscribe(tr("Day has come."));     break;
        case END_OF_NOON:    Inscribe(tr("Evening has come.")); break;
        case END_OF_EVENING: Inscribe(tr("Night has come."));   break;
        }
        if ( notify_flag > 0 ) {
            Use(nullptr);
            return INNER_ACTION_MESSAGE;
        } else {
            return INNER_ACTION_ONLY;
        }
    }

    bool Clock::Inscribe(QString str) {
        Block::Inscribe(str);
        char c;
        QTextStream txt_stream(&str);
        txt_stream >> c;
        switch ( c ) {
        case 'a': {
            int alarm_hour;
            txt_stream >> alarm_hour >> alarmTime;
            alarmTime += alarm_hour*60;
            timerTime = -1;
            } break;
        case 't':
            txt_stream >> timerTime;
            alarmTime = -1;
            break;
        default:
            alarmTime = timerTime = -1;
            break;
        }
        return true;
    }

    void Clock::SaveAttributes(QDataStream & str) const {
        str << alarmTime << timerTime;
    }

    Clock::Clock(const int kind, const int sub) :
            Active(kind, sub, NONSTANDARD)
    {}

    Clock::Clock (QDataStream & str, const int kind, const int sub) :
            Active(str, kind, sub, NONSTANDARD)
    {
        str >> alarmTime >> timerTime;
    }

// Text::
    QString Text::FullName() const {
        switch ( Sub() ) {
        case PAPER: return QObject::tr("Paper page");
        case GLASS: return QObject::tr("Screen");
        default:    return QObject::tr("Sign (%1)").arg(SubName(Sub()));
        }
    }

    usage_types Text::Use(Block * const who) {
        if ( noteId == 0 ) {
            who->ReceiveSignal(QObject::tr("Nothing is written here."));
            return USAGE_TYPE_NO;
        } else {
            return USAGE_TYPE_READ;
        }
    }

    bool Text::Inscribe(const QString str) {
        if ( '.' != str.at(0) && (noteId == 0 || GLASS == Sub()) ) {
            Block::Inscribe(str);
            return true;
        } else {
            return false;
        }
    }

// Map::
    wearable    Map::Wearable() const { return WEARABLE_OTHER; }
    QString     Map::FullName() const { return QObject::tr("Map"); }
    usage_types Map::UseOnShredMove(Block * const user) { return Use(user); }

    usage_types Map::Use(Block * const who) {
        if ( noteId == 0 ) {
            who->ReceiveSignal(QObject::tr("Set title to this map first."));
            return USAGE_TYPE_NO;
        } // else:
        if ( who == nullptr ) return USAGE_TYPE_READ;
        const Active * const active = who->ActiveBlock();
        if ( active == nullptr ) return USAGE_TYPE_READ;
        QFile map_file(home_path + active->GetWorld()->WorldName()
            + "/texts/" + GetNote());
        if ( not map_file.open(QIODevice::ReadWrite | QIODevice::Text) ) {
            return USAGE_TYPE_READ;
        }
        const long  lati = active->GetShred()->Latitude();
        const long longi = active->GetShred()->Longitude();
        const int FILE_SIZE_CHARS = 31 + 1;
        if ( 0 == map_file.size() ) { // new map
            const char header[FILE_SIZE_CHARS+1] =
                "+-----------------------------+\n";
            const char   body[FILE_SIZE_CHARS+1] =
                "|                             |\n";
            map_file.write(header, FILE_SIZE_CHARS);
            for (int i=0; i<FILE_SIZE_CHARS-3; ++i) {
                map_file.write(body, FILE_SIZE_CHARS);
            }
            map_file.write(header, FILE_SIZE_CHARS);
            longiStart = longi;
            latiStart  = lati;
        }
        const int border_dist = (FILE_SIZE_CHARS - 1) / 2;
        if (
                ( qAbs(lati  - latiStart ) > border_dist ) ||
                ( qAbs(longi - longiStart) > border_dist ) )
        {
            return USAGE_TYPE_READ;
        }
        if ( savedChar ) {
            map_file.seek(savedShift);
            map_file.putChar(savedChar);
        }
        map_file.seek( savedShift = FILE_SIZE_CHARS *
            (longi - longiStart + border_dist ) +
             lati  - latiStart  + border_dist );
        map_file.putChar('@');
        savedChar = active->GetWorld()->GetMap()->TypeOfShred(longi, lati);
        map_file.seek(FILE_SIZE_CHARS * (FILE_SIZE_CHARS - 1));
        map_file.write(" @ = ");
        map_file.putChar(savedChar);
        return USAGE_TYPE_READ;
    } // usage_types Map::Use(Block * who)

    void Map::SaveAttributes(QDataStream & out) const {
        out << longiStart << latiStart << savedShift << savedChar;
    }

    Map::Map(const int kind, const int sub) :
            Text(kind, sub),
            longiStart(),
            latiStart(),
            savedShift(),
            savedChar(0)
    {}

    Map::Map(QDataStream & str, const int kind, const int sub) :
            Text(str, kind, sub),
            longiStart(),
            latiStart(),
            savedShift(),
            savedChar()
    {
        str >> longiStart >> latiStart >> savedShift >> savedChar;
    }

// Bell:: section
    wearable Bell::Wearable() const { return WEARABLE_OTHER; }

    void Bell::Damage(int, int) {
        SendSignalAround(tr("^ Ding-ding! ^"));
        Break();
    }

    QString Bell::FullName() const {
        return tr("Bell (%1)").arg(SubName(Sub()));
    }

    usage_types Bell::Use(Block *) {
        SendSignalAround(tr("^ Ding! ^"));
        return USAGE_TYPE_NO;
    }

// Telegraph:: section
    QString Telegraph::sharedMessage;

    Telegraph::Telegraph(const int sub, const int id) :
            Active(sub, id, BLOCK_OPAQUE),
            isReceiver(true)
    {}

    Telegraph::Telegraph(QDataStream & str, const int sub, const int id) :
            Active(str, sub, id),
            isReceiver()
    {
        str >> isReceiver;
    }

    void Telegraph::SaveAttributes(QDataStream & str) const {
        str << isReceiver;
    }

    int  Telegraph::ShouldAct() const { return FREQUENT_RARE; }
    void Telegraph::ReceiveSignal(const QString str) { Inscribe(str); }
    wearable Telegraph::Wearable() const { return WEARABLE_OTHER; }

    QString Telegraph::FullName() const {
        return tr("Telegraph (%1)").arg(SubName(Sub()));
    }

    bool Telegraph::Inscribe(const QString str) {
        isReceiver = false;
        return Block::Inscribe(str);
    }

    usage_types Telegraph::Use(Block *) {
        isReceiver = not isReceiver;
        return USAGE_TYPE_NO;
    }

    inner_actions Telegraph::ActInner() {
        if ( isReceiver ) {
            const QString note = GetNote();
            if ( note != sharedMessage && not sharedMessage.isEmpty() ) {
                Inscribe(sharedMessage);
                SendSignalAround(sharedMessage);
                return INNER_ACTION_MESSAGE;
            }
        } else {
            sharedMessage = GetNote();
            isReceiver    = true;
        }
        return INNER_ACTION_ONLY;
    }

// MedKit:: section
    QString MedKit::FullName() const { return QObject::tr("MedKit"); }
    wearable MedKit::Wearable() const { return WEARABLE_OTHER; }

    usage_types MedKit::Use(Block * const user) {
        if ( user
                && GROUP_MEAT == GetSubGroup(user->Sub())
                && GetDurability() > MAX_DURABILITY/10 )
        {
            user->Mend(MAX_DURABILITY/10);
            Damage(MAX_DURABILITY/10, DAMAGE_TIME);
        }
        return USAGE_TYPE_NO;
    }

// Informer:: section
    wearable Informer::Wearable() const { return WEARABLE_OTHER; }

    usage_types Informer::Use(Block * const user) {
        switch ( Sub() ) {
        case IRON: user->ReceiveSignal(QString("Your direction: %1.").
            arg(DirString(user->GetDir()).toLower())); break;
        default: break;
        }
        return USAGE_TYPE_NO;
    }

    QString Informer::FullName() const {
        switch ( Sub() ) {
        case IRON: return QObject::tr("Compass");
        default:   return QObject::tr("Informer (%1)").arg(SubName(Sub()));
        }
    }
