/*
    This file (SegServerIOTrs.h) is part of SpkMoulinette.
 
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

/// @file SegServerFileReaderTrs.h
/// @brief Declaration file of SegServerFileReaderTrs class
/// @date 29/03/11
/// @author Pierre CLÉMENT

//------------------------------
#if !defined(SegServerIOTrs_h)
#define SegServerIOTrs_h
//------------------------------

//------------------------------
#include "alize.h"
#include "liatools.h"
#include <vector>
#include "tinyxml.h"
#include "SegServerIO.h"
//------------------------------

/// @class SegServerIOTrs
/// @brief SegServerIOTrs class contains the tools to read and write SegServer from TRS files.
/// @author Pierre CLÉMENT.
/// @date 29/03/11
class SegServerIOTrs : public SegServerIO
{
private:
public:
	SegServerIOTrs(FileName, Config*);
	~SegServerIOTrs();
    void readSegServer(SegServer&, LabelServer&);
    void writeSegServer(SegServer&, LabelServer&);
}; // end class SegServerIOTrs

//------------------------------
#endif // !defined(SegServerIOTrs_h)
//------------------------------