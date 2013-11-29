/*
    This file (SpkMoulinetteMain.cpp) is part of SpkMoulinette.

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

/// @file SpkMoulinetteMain.cpp
/// @brief Implementation file of SpkMoulinette main
/// @date 04/04/11
/// @author Pierre CLÉMENT, Corinne Fredouille
/// Modification : 24/01/2012 - add Mdtm format

#if !defined(SpkMoulinetteMain_cpp)
#define SpkMoulinetteMain_cpp

#include "SpkMoulinetteMain.h"

//-------------------------------------------------------------------------
/// @brief Display the help of SpkMoulinette.
/// @param cc the ConfigChecker which check the arguments.
void displayHelp(ConfigChecker* cc, ConfigChecker* cc_eaf)
{
    cout << endl << APP_NAME << endl;
    cout << "Version: " << VERSION_MAJOR << "." << VERSION_MINOR << endl;
    cout << endl;
    cout << "Convert segmentation files from a format to another. It only deals with speakers informations." << endl;
    cout << endl << "Options:" << endl;
    cout << cc->getParamList() << endl;
    cout << endl << "Options for EAF output format:" << endl;
    cout << cc_eaf->getParamList() << endl; 
}

//-------------------------------------------------------------------------
/// @brief Display the version of SpkMoulinette.
void displayVersion()
{
    cout << APP_NAME << " - convert files - v" << VERSION_MAJOR << "." << VERSION_MINOR << endl; 
}

//-------------------------------------------------------------------------
/// Launch SpkMoulinette.
/// @param argc number of arguemnts
/// @param argv program arguments
/// @return status code
int main (int argc, char* argv[])
{
	ConfigChecker* cc = new ConfigChecker();
	cc->addStringParam("config", false, true, "Configuration file.");
	cc->addStringParam("filesList", true, true, "List of files to convert.");
	cc->addStringParam("inputFormat", false, true, "Input files format [TRS|LBL|RTTM|MDTM|TEXTGRID|EAF|HYP|RAW] (default \"TRS\").");
	cc->addStringParam("inputFilesPath", false, true, "Input files path (default \"trs/\").");
	cc->addStringParam("inputFilesExtension", false, true, "Input files extension (default \".trs\").");
	cc->addStringParam("outputFormat", false, true, "Output files format [TRS|LBL|RTTM|MDTM|TEXTGRID|EAF|HYP|RAW] (default \"LBL\").");
	cc->addStringParam("outputFilesPath", false, true, "Output files path (default \"lbl/\").");
	cc->addStringParam("outputFilesExtension", false, true, "Output files extension (default \".lbl\").");
    
	ConfigChecker* cc_eaf = new ConfigChecker();
    
	try
	{
		CmdLine* cmdLine = new CmdLine(argc, argv);
		if (cmdLine->displayHelpRequired())
		{
			displayHelp(cc, cc_eaf);
			return 0;  
		}
		else if (cmdLine->displayVersionRequired())
		{
			displayVersion();
			return 0; 
		} 
		Config* tmp = new Config();
		cmdLine->copyIntoConfig(*tmp);
		Config* config = new Config();
		if (tmp->existsParam("config"))
        {
			config->load(tmp->getParam("config"));
        }
		cmdLine->copyIntoConfig(*config);
        
        if(!config->existsParam("inputFormat"))
            config->setParam("inputFormat", "TRS");
        if(!config->existsParam("inputFilesPath"))
            config->setParam("inputFilespath", "trs/");
        if(!config->existsParam("inputFilesExtension"))
            config->setParam("inputFilesExtension", ".trs");
        if(!config->existsParam("outputFormat"))
            config->setParam("outputFormat", "LBL");
        if(!config->existsParam("outputFilesPath"))
            config->setParam("outputFilespath", "lbl/");
        if(!config->existsParam("outputFilesExtension"))
            config->setParam("outputFilesExtension", ".lbl");
        
		
        cc->check(*config);
        
        Moulinette* moulin = new Moulinette(config);
        XList* files = new XList(config->getParam("filesList"), *config);
        XLine* file;
        while((file=files->getLine()) != NULL)
        {
            String fileName = *file->getElement();
            moulin->read(fileName);
            moulin->write(fileName);
        }
        delete file;
        delete files;
        delete moulin;
        
        delete config;
        delete cmdLine;
        delete tmp;
        
	} // end try 
	catch (alize::Exception& e)
	{
		cout << "/!\\ Error while running " << APP_NAME << ", in \"" << e.sourceFile << "\", line " << e.line << "." << endl
        << "Exiting with message: " << e.msg << "." << endl;
	}
    delete cc_eaf;
    delete cc;
	return 0;
}

#endif // !defined(SpkMoulinetteMain_cpp)
