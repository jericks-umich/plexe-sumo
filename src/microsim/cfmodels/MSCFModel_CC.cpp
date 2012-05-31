/****************************************************************************/
/// @file    MSCFModel_CC.cpp
/// @author  Michele Segata
/// @date    Wed, 18 Apr 2012
/// @version $Id: $
///
// A series of automatic Cruise Controllers (CC, ACC, CACC)
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


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include "MSCFModel_CC.h"
#include <microsim/MSVehicle.h>
#include <microsim/MSLane.h>
#include <utils/common/RandHelper.h>
#include <utils/common/SUMOTime.h>


// ===========================================================================
// method definitions
// ===========================================================================
MSCFModel_CC::MSCFModel_CC(const MSVehicleType* vtype,
                           SUMOReal accel, SUMOReal decel,
                           SUMOReal ccDecel, SUMOReal headwayTime, SUMOReal constantSpacing,
                           SUMOReal kp, SUMOReal lambda, SUMOReal c1, SUMOReal xi,
                           SUMOReal omegaN, SUMOReal tau)
    : MSCFModel(vtype, accel, decel, headwayTime), myCcDecel(ccDecel), myConstantSpacing(constantSpacing)
    , myKp(kp), myLambda(lambda), myC1(c1), myXi(xi), myOmegaN(omegaN), myTau(tau), myAlpha1(1 - myC1), myAlpha2(myC1),
    myAlpha3(-(2 * myXi - myC1 *(myXi + sqrt(myXi* myXi - 1))) * myOmegaN), myAlpha4(-(myXi + sqrt(myXi* myXi - 1)) * myOmegaN* myC1),
    myAlpha5(-myOmegaN* myOmegaN), myAlpha(TS / (myTau + TS)), myOneMinusAlpha(1 - myAlpha) {

    if (DELTA_T != 100) {
        std::cerr << "Fatal error: in order to use automatic cruise control models, time step must be set to 100 ms\n";
        assert(false);
    }

    //instantiate the driver model. For now, use Krauss as default, then needs to be parameterized
    myHumanDriver = new MSCFModel_Krauss(vtype, accel, decel, 0.5, 1.5);
}

MSCFModel_CC::~MSCFModel_CC() {}

SUMOReal
MSCFModel_CC::moveHelper(MSVehicle* const veh, SUMOReal vPos) const {
    SUMOReal vNext;
    VehicleVariables *vars = (VehicleVariables *)veh->getCarFollowVariables();

    if (vars->activeController != MSCFModel_CC::DRIVER) {
        //if during this simulation step the speed has been set by the followSpeed() method, then use such value
        if (vars->followSpeedSetTime == MSNet::getInstance()->getCurrentTimeStep()) {
            vNext = vars->controllerFollowSpeed;
        }
        //otherwise use the value set by the freeSpeed() method
        else {
            vNext = vars->controllerFreeSpeed;
        }
    }
    else
        vNext = myHumanDriver->moveHelper(veh, vPos);

    //update ego data

    //this method should be called only once per vehicle per simulation step. is it true?
    assert(vars->egoDataLastUpdate != MSNet::getInstance()->getCurrentTimeStep());

    vars->egoPreviousSpeed = vars->egoSpeed;
    vars->egoSpeed = vNext;
    vars->egoAcceleration = SPEED2ACCEL(vars->egoSpeed - vars->egoPreviousSpeed);
    vars->egoDataLastUpdate = MSNet::getInstance()->getCurrentTimeStep();

    return vNext;
}


SUMOReal
MSCFModel_CC::followSpeed(const MSVehicle* const veh, SUMOReal speed, SUMOReal gap2pred, SUMOReal predSpeed, SUMOReal predMaxDecel) const {

    VehicleVariables *vars = (VehicleVariables *)veh->getCarFollowVariables();

    //has this function already been invoked for this timestep?
    if (vars->frontDataLastUpdate != MSNet::getInstance()->getCurrentTimeStep() && !vars->ignoreModifications) {
        //relative speed at time now - 1
        double dv_t0 = vars->egoPreviousSpeed - vars->frontSpeed;
        //relative speed at time now
        double dv_t1 = speed - predSpeed;

        vars->frontDataLastUpdate = MSNet::getInstance()->getCurrentTimeStep();
        vars->frontAcceleration = vars->egoAcceleration - SPEED2ACCEL(dv_t1 - dv_t0);
        //if we receive the first update of the simulation about the vehicle in front, we might save a wrong acceleration
        //value, because we had no previous speed value, so we might save an acceleration of more than 10g for example
        //so we can just set it to 0. at the next time step (100ms) it will be correctly updated
        if (vars->frontAcceleration > 10 || vars->frontAcceleration < -10)
            vars->frontAcceleration = 0;
        vars->frontSpeed = predSpeed;
        vars->frontDistance = gap2pred;
    }

    if (vars->activeController != MSCFModel_CC::DRIVER) {
        return _v(veh, gap2pred, speed, predSpeed, desiredSpeed(veh), MSCFModel_CC::FOLLOW_SPEED);
    }
    else
        return myHumanDriver->followSpeed(veh, speed, gap2pred, predSpeed, predMaxDecel);
}


