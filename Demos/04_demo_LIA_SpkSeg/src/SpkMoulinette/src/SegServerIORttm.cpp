/*
    This file (SegServerIORttm.cpp) is part of SpkMoulinette.

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

/// @file SegServerIORttm.cpp
/// @brief Implementation file of SegServerIORttm class
/// @date 10/05/11
/// @author Pierre CLÉMENT

#if !defined(SegServerIORttm_cpp)
#define SegServerIORttm_cpp

#include "SegServerIORttm.h"

//-------------------------------------------------------------------------
/// @brief Default constructor.
SegServerIORttm::SegServerIORttm(FileName fileName, Config* conf) : SegServerIO(fileName, conf)
{
}

//-------------------------------------------------------------------------
/// @brief Destructor.
SegServerIORttm::~SegServerIORttm()
{
}

//-------------------------------------------------------------------------
/// @brief Read a SegServer from a RTTM file.
/// @param seg_server the SegServer that will be read.
void SegServerIORttm::readSegServer(SegServer& seg_server, LabelServer& label_server)
{
    XList* label_file = new XList(this->file);
    XLine* line;
    while((line = label_file->getLine()) != NULL)
    {
        String type = line->getElement(0);
        if (type == "SPKR-INFO")
        {
            String label = line->getElement(7);
            long label_id = label_server.addLabel(label);
            seg_server.createCluster(label_id);
        }
        else if (type == "SPEAKER")
        {
            String label = line->getElement(7);
            long label_id = label_server.getLabelIndexByString(label);
            if(label_id != -1)
            {
                double begin = line->getElement(3).toDouble();
                double dur = line->getElement(4).toDouble();
                unsigned long frame_begin = timeToFrameIdx(begin, .01);
                unsigned long segment_length = timeToFrameIdx(dur, .01);
                
                SegCluster& cluster = seg_server.getCluster(label_id);
                cluster.add(seg_server.createSeg(frame_begin, segment_length, label_id, label, this->file_name));
            }
            
        }
    }
    delete line;
    delete label_file;
}

//-------------------------------------------------------------------------
/// @brief Write a SegServer into a RTTM file.
/// @param seg_server the SegServer that will be read.
void SegServerIORttm::writeSegServer(SegServer& seg_server, LabelServer& label_server)
{
    ofstream writer(this->file.c_str());
    if (writer.is_open())
    {
        for(unsigned long i = 0; i < label_server.size(); ++i)
        {
            writer << "SPKR-INFO " << this->file_name << " 1 <NA> <NA> <NA> unknown " << label_server.getLabel(i).getString() << " <NA>" << endl;
        }
        for(unsigned long i = 0; i < seg_server.getSegCount(); ++i)
        {
            Seg& seg = seg_server.getSeg(i);
            float start = seg.begin()*0.01;
            float dur = seg.length()*.01;
            writer << "SPEAKER " << this->file_name << " 1 " << start << " " << dur << " <NA> <NA> " << label_server.getLabel(seg.labelCode()).getString() << " <NA>" << endl;
        }
        writer.close();
    }
    else
    {
        throw Exception("Cannot write RTTM file \""+this->file+"\".", __FILE__, __LINE__);
    }
}

#endif // !defined(SegServerIORttm_cpp)
