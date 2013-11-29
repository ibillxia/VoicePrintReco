/*
    This file (SpkMoulinetteMain.h.in) is part of SpkMoulinette.

    SpkMoulinette is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as 
    published by the Free Software Foundation, either version 3 of 
    the License, or any later version.

    SpkMoulinette is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License (LGPL) for more details.

    You should have received a copy of the GNU Lesser General Public 
    License along with SpkMoulinette.
    If not, see <http://www.gnu.org/licenses/lgpl.html>.
*/

/// @file SpkMoulinetteMain.h.in
/// @brief Declaration file of SpkMoulinette main
/// @date 04/04/11
/// @author Pierre CLÉMENT

//------------------------------
#if !defined(SpkMoulinetteMain_h)
#define SpkMoulinetteMain_h
//------------------------------

//------------------------------
#define VERSION_MAJOR 0
#define VERSION_MINOR 7
#define APP_NAME "SpkMoulinette"
//------------------------------

//------------------------------
#include "alize.h"
#include "liatools.h"
#include "Moulinette.h"
//------------------------------

//------------------------------
using namespace std;
using namespace alize;
//------------------------------

void displayHelp(ConfigChecker*, ConfigChecker*);
void displayVersion();
int main(int, char**);

//------------------------------
#endif // !defined(SpkMoulinetteMain_h)
//------------------------------
