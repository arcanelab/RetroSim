// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#include <iostream>
#include <string>
#include "SDLGPUApp.h"

std::string GetFilenameFromArgs(int argc, char *argv[]);

int main(int argc, char *argv[])
{
    printf("RetroSim v0.2.0 (SDL-gpu)\n\n");

    std::string scriptFileName = GetFilenameFromArgs(argc, argv);

    RetroSim::SDLGPUApp::Run(scriptFileName);
}

std::string GetFilenameFromArgs(int argc, char *argv[])
{
    if (argc == 1) // no arguments
        return "";

    if (argc != 2) // one argument
    {
        puts("Usage: RetroSim <script>");
        exit(0);
    }

    return std::string(argv[1]);
}
