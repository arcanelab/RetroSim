// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <regex>
#include "CoreConfig.h"
#include "FileUtils.h"

using namespace std;

namespace RetroSim
{
    void CoreConfig::Initialize(const string &basePath)
    {
        this->basePath = basePath;
        LoadConfigFile();

        if (fpsOverride > 0)
            fps = fpsOverride;
        else
            fps = 60; // TODO: query current fps

        isInitialized = true;
    }

    void CoreConfig::OverrideScriptPath(const string &scriptPath)
    {
        this->scriptPath = scriptPath;
    }

    string CoreConfig::GetDataPath()
    {
        return dataPath;
    }

    string CoreConfig::GetScriptPath()
    {
        return scriptPath;
    }

    bool CoreConfig::IsFullScreen()
    {
        return fullscreen;
    }

    void CoreConfig::LoadConfigFile()
    {
        const std::string fileName = basePath + "/retrosim.config";

        // check if file exists
        if (!RetroSim::FileExists(fileName))
        {
            printf("Config file '%s' not found.\n", fileName.c_str());
            return;
        }

        ifstream fs(fileName);
        stringstream buffer;
        buffer << fs.rdbuf();

        string actLine;
        while (getline(buffer, actLine))
        {
            if (actLine[0] == '#')
                continue;

            // create regex for key-value extraction
            std::regex keyValRegex(R"(([a-zA-Z0-9_]+)\s*:\s*(.+)$)");
            std::smatch match;
            if (std::regex_search(actLine, match, keyValRegex))
            {
                if (match.size() == 3)
                {
                    string key = match[1];
                    string value = match[2];

                    // print key-value pairs
                    // cout << key << ": " << value << endl;

                    if (key == "scriptPath")
                        scriptPath = basePath + "/" + value;
                    else if (key == "fullscreen")
                        fullscreen = (value == "true");
                    else if (key == "fpsOverride")
                        fpsOverride = stoi(value);
                    else if (key == "dataPath")
                        dataPath = basePath + "/" + value;
                    else
                        cout << "Unknown key in config file: " << key << endl;
                }
            }
        }
    }
}