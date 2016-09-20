//
//  RSDebugWindow.cpp
//  RetroSim
//
//  Created by Zoltán Majoros on 20/May/2015.
//  Copyright © 2016 Zoltán Majoros. All rights reserved.
//

#include "RSDebugWindow.h"
#include "RSFonts.h"
#include "RSMonitorInterpreter.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_TTF.h>

void RSDebugWindow::create(const char *title, int width, int height, int posX, int posY, bool isResizable)
{
    RSWindow::create(title, width, height, posX, posY, isResizable);
    
    normalFont = RSFont::getMonoFont(highDPIScale * fontSize, false);
    boldFont = RSFont::getMonoFont(highDPIScale * fontSize, true);
    calcFontDimensions();
    calcTextModeSize();
    initializeTextBuffer();
    
    resized(width, height);
    
    SDL_AddTimer(600, cursorBlinkCallback, this);
    
    monitorInterpreter = nullptr;
    monitorInterpreter = new RSMonitorInterpreter();
    monitorInterpreter->windowBuffer = &lines;
    cursorPos = (int)prompt.size()-1;
}

RSDebugWindow::~RSDebugWindow()
{
    if(monitorInterpreter)
        delete monitorInterpreter;
}

void RSDebugWindow::resized(uint32 width, uint32 height)
{
    RSWindow::resized(width, height);
    calcTextModeSize();
    
    string windowTitle = "Debugger (";
    windowTitle += std::to_string(textModeWidth);
    windowTitle += ", ";
    windowTitle += std::to_string(textModeHeight);
    windowTitle += ")";
    
    SDL_SetWindowTitle(window, windowTitle.c_str());
}

void RSDebugWindow::render()
{
    SDL_Color textColor = {192, 192, 192, 255};
    int actConsoleLine = textModeHeight - 1; // render lines from bottom-up
    int lineIndex = (int)lines.size() - 1;
    string &inputLine = lines.back();
    prompt = monitorInterpreter->numericModeStrings[monitorInterpreter->currentNumericMode] + ">";
    
    while(actConsoleLine > 0) // render text line-by-line
    {
        if(lineIndex < 0) // stop at top line
            break;
        
        SDL_Surface *textSurface = nullptr;
        
        if(actConsoleLine == textModeHeight - 1) // bottom line? => console input line
        {
            string commandLine = prompt + inputLine; // compose command input line
            if(commandLine.size() == 1) commandLine += " "; // make room for the blinking cursor character
            if(cursorVisible)
                commandLine.replace(cursorPos + 1, 1, "_");
            
            textSurface = TTF_RenderUTF8_Blended(boldFont, commandLine.c_str(), textColor);
        }
        else // normal line
        {
            string lineToPrint(lines[lineIndex]);
            if(lineToPrint.size() == 0) // TTF textrenderer crashes on empty strings, this is a workaround
                lineToPrint += " ";
            textSurface = TTF_RenderUTF8_Blended(normalFont, lineToPrint.c_str(), textColor);
        }
        
        SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface); // Texture created from rendered line
        
        if(textSurface == NULL) { printf("Error rendering text\n"); throw; }
        
        int textureWidth = 0, textureHeight = 0;
        SDL_QueryTexture(textTexture, NULL, NULL, &textureWidth, &textureHeight);
        SDL_Rect rect = {3, charHeight * actConsoleLine, textureWidth, textureHeight}; // calculate line position on canvas
        
        if(SDL_RenderCopy(renderer, textTexture, NULL, &rect)) // render the line on screen
        {
            puts("render error.");
            throw;
        }
        
        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);
        
        lineIndex--;
        actConsoleLine--;
    }
}

void RSDebugWindow::textInput(char characters[32])
{
    cursorVisible = true;
    string &lastLine = lines.back();
    if(charactersInLine >= textModeWidth - prompt.length() - 1) // trim line length to window width
        return;
    
#if 1
    if((uint8)characters[0] < 128)
    {
        lastLine += characters[0];
        cursorPos++;
    }
    else
    {
        if(((uint8)characters[0] & 0xe0) == 0xc0) // 2 byte
        {
            lastLine += characters[0];
            lastLine += characters[1];
            cursorPos += 2;
        }
        else if (((uint8)characters[0] & 0xf0) == 0xe0) // 3 byte
        {
            lastLine += characters[0];
            lastLine += characters[1];
            lastLine += characters[2];
            cursorPos += 3;
        }
    }
    charactersInLine++;
    
#endif
}

