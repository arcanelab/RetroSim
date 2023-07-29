// RetroSim - Copyright 2011-2023 Zoltán Majoros. All rights reserved.
// https://github.com/arcanelab

#include <iostream>
#include <string>
#include "Application.h"

std::string GetFilenameFromArgs(int argc, char *argv[]);

int main(int argc, char *argv[])
{
    printf("RetroSim v0.2.0 development\n");

    std::string scriptFileName = GetFilenameFromArgs(argc, argv);

    RetroSim::Application::Run(scriptFileName);
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