SUMOReal
MSCFModel_CC::stopSpeed(const MSVehicle* const veh, SUMOReal gap2pred) const {

    /**
     * This SUMO method is meant for asking the car what speed the driver would
     * apply if it would have to stop in gap2pred meters. This method is called
     * by the SUMO engine when a approaching a junction, asking something like
     * "what would you do if...?". So this method should return the current speed
     * if the model thinks that there is more than enough space to stop,
     * otherwise it should return a reduced speed. SUMO, then, will use the
     * reduced speed in two cases:
     * 1) there is the real need for using that: for example, we are approaching
     * an intersection where the traffic light is red
     * 2) we are very far from the intersection: in this case SUMO still does not
     * know whether the car will have to stop (e.g., it does not know if the
     * traffic light will be red or not), so in this case SUMO decides to use the
     * reduced speed, because IF the car will have to stop, by using this reduced
     * speed it will be able to do that.
     *
     * Point number 2) causes a problem with ACC: the reaction of the ACC to a
     * completely stopped obstacle in front (this is how a red traffic light is
     * actually modeled) is applying a huge deceleration which, combined with all
     * the really complicated logics behind SUMO, makes the car stop at every
     * junction, even if this is a simple connection between two edges.
     * For making this work there is an easy and logic workaround: if the CC
     * is switched on, then we just return the current speed. This is actually
     * correct: the radar is only able to detect real obstacles, and not
     * intersections, so in reality the ACC controller will just let the car
     * go. It is a duty of the driver to switch the CC off before an intersection.
     *
     * If the CC is switched off, then we just call the stopSpeed method of the
     * installed carFollowing model simulating the real driver.
     *
     * EVEN WORSE: by simply returning the current speed you might never accelerate
     * when needed. So when the CC is enabled we just return a really high value
     */

    VehicleVariables *vars = (VehicleVariables *)veh->getCarFollowVariables();
    if (vars->activeController != MSCFModel_CC::DRIVER)
    {
        return 1e6;
    }
    else {
        return myHumanDriver->stopSpeed(veh, gap2pred);
    }
}

SUMOReal MSCFModel_CC::freeSpeed(const MSVehicle* const veh, SUMOReal speed, SUMOReal seen, SUMOReal maxSpeed) const {
    VehicleVariables *vars = (VehicleVariables *)veh->getCarFollowVariables();
    if (vars->activeController != MSCFModel_CC::DRIVER)
    {
        return _v(veh, seen, speed, maxSpeed, desiredSpeed(veh), MSCFModel_CC::FREE_SPEED);
    }
    else {
        return MSCFModel::freeSpeed(veh, speed, seen, maxSpeed);
    }
}

SUMOReal
MSCFModel_CC::interactionGap(const MSVehicle* const veh, SUMOReal vL) const {

    VehicleVariables *vars = (VehicleVariables *)veh->getCarFollowVariables();
    if (vars->activeController != MSCFModel_CC::DRIVER)
    {
        //maximum radar range is CC is enabled
        return 250;
    }
    else {
        return myHumanDriver->interactionGap(veh, vL);
    }

}

SUMOReal
MSCFModel_CC::maxNextSpeed(SUMOReal speed) const {
    return speed + (SUMOReal) ACCEL2SPEED(getMaxAccel());
}

