/*
    This file (SegServerIOLbl.cpp) is part of SpkMoulinette.

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

/// @file SegServerIOLbl.cpp
/// @brief Implementation file of SegServerIOLbl class
/// @date 04/04/11
/// @author Pierre CLÉMENT

#if !defined(SegServerIOLbl_cpp)
#define SegServerIOLbl_cpp

#include "SegServerIOLbl.h"

//-------------------------------------------------------------------------
/// @brief Default constructor.
SegServerIOLbl::SegServerIOLbl(FileName fileName, Config* conf) : SegServerIO(fileName, conf)
{
}

//-------------------------------------------------------------------------
/// @brief Destructor.
SegServerIOLbl::~SegServerIOLbl()
{
}

//-------------------------------------------------------------------------
/// @brief Read a SegServer from a LBL file.
/// @param seg_server the SegServer that will be read.
void SegServerIOLbl::readSegServer(SegServer& seg_server, LabelServer& label_server)
{
    XList* label_file = new XList(this->file);
    XLine* segment;
    while((segment = label_file->getLine()) != NULL)
    {
        double begin = segment->getElement(0).toDouble();
        double end = segment->getElement(1).toDouble();
        String label = segment->getElement(2);
        
        unsigned long frame_begin = timeToFrameIdx(begin, .01);
        unsigned long segment_length = timeToFrameIdx(end, .01) - frame_begin + 1;
        
        long label_id = label_server.getLabelIndexByString(label);
        if(label_id == -1)
        {
            label_id = label_server.addLabel(label);
            seg_server.createCluster(label_id);
        }
        
        SegCluster& cluster = seg_server.getCluster(label_id);
        cluster.add(seg_server.createSeg(frame_begin, segment_length, label_id, label, this->file_name));
    }
    delete segment;
    delete label_file;
}

//-------------------------------------------------------------------------
/// @brief Write a SegServer into a LBL file.
/// @param seg_server the SegServer that will be read.
void SegServerIOLbl::writeSegServer(SegServer& seg_server, LabelServer& label_server)
{
    ofstream writer(this->file.c_str());
    if (writer.is_open())
    {
        for(unsigned long i = 0; i < seg_server.getSegCount(); ++i)
        {
            Seg& seg = seg_server.getSeg(i);
            float start = seg.begin()*0.01;
            float end = (seg.begin() + seg.length() - 1)*.01;
            writer << start << " " << end << " " << label_server.getLabel(seg.labelCode()).getString() << endl;
        }
        writer.close();
    }
    else
    {
        throw Exception("Cannot write LBL file \""+this->file+"\".", __FILE__, __LINE__);
    }
}

#endif // !defined(SegServerIOLbl_cpp)