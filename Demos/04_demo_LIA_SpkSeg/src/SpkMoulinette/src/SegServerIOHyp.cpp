/*
    This file (SegServerIOHyp.cpp) is part of SpkMoulinette.

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

/// @file SegServerIOHyp.cpp
/// @brief Implementation file of SegServerIOHyp class
/// @date 24/01/2012
/// @author Pierre CLÉMENT, Corinne Fredouille

#if !defined(SegServerIOHyp_cpp)
#define SegServerIOHyp_cpp

#include "SegServerIOHyp.h"

//-------------------------------------------------------------------------
/// @brief Default constructor.
SegServerIOHyp::SegServerIOHyp(FileName fileName, Config* conf) : SegServerIO(fileName, conf)
{
}

//-------------------------------------------------------------------------
/// @brief Destructor.
SegServerIOHyp::~SegServerIOHyp()
{
}

//-------------------------------------------------------------------------
/// @brief Read a SegServer from a Hyp file.
/// @param seg_server the SegServer that will be read.
void SegServerIOHyp::readSegServer(SegServer& seg_server, LabelServer& label_server)
{
    XList* label_file = new XList(this->file);
    XLine* line;
    while((line = label_file->getLine()) != NULL)
    {

	double begin = line->getElement(1).toDouble();
	double dur = line->getElement(2).toDouble()-line->getElement(1).toDouble();
        String label = line->getElement(4);
        
        unsigned long frame_begin = timeToFrameIdx(begin, .01);
        unsigned long segment_length = timeToFrameIdx(dur, .01);
        
        long label_id = label_server.getLabelIndexByString(label);
        if(label_id == -1)
        {
            label_id = label_server.addLabel(label);
            seg_server.createCluster(label_id);
        }
        
        SegCluster& cluster = seg_server.getCluster(label_id);
        cluster.add(seg_server.createSeg(frame_begin, segment_length, label_id, label, this->file_name));
        
    }
    delete line;
    delete label_file;
}

//-------------------------------------------------------------------------
/// @brief Write a SegServer into a Hyp file.
/// @param seg_server the SegServer that will be read.
void SegServerIOHyp::writeSegServer(SegServer& seg_server, LabelServer& label_server)
{
    ofstream writer(this->file.c_str());
    if (writer.is_open())
    {
        for(unsigned long i = 0; i < seg_server.getSegCount(); ++i)
        {
            Seg& seg = seg_server.getSeg(i);
            float start = seg.begin()*0.01;
            float dur = seg.length()*.01;
            writer << this->file_name <<  " " << start << " " << start+dur-0.01 << " speaker " << label_server.getLabel(seg.labelCode()).getString() << endl;
        }
        writer.close();
    }
    else
    {
        throw Exception("Cannot write Hyp file \""+this->file+"\".", __FILE__, __LINE__);
    }
}

#endif // !defined(SegServerIOHyp_cpp)
