/****************************************************************************/
/// @file    AGActivityGen.h
/// @author  Piotr Woznica
/// @author  Daniel Krajzewicz
/// @author  Walter Bamberger
/// @date    July 2010
/// @version $Id$
///
// Main class that handles City, Activities and Trips
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo.sourceforge.net/
// Copyright (C) 2001-2012 DLR (http://www.dlr.de/) and contributors
// activitygen module
// Copyright 2010 TUM (Technische Universitaet Muenchen, http://www.tum.de/)
/****************************************************************************/
//
//   This file is part of SUMO.
//   SUMO is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
/****************************************************************************/
#ifndef AGACTIVITYGEN_H
#define AGACTIVITYGEN_H


// ===========================================================================
// included modules
// ===========================================================================
#include <iostream>
#include <list>
#include <string>
#include <router/RONet.h>
#include "city/AGCity.h"
#include "activities/AGTrip.h"
#include "activities/AGActivities.h"
#include "city/AGStreet.h"
#include "city/AGPosition.h"


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class AGActivityGen
 * @brief Central object handling City, Activities and Trips
 */
class AGActivityGen {
public:
    //AGActivityGen() {};
    /** @brief Constructor
     *
     * @param[in] input input stat-file name (containing information about the city)
     * @param[in] output xml file in which we'll write the routes generated
     * @param[in] net network of the city
     */
    AGActivityGen(std::string input, std::string output, RONet* net) :
        inputFile(input),
        outputFile(output),
        net(net),
        //activities(),
        city(net) {};
    /** @brief build the internal city
     *
     * TO CALL 1: First function to be called:
     * imports the XML input file and generates the whole city.
     */
    void importInfoCity();

    /**@brief build activities and trips of the population and generate routes
     *
     * TO CALL 2:
     * generates City's Activity and the corresponding trips
     *
     * @param[in] days      : duration of the simulation (>=0) (day of the end - day of the beginning)
     * @param[in] beginTime : instant of the simulation beginning (in the first day)
     * @param[in] endTime   : instant of the simulation ending (in the last day)
     * NOTE: if (days==0) : endTime > beginTime
     *
     * EXAMPLE: if days=1, endTime=0, beginTime=0: The duration
     * will be 24 hours from 12am to 12amof the next day
     */
    void makeActivityTrips(int days = 1, int beginTime = 0, int endTime = 0);

protected:
    // @brief xml files: statistics on the city and generated routes
    std::string inputFile, outputFile;
    //Activities activities;
    // @brief city object containing all households and vehicles
    AGCity city;
    // @brief time of beginning and ending of the simulation and the duration of the simulation in days (min 1 day (beginning and end in the same day)
    int durationInDays, beginTime, endTime;
    // @brief network of the city
    RONet* net;

    /**
     * @brief validation: compatibility of the given trip
     *
     * @param[in] trip to be validated
     *
     * @returns whether the trip is compatible with the time boundaries or not.
     * for this begin, end and duration of the simulation must be defined
     */
    bool timeTripValidation(AGTrip);
    /**
     * @brief generate the output file (trips or routes) using a trip list
     *
     * @param[in] trips generated by the different activities
     */
    void generateOutputFile(std::list<AGTrip>& trips);
    /**
     * @breif introduce a slight variation into the departure time of "default" vehicles
     *
     * @param[in] trip on which a random (normally distributed) variation will be tried
     */
    void varDepTime(AGTrip& trip);

private:
    /// @brief invalidated assignment operator
    AGActivityGen& operator=(const AGActivityGen&);
};

#endif

/****************************************************************************/

