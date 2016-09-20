//
//  RSDebugWindow.h
//  RetroSim
//
//  Created by Zoltán Majoros on 07/Nov/15.
//  Copyright © 2015 Zoltán Majoros. All rights reserved.
//

#pragma once

#include "RSWindow.h"
#include <string>
#include <vector>

#define LOG_MAXLINES 256

using std::string;
using std::vector;

//class RSCore;
class RSMonitorInterpreter;
class _TTF_Font;

struct RSDebugWindow : public RSWindow
{
    _TTF_Font *normalFont = nullptr;
    _TTF_Font *boldFont = nullptr;
    int fontSize = 16;
    int charWidth = 0, charHeight = 0;
    int textModeWidth = 0, textModeHeight = 0;
    
    vector<string> lines; // TODO: add a separate inputLine variable to store the current input
    
//    SDL_TimerID cursorTimer = 0;
    int cursorPos = 0;
    bool cursorVisible = true;
    string prompt = "hex>";
    vector<string> commandHistory;
    uint16 commandHistoryIndex = 0;
    
    virtual void create(const char *title, int width, int height, int posX, int posY, bool isResizable = true) override;
    virtual void resized(uint32 width, uint32 height) override;
    
    void render();

    uint8 charactersInLine = 0;
    
    virtual void textInput(char characters[32]) override;
    virtual void keyInput(int scancode) override;
    
    void processInput(const string &input);
    void processInputFromFramework(const string &input);
    void handleReturn(bool addNewLine = false);
    bool cursorEnabled = false;
    
    RSMonitorInterpreter *monitorInterpreter = nullptr;
    
    virtual ~RSDebugWindow();
    
private:

    //RSCore *core = nullptr;
    static uint32 cursorBlinkCallback(uint32 interval, void *param);
    const std::string copyrightMessage = "Copyright (c) 2011-2016 Zoltán Majoros (zoltan@arcanelab.com)";

    void initializeTextBuffer();
    void calcTextModeSize(); // given the window's dimension calculate how many characters fits it
    void calcFontDimensions(); // render a single letter and store resulting texture size
};
