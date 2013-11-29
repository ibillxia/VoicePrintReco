/*
    This file (Moulinette.h) is part of SpkMoulinette.

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

/// @file Moulinette.h
/// @brief Declaration file of Moulinette class
/// @date 04/04/11
/// @author Pierre CLÉMENT

//------------------------------
#if !defined(Moulinette_h)
#define Moulinette_h
//------------------------------

//------------------------------
#include "alize.h"
#include "liatools.h"
#include "SegServerIO.h"
#include "SegServerIOTrs.h"
#include "SegServerIOLbl.h"
#include "SegServerIORttm.h"
#include "SegServerIOEaf.h"
#include "SegServerIOMdtm.h"
#include "SegServerIOHyp.h"
//------------------------------

/// @class Moulinette
/// @brief Moulinette class contains the shortcut to easily read and write SegServers from/to different formats.
/// @author Pierre CLÉMENT.
/// @date 04/04/11
class Moulinette
{
private:
    String in_format; ///< Input files format.
    String in_path; ///< Input files path.
    String in_ext; ///< Input files extension.
    String out_format; ///< Output files format.
    String out_path; ///< Output files path.
    String out_ext; ///< Output files extension.
    SegServer* seg_server; ///< SegServer to read.
    LabelServer* label_server; ///< LabelServer used with the SegServer.
public:
	Moulinette(Config*);
	~Moulinette();
    void read(String);
    void write(String);
}; // end class Moulinette

//------------------------------
#endif // !defined(Moulinette_h)
//------------------------------
