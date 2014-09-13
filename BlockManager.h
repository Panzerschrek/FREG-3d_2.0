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

#ifndef BLOCKMANAGER_H
#define BLOCKMANAGER_H

#include "header.h"

class Block;
class QDataStream;

/** \class BlockManager BlockManager.h
     * \brief This class is used for creating and deleting blocks,
     * also for loading them from file.
     *
     * Memory management, if any, should be implemented in this class.
     * At the current moment no special memory management is used.
     *
     * Normal blocks: blocks that are not special, e.g. usual stone, air, soil
     * are actually one block (for each substance).
     * One can receive a pointer to such block with
     * Block * Normal(int sub).
     * Normal blocks are not needed to be deleted.
     * Use Block * NewBlock(int kind, int sub) to receive a pointer to
     * block that will be changed (damaged, inscribed, etc). */

class BlockManager final {
public:
     BlockManager();
    ~BlockManager();

    /// Use this to receive a pointer to normal block.
    Block * Normal(int sub) const;
    /// Use this to receive a pointer to new not-normal block.
    static Block * NewBlock(int kind, int sub);
    /// Use this to load block from file.
    static Block * BlockFromFile(QDataStream &, int kind, int sub);
    /// Returns true if block is normal.
    static bool KindSubFromFile(QDataStream &, quint8 * kind, quint8 * sub);
    /// Does not actually delete normal blocks.
    void DeleteBlock(Block *) const;
    /// For memory economy.
    /** Checks and replaces block with corresponding normal block.
     *  Can delete block, use carefully. */
    Block * ReplaceWithNormal(Block * block) const;

    /// If kind is unknown, returns "unknown_kind".
    static QString KindToString(int kind);
    /// If substance is unknown, returns "unknown_sub".
    static QString SubToString(int sub);
    /// If string is not convertible to kind, returns LAST_KIND.
    static int StringToKind(QString);
    /// If string is not convertible to substance, returns LAST_SUB.
    static int StringToSub(QString);

    constexpr static int MakeId(const int kind, const int sub) {
        return (kind << 8) | sub;
    }

    static int KindFromId(int id);
    static int SubFromId(int id);

private:
    BlockManager(const BlockManager &) = delete;
    BlockManager & operator=(const BlockManager &) = delete;

    Block * normals[LAST_SUB];
    static const QByteArray kinds[];
    static const QByteArray subs[];
};

extern BlockManager block_manager;

#endif // BLOCKMANAGER_H
