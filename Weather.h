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

#ifndef WEATHER_H
#define WEATHER_H

#include <QString>

enum weathers {
    WEATHER_CLEAR,
    WEATHER_RAIN,
    WEATHER_DEW,
    WEATHER_CLOUDS
};

class Weather {
public:
    void SetWeather(weathers);
    weathers GetWeather() const;

    static QString GetWeatherString(weathers);

private:
    weathers weather;
};

#endif // WEATHER_H
