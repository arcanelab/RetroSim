// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <regex>
#include "Config.h"
#include "FileUtils.h"

using namespace std;

void Config::LoadConfigFile()
{
    const std::string fileName = "retrosim.config";
    
    // check if file exists
    if(!RetroSim::FileExists(fileName))
    {
        cout << "Config file not found: " << fileName << endl;
        return;
    }

    ifstream fs(fileName);
    stringstream buffer;
    buffer << fs.rdbuf();
    
    string actLine;
    while(getline(buffer, actLine))
    {
        if(actLine[0] == '#')
            continue;

        // create regex for key-value extraction
        std::regex keyValRegex(R"(([a-zA-Z0-9_]+)\s*:\s*(.+)$)");
        std::smatch match;
        if(std::regex_search(actLine, match, keyValRegex))
        {
            if(match.size() == 3)
            {
                string key = match[1];
                string value = match[2];

                // print key-value pairs
                cout << key << ": " << value << endl;

                if(key == "scriptPath")
                    scriptPath = value;
                else if(key == "fullscreen")
                    fullscreen = (value == "true");
                else if(key == "fpsOverride")
                    fpsOverride = stoi(value);
                else
                    cout << "Unknown key in config file: " << key << endl;
            }
        }
    }

}