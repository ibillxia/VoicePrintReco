/*
    This file (SegServerIO.h) is part of SpkMoulinette.

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

/// @file SegServerIO.h
/// @brief Declaration file of SegServerIO class
/// @date 05/04/11
/// @author Pierre CLÉMENT

//------------------------------
#if !defined(SegServerIO_h)
#define SegServerIO_h
//------------------------------

//------------------------------
#include "alize.h"
#include "liatools.h"
#include <string>
//------------------------------

//------------------------------
using namespace alize;
//------------------------------

/// @class SegServerIO
/// @brief SegServerIO class contains the tools to read and write SegServers from RAW files.
/// @author Pierre CLÉMENT.
/// @date 05/04/11
class SegServerIO
{
protected:
    String file; ///< The complete file name, with its path and file extension.
    String file_name; ///< The file's name.
    String file_path; ///< The file's path.
    String file_ext; ///< The file's extension.
    String normalizeName(String); 
public:
	SegServerIO(FileName, Config*);
	~SegServerIO();
    virtual void readSegServer(SegServer&, LabelServer&);
    virtual void writeSegServer(SegServer&, LabelServer&);
}; // end class SegServerIO

//------------------------------
#endif // !defined(SegServerIO_h)
//------------------------------