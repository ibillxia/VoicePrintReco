/*
    This file (SegServerIOHyp.h) is part of SpkMoulinette.

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

/// @file SegServerIOHyp.h
/// @brief Declaration file of SegServerIOHyp class
/// @date 10/05/11
/// @author Pierre CLÉMENT

//------------------------------
#if !defined(SegServerIOHyp_h)
#define SegServerIOHyp_h
//------------------------------

//------------------------------
#include "alize.h"
#include "liatools.h"
#include "SegServerIO.h"
//------------------------------

/// @class SegServerIOHyp
/// @brief SegServerIOHyp class contains the tools to read and write SegServer from Hyp files.
/// @author Pierre CLÉMENT, Corinne Fredouille.
/// @date 10/05/11, 24/01/12
class SegServerIOHyp : public SegServerIO
{
private:
public:
	SegServerIOHyp(FileName, Config*);
	~SegServerIOHyp();
    void readSegServer(SegServer&, LabelServer&);
    void writeSegServer(SegServer&, LabelServer&);
}; // end class SegServerIOHyp

//------------------------------
#endif // !defined(SegServerIOHyp_h)
//------------------------------
