//
//  A65000MMU.h
//  A65000CPU
//
//  Created by Zoltán Majoros on 11/26/13.
//  Copyright (c) 2013 Zoltán Majoros. All rights reserved.
//

#pragma once

#include "A65000Exception.h"
#include "A65000CPU.h"

struct A65000MMU
{
    const uint32_t MAXMEM = 0x1000000;
    uint8_t *memPtr = nullptr;

    A65000MMU()
    {
        memPtr = new uint8_t[MAXMEM];
        if(memPtr == nullptr)
        {
            // TODO: check if allocation was successful
        }
        else
        {   // initialize memory to all zero
            uint32_t i = 0;
            while(i<MAXMEM)
                memPtr[i++] = 0;
        }
    }
    
    ~A65000MMU()
    {
        delete [] memPtr;
    }

    template <class T>
    T read(const uint32_t address)
    {
        if(address > (MAXMEM-sizeof(T)))
        {
            A65000Exception exception(EX_INVALID_ADDRESS);
            exception.address = address;
            throw exception;
        }
        
        return *(T *)&memPtr[address];
    }

    template <class T>
    void write(const uint32_t &address, const T &value)
    {
        if(address > (MAXMEM-sizeof(T)))
        {
            A65000Exception exception(EX_INVALID_ADDRESS);
            exception.address = address;
            throw exception;
        }
        
        *(T *)&memPtr[address] = value;
    }

};
