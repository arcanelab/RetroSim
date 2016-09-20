//
//  RSFile.h
//  RetroSim
//
//  Created by Zoltán Majoros on 17/May/2015.
//  Copyright © 2016 Zoltán Majoros. All rights reserved.
//

#pragma once

//#include <istream>
#include <fstream>
#include <string>
#include <vector>

using std::string;
using std::vector;

/*
struct RSBinaryFileIn
{
    size_t size = 0;
    uint8_t *content = nullptr;
    string errorMessage;
    bool success = true;

    std::ifstream file;
    string name;

    RSBinaryFileIn(const string& name)
    {
        this->name = name;

        open();
        getFileSize();
        readBinary();
        file.close();
    }

    void open()
    {
        file.open(name);
        if(file.is_open() == false)
        {
            errorMessage = string("Could not open file: '") + name + "'";
            success = false;
            return;
        }
    }
    
    void getFileSize()
    {
        file.seekg(0, std::ios::beg);
        size = file.tellg();
        file.seekg(0, std::ios::beg);
    }
    
    void readBinary()
    {
        content = new uint8_t[size];
        
        if(!file.read((char *)content, size))
        {
            errorMessage = string("Error reading file: '") + name + "'";
            success = false;
            if(content != nullptr)
                delete [] content;
            content = nullptr;
        }
    }
    
    ~RSBinaryFileIn()
    {
        if(content != nullptr)
            delete [] content;
    }
};

 */

/*
 Example use
 Read:
 
 RSFileIn file("startup.init");
 if(file.success)
 {
     file.content is reserved, available for access, will be freed when 'file' goes out of scope
     file.size is calculated
 }
 else
 {
    print(file.errorMessage);
 }
 
 
*/

struct RSTextFileIn
{
    string errorMessage;
    bool success = true;
    vector<string> lines;
    bool verbose = true;
    
    RSTextFileIn(const string& filename)
    {
        if(verbose)
        {
            string msg = string("Loading file '") + filename + "'";
            puts(msg.c_str());
        }
        
        std::ifstream file(filename);
        if(!file.is_open())
        {
            errorMessage = string("Could not open file: '") + filename + "'";
            success = false;
            return;
        }
        
        string line;
        
        while(getline(file, line))
        {
            lines.push_back(line);
        }

        file.close();
    }
};

/*
 Example use:
 
 RSTextFileIn file("names.txt");
 if(file.success)
    file.lines is populated with the file contents, line by line
 
*/