SUMOReal
MSCFModel_CC::_v(const MSVehicle* const veh, SUMOReal gap2pred, SUMOReal egoSpeed, SUMOReal predSpeed, SUMOReal desSpeed, enum CONTROLLER_INVOKER invoker) const {

    //acceleration computed by the controller
    double controllerAcceleration;
    //acceleration actually actuated by the engine
    double engineAcceleration;
    //speed computed by the model
    double speed;
    //acceleration computed by the Cruise Control
    double ccAcceleration;
    //acceleration computed by the Adaptive Cruise Control
    double accAcceleration;
    //acceleration computed by the Cooperative Adaptive Cruise Control
    double caccAcceleration;

    bool debug = true;

    VehicleVariables* vars = (VehicleVariables*) veh->getCarFollowVariables();

    std::string id = veh->getID();

    switch (vars->activeController) {

        case MSCFModel_CC::ACC:

            ccAcceleration = _cc(egoSpeed, vars->ccDesiredSpeed);
            accAcceleration = _acc(egoSpeed, predSpeed, gap2pred);

            if (gap2pred > 250 || ccAcceleration < accAcceleration) {
                controllerAcceleration = ccAcceleration;
            }
            else {
                controllerAcceleration = accAcceleration;
            }


            break;

        case MSCFModel_CC::CACC:

            double predAcceleration, leaderAcceleration, leaderSpeed;

            if (invoker == MSCFModel_CC::FOLLOW_SPEED) {
                predAcceleration = vars->frontAcceleration;
                leaderAcceleration = vars->leaderAcceleration;
                leaderSpeed = vars->leaderSpeed;
            }
            else {
                /* if the method has not been invoked from followSpeed() then it has been
                 * invoked from stopSpeed(). In such case we set all parameters of preceding
                 * vehicles as they were non-moving obstacles
                 */
                predAcceleration = 0;
                leaderAcceleration = 0;
                leaderSpeed = 0;
            }

            //TODO: again modify probably range/range-rate controller is needed
            ccAcceleration = _cc(egoSpeed, vars->ccDesiredSpeed);
            caccAcceleration = _cacc(egoSpeed, predSpeed, predAcceleration, gap2pred, leaderSpeed, leaderAcceleration);
            controllerAcceleration = fmin(ccAcceleration, caccAcceleration);

            break;

        case MSCFModel_CC::DRIVER:

            std::cerr << "Switching to normal driver behavior still not implemented in MSCFModel_CC\n";
            assert(false);
            break;

        default:

            std::cerr << "Invalid controller selected in MSCFModel_CC\n";
            assert(false);
            break;

    }

    //compute the actual acceleration applied by the engine
    engineAcceleration = _actuator(controllerAcceleration, vars->egoAcceleration);

    //compute the speed from the actual acceleration
    speed = egoSpeed + ACCEL2SPEED(engineAcceleration);

    //if we have to ignore modifications (e.g., when this method is invoked by the lane changing logic)
    //DO NOT change the state of the vehicle
    if (!vars->ignoreModifications) {

        if (invoker == MSCFModel_CC::FOLLOW_SPEED && vars->followSpeedSetTime != MSNet::getInstance()->getCurrentTimeStep()) {
            vars->controllerFollowSpeed = speed;
            vars->followSpeedSetTime = MSNet::getInstance()->getCurrentTimeStep();
        }

        if (invoker == MSCFModel_CC::FREE_SPEED)
            vars->controllerFreeSpeed = speed;

    }

    return MAX2(SUMOReal(0), speed);
}

SUMOReal
MSCFModel_CC::_cc(SUMOReal egoSpeed, SUMOReal desSpeed) const {

    //Eq. 5.5 of the Rajamani book, with Ki = 0 and bounds on max and min acceleration
    return fmin(myAccel, fmax(-myCcDecel, -myKp * (egoSpeed - desSpeed)));

}

SUMOReal
MSCFModel_CC::_acc(SUMOReal egoSpeed, SUMOReal predSpeed, SUMOReal gap2pred) const {

    //Eq. 6.18 of the Rajamani book
    return fmin(myAccel, fmax(-myDecel, -1.0 / myHeadwayTime * (egoSpeed - predSpeed + myLambda * (-gap2pred + myHeadwayTime * egoSpeed))));

}

SUMOReal
MSCFModel_CC::_cacc(SUMOReal egoSpeed, SUMOReal predSpeed, SUMOReal predAcceleration, SUMOReal gap2pred, SUMOReal leaderSpeed, SUMOReal leaderAcceleration) const {

    //compute epsilon, i.e., the desired distance error
    double epsilon = -gap2pred + myConstantSpacing; //NOTICE: error (if any) should already be included in gap2pred
    //compute epsilon_dot, i.e., the desired speed error
    double epsilon_dot = egoSpeed - predSpeed;
    //Eq. 7.39 of the Rajamani book
    return fmin(myAccel, fmax(-myDecel, myAlpha1 * predAcceleration + myAlpha2 * leaderAcceleration +
                              myAlpha3 * epsilon_dot + myAlpha4 * (egoSpeed - leaderSpeed) + myAlpha5 * epsilon));

}

