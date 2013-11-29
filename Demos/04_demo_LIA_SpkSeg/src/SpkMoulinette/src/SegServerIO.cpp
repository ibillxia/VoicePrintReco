/*
    This file (SegServerIO.cpp) is part of SpkMoulinette.

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

/// @file SegServerIO.cpp
/// @brief Implementation file of SegServerIO class
/// @date 05/04/11
/// @author Pierre CLÉMENT

#if !defined(SegServerIO_cpp)
#define SegServerIO_cpp

#include "SegServerIO.h"

//-------------------------------------------------------------------------
/// @brief Default constructor.
SegServerIO::SegServerIO(FileName fileName, Config* conf)
{
    this->file_name = fileName;
    this->file_path = conf->getParam("filePath");
    this->file_ext = conf->getParam("fileExtension");
    this->file = this->file_path + "/"  + fileName + this->file_ext;
}

//-------------------------------------------------------------------------
/// @brief Destructor.
SegServerIO::~SegServerIO()
{
}

//-------------------------------------------------------------------------
/// @brief Read a SegServer from a RAW file.
/// @param seg_server the SegServer that will be read.
/// @param label_server the LabelServer used with the SegServer. It will contain speakers labels (name or id).
void SegServerIO::readSegServer(SegServer& seg_server, LabelServer& label_server)
{
    Config* conf_tmp;
    conf_tmp->setParam("loadSegServerFileFormat", "RAW");
    conf_tmp->setParam("loadSegServerFileExtension", this->file_ext);
    conf_tmp->setParam("loadServerFilesPath", this->file_path);
    conf_tmp->setParam("sampleRate", "100");
    seg_server.load(this->file_name, *conf_tmp);
    delete conf_tmp;
}

//-------------------------------------------------------------------------
/// @brief Write a SegServer into a RAW file.
/// @param seg_server the SegServer to write.
/// @param label_server the LabelServer that contains the speakers labels.
void SegServerIO::writeSegServer(SegServer& seg_server, LabelServer& label_server)
{
    Config* conf_tmp;
    conf_tmp->setParam("saveSegServerFileFormat", "RAW");
    conf_tmp->setParam("saveSegServerFileExtension", this->file_ext);
    conf_tmp->setParam("segServerFilesPath", this->file_path);
    conf_tmp->setParam("sampleRate", "100");
    seg_server.save(this->file_name, *conf_tmp);
    delete conf_tmp;
}

//-------------------------------------------------------------------------
/// @brief Clean the name from whitespaces.
/// For manually anotated files, user's can type the name of the speaker
/// with spaces (for example "Pierre CLEMENT"). But some formats (such as RTTM or LBL)
/// do not support whitespaces in names.
/// This function does not remove any accent!
/// @param name the original name of the speaker.
/// @return the cleaned name of the speaker.
/// Example:
///    "First-Name Jr. Comp'Last-Name" will give "first-name_jr_comp_last-name"
///    "speaker#1" will give "speaker_1"
String SegServerIO::normalizeName(String name)
{
    // alize::String do not provide a replace function, so:
    std::string std_name(name.c_str());
    std::string std_final;
    // Tolower & removing special characters
    for (int i=0;i<strlen(std_name.c_str());i++)
    {
        char tmp = std::tolower(std_name[i]);
        switch (tmp) {	
            case ' ':
            case '#':
            case '\'':
                std_final += '_';
                break;
            case '.':
                break;
            default:
                std_final += tmp;
        }
        
    }
    return String(std_final.c_str());
}

#endif // !defined(SegServerIO_cpp)