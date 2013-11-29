/*
    This file (SegServerIOTrs.cpp) is part of SpkMoulinette.

    LIA_DiscussionPattern is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as 
    published by the Free Software Foundation, either version 3 of 
    the License, or any later version.

    LIA_DiscussionPattern is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License (LGPL) for more details.

    You should have received a copy of the GNU Lesser General Public 
    License along with LIA_DiscussionPattern.
    If not, see <http://www.gnu.org/licenses/lgpl.html>.
*/

/// @file SegServerIOTrs.cpp
/// @brief Implementation file of SegServerIOTrs class
/// @date 29/03/11
/// @author Pierre CLÉMENT

#if !defined(SegServerIOTrs_cpp)
#define SegServerIOTrs_cpp

#include "SegServerIOTrs.h"

//-------------------------------------------------------------------------
/// @brief Default constructor.
SegServerIOTrs::SegServerIOTrs(FileName fileName, Config* conf) : SegServerIO(fileName, conf)
{
}

//-------------------------------------------------------------------------
/// @brief Destructor.
SegServerIOTrs::~SegServerIOTrs()
{
}

//-------------------------------------------------------------------------
/// @brief Read a SegServer from a TRS file.
/// @param seg_server the SegServer that will be read.
void SegServerIOTrs::readSegServer(SegServer& seg_server, LabelServer& label_server)
{
    TiXmlDocument* doc = new TiXmlDocument(this->file.c_str());
    if(doc->LoadFile())
    {
        TiXmlHandle* doc_root = new TiXmlHandle(doc);
        
        // Retrieves the speakers:
        LabelServer* label_server_tmp = new LabelServer();
        TiXmlElement* spk = doc_root->FirstChildElement().FirstChild("Speakers").FirstChild().Element();
        while(spk)
        {
            String name = spk->Attribute("name");
            name = this->normalizeName(name);
            label_server.addLabel(name);
            seg_server.createCluster();
            String id = spk->Attribute("id");
            label_server_tmp->addLabel(id);
            spk = spk->NextSiblingElement();
        }
        
        // Retrieves the segments:
        TiXmlElement* section = doc_root->FirstChildElement().FirstChild("Episode").FirstChild().Element();
        while(section)
        {
            TiXmlElement* turn = section->FirstChildElement("Turn");
            while(turn)
            {
                if(turn->Attribute("speaker"))
                {
                    String speaker = turn->Attribute("speaker");
                    speaker = speaker.getToken(0, " ");
                    unsigned long id = label_server_tmp->getLabelIndexByString(speaker);
                    double start = String(turn->Attribute("startTime")).toDouble();
                    double end = String(turn->Attribute("endTime")).toDouble();
                    
                    unsigned long segFrameBegin=timeToFrameIdx(start, .01); // begin in frame
                    unsigned long segFrameLength=timeToFrameIdx(end, .01) - segFrameBegin;
                    
                    SegCluster& cluster = seg_server.getCluster(id);
                    cluster.add(seg_server.createSeg(segFrameBegin, segFrameLength, id, label_server.getLabel(id).getString(), this->file_name));
                }
                turn = turn->NextSiblingElement("Turn");
            }
            section = section->NextSiblingElement();
        }
        delete label_server_tmp;
        delete doc_root;
    }
    else
    {
        throw Exception("Cannot open TRS file \""+this->file+"\".", __FILE__, __LINE__);
    }
    delete doc;
}

//-------------------------------------------------------------------------
/// @brief Write a SegServer into a TRS file. Cannot use TinyXml save function, rounding problem on Mac OS.
/// @param seg_server the SegServer that will be read.
void SegServerIOTrs::writeSegServer(SegServer& seg_server, LabelServer& label_server)
{
    ofstream writer(this->file.c_str());
    if (writer.is_open())
    {
        writer << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << endl
        << "<!DOCTYPE Trans SYSTEM \"trans-14.dtd\">" << endl
        << "<Trans audio_filename=\"" << this->file_name << "\" version=\"1\">" << endl;
        
        // Speakers:
        writer << "\t<Speakers>" << endl ;
        for(int i=0; i < label_server.size(); ++i)
        {
            writer << "\t\t<Speaker id=\"spk"<< i+1 << "\" name=\"" << label_server.getLabel(i).getString() << "\" />" << endl;
        }
        writer << "\t</Speakers>" << endl;
        
        // Turns:
        writer << "\t<Episode>" << endl;
        
        if(seg_server.getSegCount() > 0)
        {
            double startTime = frameIdxToTime(seg_server.getSeg(seg_server.getSegCount()-1).begin(), .01);
            double endTime = startTime + frameIdxToTime(seg_server.getSeg(seg_server.getSegCount()-1).length(), .01);
            
            writer << "\t\t<Section type=\"nontrans\" startTime=\"0.00\" endTime=\"" << endTime << "\">" << endl;
            for(int i=0; i < seg_server.getSegCount(); ++i)
            {
                startTime = frameIdxToTime(seg_server.getSeg(i).begin(), .01);
                endTime = startTime + frameIdxToTime(seg_server.getSeg(i).length(), .01);
                writer << "\t\t\t<Turn speaker=\"spk" << seg_server.getSeg(i).labelCode() + 1
                << "\" startTime=\"" << startTime << "\" endTime=\"" << endTime << "\" />" << endl;
            }
            writer << "\t\t</Section>" << endl
            << "\t</Episode>" << endl;
        }
        writer << "</Trans>";
    }
    else
    {
        throw Exception("Cannot write TRS file \""+this->file+"\".", __FILE__, __LINE__);
    }
}

#endif // !defined(SegServerIOTrs_cpp)