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

namespace RetroSim::Config
{
    ConfigValues config;

    void LoadConfigFile(std::string basePath);

    void Initialize(char *basePath)
    {
        // Set default values in case they can't be read from the config file.
        config.scriptPath = "";
        config.dataPath = "";
        config.fullscreen = false;
        config.fpsOverride = 0;

        std::string path(basePath);
        LoadConfigFile(path);

        if (config.fpsOverride > 0)
            config.fps = config.fpsOverride;
        else
            config.fps = 60; // TODO: query current fps

        config.isInitialized = true;
    }

    void LoadConfigFile(std::string basePath)
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
                        config.scriptPath = basePath + "/" + value;
                    else if (key == "fullscreen")
                        config.fullscreen = (value == "true");
                    else if (key == "fpsOverride")
                        config.fpsOverride = stoi(value);
                    else if (key == "dataPath")
                        config.dataPath = basePath + "/" + value;
                    else
                        cout << "Unknown key in config file: " << key << endl;
                }
            }
        }
    }
}