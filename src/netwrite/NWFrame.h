/****************************************************************************/
/// @file    NWFrame.h
/// @author  Daniel Krajzewicz
/// @date    Tue, 20 Nov 2001
/// @version $Id$
///
// Sets and checks options for netwrite
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo.sourceforge.net/
// Copyright (C) 2001-2011 DLR (http://www.dlr.de/) and contributors
/****************************************************************************/
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
/****************************************************************************/
#ifndef NWFrame_h
#define NWFrame_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif


// ===========================================================================
// class declarations
// ===========================================================================
class OptionsCont;
class NBNetBuilder;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class NWFrame
 * @brief Sets and checks options for netwrite; saves the network
 */
class NWFrame {
public:
    /** @brief Inserts options used by the network writer
     */
    static void fillOptions();


    /** @brief Checks set options from the OptionsCont-singleton for being valid
     * @return Whether needed options are set and have proper values
     */
    static bool checkOptions();


    /** @brief Writes the network stored in the given net builder
     */
    static void writeNetwork(const OptionsCont &oc, NBNetBuilder &nb);


};


#endif

/****************************************************************************/
