//
//  Time.h
//  RetroSim
//
//  Created by Zoltán Majoros on 20/Oct/15.
//  Copyright (c) 2015 Zoltán Majoros. All rights reserved.
//

#pragma once
#include <chrono>

struct RSTime
{
    static std::chrono::time_point<std::chrono::high_resolution_clock> startTime/* = std::chrono::high_resolution_clock::now()*/;
    static std::chrono::time_point<std::chrono::high_resolution_clock> endTime/* = std::chrono::high_resolution_clock::now()*/;
    
    static std::chrono::high_resolution_clock myclock;
    
    RSTime();    
    static double getElapsedTimeInMicroseconds();
};
