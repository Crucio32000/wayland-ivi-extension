/// @file       input_policy_commands.cpp
/// @brief      Module input_policy_commands - Implementation
///   
/// @author     Domenico Nicita <domenico.nicita@artgroup-spa.com>
///   
/// @copyright
///             Copyright 2025 - Art Spa.
///             All rights reserved.
///             This file is copyrighted and the property of Art Spa.
///             It contains confidential and proprietary information. Any copies of
///             this file (in whole or in part) may only be used subject to prior
///             written permission from Art Spa.
///
/// @note       Module based on the Template "SWC Template Cpp Language" version 1.1.1
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include "ilm_input_policy.h"
#include "LMControl.h"
#include "Expression.h"
#include "ExpressionInterpreter.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <map>
#include <algorithm>
#include <iterator>
#include <cstring>
#include <signal.h> // signal
#include <unistd.h> // alarm

using namespace std;


#define COMMAND(text) COMMAND2(__COUNTER__,text)

#define COMMAND2(x,y) COMMAND3(x,y)

#define COMMAND3(funcNumber, text) \
    void artFunc_ ## funcNumber(Expression* input); \
    static const bool reg_ ## funcNumber = \
        ExpressionInterpreter::addExpression(artFunc_ ## funcNumber, text); \
    void artFunc_ ## funcNumber(Expression* input)


//=============================================================================
//          ART SPECIFICS
//=============================================================================
//=============================================================================
COMMAND("add surface <sid> input region <x> <y> <w> <h>")
//=============================================================================
{
    t_ilm_surface surfaceid = input->getUint("sid");
    t_ilm_int x = input->getInt("x");
    t_ilm_int y = input->getInt("y");
    t_ilm_int w = input->getInt("w");
    t_ilm_int h = input->getInt("h");

    ilmErrorTypes callResult = ilm_addInputRectangle(surfaceid, x, y, w, h);

    if (ILM_SUCCESS != callResult)
    {
        cout << "LayerManagerService returned: " << ILM_ERROR_STRING(callResult) << "\n";
        cout << "Failed to add input region (" << x << ", " << y << ", " << w << ", " << h << ") for surface with ID " << surfaceid << "\n";
        return;
    }
}

//=============================================================================
COMMAND("clear surface <sid> input regions")
//=============================================================================
{
    t_ilm_surface surfaceid = input->getUint("sid");

    ilmErrorTypes callResult = ilm_clearInputRegions(surfaceid);

    if (ILM_SUCCESS != callResult)
    {
        cout << "LayerManagerService returned: " << ILM_ERROR_STRING(callResult) << "\n";
        cout << "Failed to clear input regions for surface with ID " << surfaceid << "\n";
        return;
    }
}

//=============================================================================
COMMAND("reset surface <sid> input region")
//=============================================================================
{
    t_ilm_surface surfaceid = input->getUint("sid");

    ilmErrorTypes callResult = ilm_resetInputRegions(surfaceid);

    if (ILM_SUCCESS != callResult)
    {
        cout << "LayerManagerService returned: " << ILM_ERROR_STRING(callResult) << "\n";
        cout << "Failed to reset input regions for surface with ID " << surfaceid << "\n";
        return;
    }
}

//=============================================================================
COMMAND("commit surface <sid> input regions")
//=============================================================================
{
    t_ilm_surface surfaceid = input->getUint("sid");

    ilmErrorTypes callResult = ilm_commitInputRegion(surfaceid);

    if (ILM_SUCCESS != callResult)
    {
        cout << "LayerManagerService returned: " << ILM_ERROR_STRING(callResult) << "\n";
        cout << "Failed to clear input regions for surface with ID " << surfaceid << "\n";
        return;
    }
}