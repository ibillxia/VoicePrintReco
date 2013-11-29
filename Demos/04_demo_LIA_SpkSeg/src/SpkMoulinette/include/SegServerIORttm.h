/*
    This file (SegServerIORttm.h) is part of SpkMoulinette.

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

/// @file SegServerIORttm.h
/// @brief Declaration file of SegServerIORttm class
/// @date 10/05/11
/// @author Pierre CLÉMENT

//------------------------------
#if !defined(SegServerIORttm_h)
#define SegServerIORttm_h
//------------------------------

//------------------------------
#include "alize.h"
#include "liatools.h"
#include "SegServerIO.h"
//------------------------------

/// @class SegServerIORttm
/// @brief SegServerIORttm class contains the tools to read and write SegServer from RTTM files.
/// @author Pierre CLÉMENT.
/// @date 10/05/11
class SegServerIORttm : public SegServerIO
{
private:
public:
	SegServerIORttm(FileName, Config*);
	~SegServerIORttm();
    void readSegServer(SegServer&, LabelServer&);
    void writeSegServer(SegServer&, LabelServer&);
}; // end class SegServerIORttm

//------------------------------
#endif // !defined(SegServerIORttm_h)
//------------------------------