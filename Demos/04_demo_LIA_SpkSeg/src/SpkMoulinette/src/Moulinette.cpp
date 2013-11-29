/*
    This file (Moulinette.cpp) is part of SpkMoulinette.

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

/// @file Moulinette.cpp
/// @brief Implementation file of Moulinette class
/// @date 04/04/11
/// @author Pierre CLÉMENT

#if !defined(Moulinette_cpp)
#define Moulinette_cpp

#include "Moulinette.h"

//-------------------------------------------------------------------------
/// @brief Default constructor.
/// @param conf The config file used.
Moulinette::Moulinette(Config* conf)
{
    this->in_format = conf->getParam("inputFormat");
    this->in_path = conf->getParam("inputFilesPath");
    this->in_ext = conf->getParam("inputFilesExtension");
    this->out_format = conf->getParam("outputFormat");
    this->out_path = conf->getParam("outputFilesPath");
    this->out_ext = conf->getParam("outputFilesExtension");
    
    this->seg_server = new SegServer();
    this->label_server = new LabelServer();
}

//-------------------------------------------------------------------------
/// @brief Destructor.
Moulinette::~Moulinette()
{
}

//-------------------------------------------------------------------------
/// @brief Write a segmentation file.
/// @param fileName the name of the file to read.
void Moulinette::read(String fileName)
{
    this->label_server->clear();
    this->seg_server->removeAllClusters();
    this->seg_server->removeAllSegs();
    
    Config* conf_tmp = new Config();
    conf_tmp->setParam("filePath", this->in_path);
    conf_tmp->setParam("fileExtension", this->in_ext);
    SegServerIO* reader = NULL;
    
    if(this->in_format == "RAW")
    {
        reader = new SegServerIO(fileName, conf_tmp);
    }
    else if(this->in_format == "TRS")
    {
        reader = new SegServerIOTrs(fileName, conf_tmp);
    }
    else if (this->in_format == "LBL")
    {
        reader = new SegServerIOLbl(fileName, conf_tmp);
    }
    else if (this->in_format == "RTTM")
    {
        reader = new SegServerIORttm(fileName, conf_tmp);
    }
    else if (this->in_format == "EAF")
    {
        reader = new SegServerIOEaf(fileName, conf_tmp);
    }
    else if (this->in_format == "MDTM")
    {
        reader = new SegServerIOMdtm(fileName, conf_tmp);
    }
    else if (this->in_format == "HYP")
    {
        reader = new SegServerIOHyp(fileName, conf_tmp);
    }
    delete conf_tmp;
    
    if(reader != NULL)
    {
        reader->readSegServer(*this->seg_server, *this->label_server);        
        delete reader;
    }
    else
    {
        throw Exception("Unknown input file format \"" + this->in_format + "\".", __FILE__, __LINE__);
    }
}

//-------------------------------------------------------------------------
/// @brief Write a configuration file.
/// @param fileName the name of the file to write.
void Moulinette::write(String fileName)
{
    
    Config* conf_tmp = new Config();
    conf_tmp->setParam("filePath", this->out_path);
    conf_tmp->setParam("fileExtension", this->out_ext);
    
    SegServerIO* writer = NULL;
    if(this->out_format == "RAW")
    {
        writer = new SegServerIO(fileName, conf_tmp);
    }
    else if(this->out_format == "TRS")
    {
        writer = new SegServerIOTrs(fileName, conf_tmp);
    }
    else if (this->out_format == "LBL")
    {
        writer = new SegServerIOLbl(fileName, conf_tmp);
    }
    else if (this->out_format == "RTTM")
    {
        writer = new SegServerIORttm(fileName, conf_tmp);
    }
    else if (this->out_format == "EAF")
    {
        writer = new SegServerIOEaf(fileName, conf_tmp);
    }
    else if (this->out_format == "MDTM")
    {
        writer = new SegServerIOMdtm(fileName, conf_tmp);
    }
    else if (this->out_format == "HYP")
    {
        writer = new SegServerIOHyp(fileName, conf_tmp);
    }
    delete conf_tmp;
    
    if(writer != NULL)
    {
        writer->writeSegServer(*this->seg_server, *this->label_server);       
        delete writer;
    }
    else
    {
        throw Exception("Unknown output file format \"" + this->out_format + "\".", __FILE__, __LINE__);
    }    
}

#endif // !defined(MoulinetteSegReader_cpp)
