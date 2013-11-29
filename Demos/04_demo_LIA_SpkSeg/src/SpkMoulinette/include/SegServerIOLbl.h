/*
    This file (SegServerIOLbl.h) is part of SpkMoulinette.

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

/// @file SegServerIOLbl.h
/// @brief Declaration file of SegServerIOLbl class
/// @date 04/04/11
/// @author Pierre CLÉMENT

//------------------------------
#if !defined(SegServerIOLbl_h)
#define SegServerIOLbl_h
//------------------------------

//------------------------------
#include "alize.h"
#include "liatools.h"
#include "SegServerIO.h"
//------------------------------

/// @class SegServerIOLbl
/// @brief SegServerIOLbl class contains the tools to read and write SegServer from LBL files.
/// @author Pierre CLÉMENT.
/// @date 04/04/11
class SegServerIOLbl : public SegServerIO
{
private:
public:
	SegServerIOLbl(FileName, Config*);
	~SegServerIOLbl();
    void readSegServer(SegServer&, LabelServer&);
    void writeSegServer(SegServer&, LabelServer&);
}; // end class SegServerIOLbl

//------------------------------
#endif // !defined(SegServerIOLbl_h)
//------------------------------