/*
 * Copyright (C) 2006-2018 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * BSD-3-Clause license. See the accompanying LICENSE file for details.
 */

#include <yarp/dev/IJoypadController.h>
#include <yarp/dev/DeviceDriver.h>
#include <yarp/os/PeriodicThread.h>
#include <vector>
#include <JoypadControlNetUtils.h>

/**
* @ingroup dev_impl_network_clients
*
* \section JoypadControlClient Description of input parameters
* \brief joypad input network wrapper on client side
*
* Parameters accepted in the config argument of the open method:
* |   Parameter name  | Type   | Units | Default Value | Required  | Description                                  | Notes |
* |:-----------------:|:------:|:-----:|:-------------:|:---------:|:--------------------------------------------:|:-----:|
* | local             | string |       |               | yes       | name for the local port to open              |       |
* | remote            | string |       |               | yes       | name of the remote server port to connect to |       |
**/

#define DEFAULT_THREAD_PERIOD 10
namespace yarp
{
    namespace dev
    {
        class JoypadControlClient;
        class JoypadControlWatchdog;
    }
}

class yarp::dev::JoypadControlWatchdog : public yarp::os::PeriodicThread
{
public:
    JoypadControlWatchdog() : PeriodicThread(0.250) {}
    virtual ~JoypadControlWatchdog() = default;


    std::vector<JoypadControl::LoopablePort*> m_ports;
    virtual void run() YARP_OVERRIDE;

};

class yarp::dev::JoypadControlClient : public yarp::dev::IJoypadEventDriven,
                                       public yarp::dev::DeviceDriver
{
private:
    //---------------utils
    #define JOYPORT yarp::dev::JoypadControl::JoyPort
    typedef yarp::sig::Vector                  Vector;
    typedef yarp::sig::VectorOf<unsigned char> CharVector;

    //---------------properties
    yarp::os::Port        m_rpcPort;
    JOYPORT<Vector>       m_buttonsPort;
    JOYPORT<Vector>       m_axisPort;
    JOYPORT<Vector>       m_trackballPort;
    JOYPORT<Vector>       m_touchPort;
    JOYPORT<CharVector>   m_hatsPort;
    JOYPORT<Vector>       m_stickPort;
    std::vector<size_t>   m_stickDof;
    bool                  m_rpc_only;
    std::string m_local;
    std::string m_remote;

    yarp::dev::JoypadControlWatchdog          watchdog;
    std::vector<JoypadControl::LoopablePort*> m_ports;

    //--------------method
    bool getCount(const int& vocab_toget, unsigned int& value);
    bool getJoypadInfo();

public:
    JoypadControlClient();
    virtual ~JoypadControlClient() {}

    //rateThread

    //DeviceDriver
    bool open(yarp::os::Searchable& config) override;
    bool close() override;

    //IJoypadController;
    bool getRawAxisCount(unsigned int& axis_count) override;
    bool getRawButtonCount(unsigned int& button_count) override;
    bool getRawTrackballCount(unsigned int& Trackball_count) override;
    bool getRawHatCount(unsigned int& Hat_count) override;
    bool getRawTouchSurfaceCount(unsigned int& touch_count) override;
    bool getRawStickCount(unsigned int& stick_count) override;
    bool getRawStickDoF(unsigned int stick_id, unsigned int& DoF) override;
    bool getRawButton(unsigned int button_id, float& value) override;
    bool getRawTrackball(unsigned int trackball_id, yarp::sig::Vector& value) override;
    bool getRawHat(unsigned int hat_id, unsigned char& value) override;
    bool getRawAxis(unsigned int axis_id, double& value) override;
    bool getRawStick(unsigned int stick_id, yarp::sig::Vector& value, JoypadCtrl_coordinateMode coordinate_mode) override;
    bool getRawTouch(unsigned int touch_id, yarp::sig::Vector& value) override;

    #undef JoyPort

};
