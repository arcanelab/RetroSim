//
//  RSTime.cpp
//  RetroSim
//
//  Created by Zoltán Majoros on 20/May/2015.
//  Copyright © 2016 Zoltán Majoros. All rights reserved.
//

#include "RSTime.h"

std::chrono::time_point<std::chrono::high_resolution_clock> RSTime::startTime/* = std::chrono::high_resolution_clock::now()*/;
std::chrono::time_point<std::chrono::high_resolution_clock> RSTime::endTime/* = std::chrono::high_resolution_clock::now()*/;
std::chrono::high_resolution_clock myclock;

RSTime::RSTime()
{
    RSTime::startTime = std::chrono::high_resolution_clock::now();
    RSTime::endTime = std::chrono::high_resolution_clock::now();
}

double RSTime::getElapsedTimeInMicroseconds()
{
    endTime = myclock.now();
    return std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
}