void RSDebugWindow::keyInput(int scancode)
{
    cursorVisible = true;
    switch((SDL_Scancode)scancode)
    {
        case SDL_SCANCODE_BACKSPACE:
        {
            string &lastLine = lines.back();
            const uint8 lastLineSize = lastLine.size();
            if(lastLineSize > 0) // non-empty?
            {
                if(lastLineSize > 1)
                {
                    if(((lastLine[lastLineSize-1] & 0xc0) == 0x80) && ((lastLine[lastLineSize-2] & 0xe0) == 0xc0)) // 2-byte utf8?
                    {
                        lastLine.replace(lastLine.size()-2, 2, ""); // delete last character
                        cursorPos -= 2;
                        charactersInLine--;
                        break;
                    }
                    if(lastLineSize > 2)
                    {
                        if(((lastLine[lastLineSize-1] & 0xc0) == 0x80) && ((lastLine[lastLineSize-2] & 0xc0) == 0x80) && ((lastLine[lastLineSize-3] & 0xf0) == 0xe0)) // 3-byte utf8?
                        {
                            lastLine.replace(lastLine.size()-3, 3, ""); // delete last character
                            cursorPos -= 3;
                            charactersInLine--;
                            break;
                        }
                    }
                    if(lastLineSize > 3)
                    {
                        if(((lastLine[lastLineSize-1] & 0xc0) == 0x80) && ((lastLine[lastLineSize-2] & 0xc0) == 0x80) && ((lastLine[lastLineSize-3] & 0xc0) == 0x80) && ((lastLine[lastLineSize-4] & 0xf0) == 0xe0)) // 4-byte utf8?
                        {
                            lastLine.replace(lastLine.size()-4, 4, ""); // delete last character
                            cursorPos -= 4;
                            charactersInLine--;
                            break;
                        }
                    }
                }
                lastLine.replace(lastLine.size()-1, 1, ""); // delete last character
                cursorPos--;
                charactersInLine--;
            }
        }
            break;
        case SDL_SCANCODE_RETURN:
        {
            handleReturn();
        }
            break;
        case SDL_SCANCODE_UP:
        {
            if(commandHistoryIndex > 0)
            {
                commandHistoryIndex--;
                lines.back() = commandHistory[commandHistoryIndex];
                cursorPos = (int)(lines.back().size() + prompt.size() - 1);
                charactersInLine = lines.back().size();
            }
        }
            break;
        case SDL_SCANCODE_DOWN:
        {
            if(commandHistoryIndex < ((int)commandHistory.size() - 1))
            {
                commandHistoryIndex++;
                lines.back() = commandHistory[commandHistoryIndex];
                cursorPos = (int)(lines.back().size() + prompt.size() - 1);
                charactersInLine = lines.back().size();
            }
        }
            break;
#if 0
        case SDL_SCANCODE_RETURN2:
        case SDL_SCANCODE_LEFT:
        case SDL_SCANCODE_RIGHT:
        case SDL_SCANCODE_DOWN:
        case SDL_SCANCODE_TAB:
        case SDL_SCANCODE_ESCAPE:
        case SDL_SCANCODE_PAGEUP:
        case SDL_SCANCODE_PAGEDOWN:
        case SDL_SCANCODE_HOME:
        case SDL_SCANCODE_INSERT:
        case SDL_SCANCODE_F1:
        case SDL_SCANCODE_F2:
        case SDL_SCANCODE_F3:
        case SDL_SCANCODE_F4:
        case SDL_SCANCODE_F5:
        case SDL_SCANCODE_F6:
        case SDL_SCANCODE_F7:
        case SDL_SCANCODE_F8:
        case SDL_SCANCODE_F9:
        case SDL_SCANCODE_F10:
        case SDL_SCANCODE_F11:
        case SDL_SCANCODE_F12:
        case SDL_SCANCODE_LALT:
        case SDL_SCANCODE_RALT:
        case SDL_SCANCODE_LCTRL:
        case SDL_SCANCODE_RCTRL:
        case SDL_SCANCODE_LSHIFT:
        case SDL_SCANCODE_RSHIFT:
            break;
#endif
        default:
            break;
    }
}

void RSDebugWindow::handleReturn(bool addNewLine)
{
    charactersInLine = 0;
    const string inputLine = lines.back();
    if(inputLine.compare("") == 0)
        return;
    
    lines.back().insert(0, prompt);
    
    processInput(inputLine);
    
    lines.push_back(""); // add new line
    commandHistoryIndex = commandHistory.size();
    cursorPos = (int)prompt.size()-1;
    
    if(lines.size() > LOG_MAXLINES) // trim 'lines' to LOG_MAXLINES, if it exceeded that value
        lines.erase(lines.begin());
}

void RSDebugWindow::processInput(const string &input)
{
    if(input.length() == 0)
        return;
    
    commandHistory.push_back(input);
    
    vector<string> output = monitorInterpreter->processLine(input);
    lines.insert(lines.end(), output.begin(), output.end()); // append command output to window output
}

void RSDebugWindow::processInputFromFramework(const string &input)
{
    lines.push_back(input);
    handleReturn();
}

uint32 RSDebugWindow::cursorBlinkCallback(uint32 interval, void *param)
{
    RSDebugWindow *self = (RSDebugWindow *)param;
    
    if(self->cursorEnabled)
        self->cursorVisible = !(self->cursorVisible);
    
    return interval;
}

void RSDebugWindow::initializeTextBuffer()
{
    lines.push_back("[ Retrosim debugger / machine code monitor ]");
    lines.push_back(copyrightMessage);
    lines.push_back("Enter 'help' for available commands");
    lines.push_back(""); // this will be the empty input line
}

void RSDebugWindow::calcTextModeSize() // given the window's dimension calculate how many characters fits it
{
    textModeWidth = pixelWidth / charWidth;
    textModeHeight = pixelHeight / charHeight;
    //printf("textModeWidth = %d, textModeHeight = %d\n", textModeWidth, textModeHeight);
}

void RSDebugWindow::calcFontDimensions() // render a single letter and store resulting texture size
{
    SDL_Color textColor = {255, 255, 255, 128};
    SDL_Surface *textSurface = TTF_RenderText_Blended(normalFont, " ", textColor);
    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if(textSurface == NULL) { printf("Error rendering text\n"); }
    SDL_QueryTexture(textTexture, NULL, NULL, &charWidth, &charHeight);
    printf("charWidth = %d, charHeight = %d\n", charWidth, charHeight);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
    //        charHeight += 3 * highDPIScale; // Only for PET fonts
}