SUMOReal
MSCFModel_CC::_actuator(SUMOReal acceleration, SUMOReal currentAcceleration) const {

    //standard low-pass filter discrete implementation
    return myAlpha * acceleration + myOneMinusAlpha * currentAcceleration;

}

void
MSCFModel_CC::setCCDesiredSpeed(const MSVehicle* veh, SUMOReal ccDesiredSpeed) const {
    VehicleVariables* vars = (VehicleVariables*) veh->getCarFollowVariables();
    vars->ccDesiredSpeed = ccDesiredSpeed;
}

void
MSCFModel_CC::setLeaderInformation(const MSVehicle* veh, SUMOReal speed, SUMOReal acceleration) const {
    VehicleVariables* vars = (VehicleVariables*) veh->getCarFollowVariables();
    vars->leaderAcceleration = acceleration;
    vars->leaderSpeed = speed;
}

void
MSCFModel_CC::getVehicleInformation(const MSVehicle* veh, SUMOReal& speed, SUMOReal& acceleration) const {
    VehicleVariables* vars = (VehicleVariables*) veh->getCarFollowVariables();
    speed = vars->egoSpeed;
    acceleration = vars->egoAcceleration;
}

void MSCFModel_CC::switchOnACC(const MSVehicle *veh, double ccDesiredSpeed)  const {
    VehicleVariables* vars = (VehicleVariables*) veh->getCarFollowVariables();
    vars->ccDesiredSpeed = ccDesiredSpeed;
    vars->activeController = MSCFModel_CC::ACC;
}

void MSCFModel_CC::setActiveController(const MSVehicle *veh, enum MSCFModel_CC::ACTIVE_CONTROLLER activeController) const {
    VehicleVariables* vars = (VehicleVariables*) veh->getCarFollowVariables();
    vars->activeController = activeController;
}

enum MSCFModel_CC::ACTIVE_CONTROLLER MSCFModel_CC::getActiveController(const MSVehicle *veh) const {
    VehicleVariables* vars = (VehicleVariables*) veh->getCarFollowVariables();
    return vars->activeController;
}

enum MSCFModel_CC::PLATOONING_LANE_CHANGE_ACTION MSCFModel_CC::getLaneChangeAction(const MSVehicle *veh) const {
    VehicleVariables* vars = (VehicleVariables*) veh->getCarFollowVariables();
    return vars->laneChangeAction;
}

void MSCFModel_CC::setLaneChangeAction(const MSVehicle *veh, enum MSCFModel_CC::PLATOONING_LANE_CHANGE_ACTION action) const {
    VehicleVariables* vars = (VehicleVariables*) veh->getCarFollowVariables();
    vars->laneChangeAction = action;
}

void MSCFModel_CC::getRadarMeasurements(const MSVehicle * veh, double &distance, double &relativeSpeed) const {
    VehicleVariables* vars = (VehicleVariables*) veh->getCarFollowVariables();
    distance = vars->frontDistance;
    relativeSpeed = vars->frontSpeed - vars->egoSpeed;
}

void MSCFModel_CC::setControllerFakeData(const MSVehicle *veh, double frontDistance, double frontSpeed, double frontAcceleration,
            double leaderSpeed, double leaderAcceleration) const {

    VehicleVariables *vars = (VehicleVariables *) veh->getCarFollowVariables();
    vars->fakeData.frontAcceleration = frontAcceleration;
    vars->fakeData.frontDistance = frontDistance;
    vars->fakeData.frontSpeed = frontSpeed;
    vars->fakeData.leaderAcceleration = leaderAcceleration;
    vars->fakeData.leaderSpeed = leaderSpeed;
}

void MSCFModel_CC::setIgnoreModifications(const MSVehicle *veh, bool ignore) const {
    VehicleVariables *vars = (VehicleVariables *) veh->getCarFollowVariables();
    vars->ignoreModifications = ignore;
}

MSCFModel*
MSCFModel_CC::duplicate(const MSVehicleType* vtype) const {
    return new MSCFModel_CC(vtype,
                            myAccel, myDecel,
                            myCcDecel, myHeadwayTime, myConstantSpacing,
                            myKp, myLambda, myC1, myXi,
                            myOmegaN, myTau);
}