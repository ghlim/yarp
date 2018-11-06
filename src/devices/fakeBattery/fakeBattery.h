/*
* Copyright (C) 2006-2018 Istituto Italiano di Tecnologia (IIT)
* All rights reserved.
*
* This software may be modified and distributed under the terms of the
* BSD-3-Clause license. See the accompanying LICENSE file for details.
*/

#ifndef __FAKEBATTERY_H__
#define __FAKEBATTERY_H__

#include <yarp/os/PeriodicThread.h>
#include <yarp/os/Semaphore.h>
#include <yarp/dev/IBattery.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/os/ResourceFinder.h>
#include <yarp/sig/Vector.h>

namespace yarp {
    namespace dev {
        class FakeBattery;
    }
}

class yarp::dev::FakeBattery : public yarp::os::PeriodicThread, public yarp::dev::IBattery, public yarp::dev::DeviceDriver
{
protected:
    yarp::os::Semaphore mutex;

    short              status;
    double             timeStamp;
    yarp::sig::Vector  data;
    double             battery_charge;
    double             battery_voltage;
    double             battery_current;
    double             battery_temperature;
    std::string        battery_info;
    unsigned char      backpack_status;

    bool logEnable;
    bool verboseEnable;
    bool screenEnable;
    bool debugEnable;
    bool shutdownEnable;

    char                log_buffer[1024];
    FILE                *logFile;
    yarp::os::ResourceFinder   rf;
    std::string         remoteName;
    std::string         localName;

public:
    FakeBattery(int period=20);
    virtual ~FakeBattery();

    virtual bool open(yarp::os::Searchable& config);
    virtual bool close();

    virtual bool getBatteryVoltage     (double &voltage);
    virtual bool getBatteryCurrent     (double &current);
    virtual bool getBatteryCharge      (double &charge);
    virtual bool getBatteryStatus      (Battery_status &status);
    virtual bool getBatteryInfo        (std::string &info);
    virtual bool getBatteryTemperature (double &temperature);

    virtual bool threadInit();
    virtual void threadRelease();
    virtual void run();

    void check_battery_status();
    void notify_message(std::string msg);
    void stop_robot(std::string quit_port);
};


#endif
