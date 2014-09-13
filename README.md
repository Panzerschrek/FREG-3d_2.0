About                                                               {#mainpage}
=====

FREG is a free-roaming elementary game.
Inspired by Minecraft and Angband.
This is a version of FREG with text graphics.

Development stage: implementing basics.

Language: C++11, Qt (version >= 5.3)

Non-standard libraries: libncurses5

Development is run on GNU/Linux, but on MS Windows should work too with
PDCurses ( http://pdcurses.sourceforge.net/ ).

Now only singleplayer is developed, multiplayer is planned.

Compiling
---------

    qmake freg-nox.pro;
    make;

Or open freg-nox with QtCreator.

Map editing
-----------

You can edit map.txt files in your text editor.
`map.txt` should be square, with CR line endings only.
There is Vim highlight file `fregMap.vim`. You can copy it in `~/.vim/syntax`
and set file type with command:
`:set filetype=fregMap`

Also on large maps try option
`:set nowrap`

Extra fun: block copy (Ctrl-v, then select, then p) and block paste (1vp).

--------

Copyright (C) 2014 Alexander Kromm (mmaulwurff@gmail.com).

[Source code is available at GitHub](https://github.com/mmaulwurff/freg)

[Google+ Community](https://plus.google.com/communities/101873782136369825650)

[VK community](https://vk.com/freg_dev)

FREG is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

FREG is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with FREG. If not, see <http://www.gnu.org/licenses/>.
