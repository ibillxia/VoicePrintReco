/*
    This file (SegServerIOEaf.cpp) is part of SpkMoulinette.

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

/// @file SegServerIOEaf.cpp
/// @brief Implementation file of SegServerIOEaf class
/// @date 05/04/11
/// @author Pierre CLÉMENT

#if !defined(SegServerIOEaf_cpp)
#define SegServerIOEaf_cpp

#include "SegServerIOEaf.h"

//-------------------------------------------------------------------------
/// @brief Default constructor.
SegServerIOEaf::SegServerIOEaf(FileName fileName, Config* conf) : SegServerIO(fileName, conf)
{
}

//-------------------------------------------------------------------------
/// @brief Destructor.
SegServerIOEaf::~SegServerIOEaf()
{
}

//-------------------------------------------------------------------------
/// @brief Read a SegServer from a EAF file.
/// @param seg_server the SegServer that will be read.
void SegServerIOEaf::readSegServer(SegServer& seg_server, LabelServer& label_server)
{

}

//-------------------------------------------------------------------------
/// @brief Write a SegServer into a EAF file.
/// @param seg_server the SegServer that will be read.
void SegServerIOEaf::writeSegServer(SegServer& seg_server, LabelServer& label_server)
{
    ofstream writer(this->file.c_str());
    if (writer.is_open())
    {
        writer.close();
    }
    else
    {
        throw Exception("Cannot write EAF file \""+this->file+"\".", __FILE__, __LINE__);
    }
}

#endif // !defined(SegServerIOEaf_cpp)