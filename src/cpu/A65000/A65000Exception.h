//
//  A65000Exception.h
//  A65000CPU
//
//  Created by Zoltán Majoros on 11/26/13.
//  Copyright (c) 2013 Zoltán Majoros. All rights reserved.
//

#pragma once
#include <cstdint>

struct A65000Exception
{
    enum class Type
    {
        NO_EXCEPTION,
        EX_INVALID_ADDRESS,
        EX_INVALID_INSTRUCTION,
        EX_DIVISION_BY_ZERO
    };

    Type type;
    A65000Exception(): type(Type::NO_EXCEPTION) {}
};
