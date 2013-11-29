/*
    This file (SegServerIOMdtm.h) is part of SpkMoulinette.

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

/// @file SegServerIOMdtm.h
/// @brief Declaration file of SegServerIOMdtm class
/// @date 10/05/11
/// @author Pierre CLÉMENT

//------------------------------
#if !defined(SegServerIOMdtm_h)
#define SegServerIOMdtm_h
//------------------------------

//------------------------------
#include "alize.h"
#include "liatools.h"
#include "SegServerIO.h"
//------------------------------

/// @class SegServerIOMdtm
/// @brief SegServerIOMdtm class contains the tools to read and write SegServer from Mdtm files.
/// @author Pierre CLÉMENT, Corinne Fredouille.
/// @date 10/05/11, 24/01/12
class SegServerIOMdtm : public SegServerIO
{
private:
public:
	SegServerIOMdtm(FileName, Config*);
	~SegServerIOMdtm();
    void readSegServer(SegServer&, LabelServer&);
    void writeSegServer(SegServer&, LabelServer&);
}; // end class SegServerIOMdtm

//------------------------------
#endif // !defined(SegServerIOMdtm_h)
//------------------------------
