/****************************************************************************/
/// @file    MSDetectorFileOutput.h
/// @author  Christian Roessel
/// @date    2004-11-23
/// @version $Id$
///
//	missingDescription
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo.sourceforge.net/
// copyright : (C) 2001-2007
//  by DLR (http://www.dlr.de/) and ZAIK (http://www.zaik.uni-koeln.de/AFS)
/****************************************************************************/
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
/****************************************************************************/
#ifndef MSDetectorFileOutput_h
#define MSDetectorFileOutput_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <string>
#include <microsim/MSUnit.h>
#include <utils/common/SUMOTime.h>
#include <utils/iodevices/OutputDevice.h>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSDetectorFileOutput
 *
 * Pure virtual base class for classes (e.g. MSLaneState and
 * MSInductLoop) that should produce XML-output via the
 * MSDetector2File or MSTravelcostDetector singletons.
 *
 * @see MSDetector2File
 * @see MSTravelcostDetector
 * @see MSLaneState
 * @see MSInductLoop
 */
class MSDetectorFileOutput
{
public:
    /// (virtual) destructor
    virtual ~MSDetectorFileOutput()
    { }

    /**
     * Get the XML-formatted output of the concrete detector.
     *
     * @param lastNTimesteps Generate data out of the interval
     * (now-lastNTimesteps, now].
     */
    virtual void writeXMLOutput(OutputDevice &dev,
                                SUMOTime startTime, SUMOTime stopTime) = 0;

    /**
     * Get an opening XML-element containing information about the detector.
     */
    virtual void writeXMLDetectorProlog(OutputDevice &dev) const = 0;

};


#endif

/****************************************************************************/

