//
//  A65000Exception.h
//  A65000CPU
//
//  Created by Zoltán Majoros on 11/26/13.
//  Copyright (c) 2013 Zoltán Majoros. All rights reserved.
//

#pragma once

enum A65000ExceptionTypes
{
    EX_INVALID_ADDRESS,
    EX_INVALID_INSTRUCTION,
    EX_DIVISION_BY_ZERO
};

class A65000Exception
{
public:
    A65000ExceptionTypes type;
    uint64 address;
    A65000Exception(A65000ExceptionTypes type):type(type){}
};
