//
//  ICPUInterface.h
//  A65000CPU
//
//  Created by Zoltán Majoros on 11/26/13.
//  Copyright (c) 2013 Zoltán Majoros. All rights reserved.
//

#pragma once

class ICPUInterface
{
public:
    virtual int Tick() = 0;
    virtual void Reset() = 0;
    virtual void InterruptRaised(bool isNMI) = 0;
    virtual ~ICPUInterface() {}; // virtual destructor, see http://stackoverflow.com/a/318137/1546790
    virtual void SetPC(unsigned int newPC) = 0;
    bool sleep = false;
};
