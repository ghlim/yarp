/*
 * Copyright (C) 2006-2018 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * BSD-3-Clause license. See the accompanying LICENSE file for details.
 */

#include <yarp/os/NetworkClock.h>

#include <yarp/conf/numeric.h>
#include <yarp/conf/system.h>

#include <yarp/os/Mutex.h>
#include <yarp/os/NestedContact.h>
#include <yarp/os/NetInt32.h>
#include <yarp/os/Network.h>
#include <yarp/os/Os.h>
#include <yarp/os/Port.h>
#include <yarp/os/PortReader.h>
#include <yarp/os/Semaphore.h>
#include <yarp/os/SystemClock.h>
#include <yarp/os/SystemInfo.h>
#include <yarp/os/impl/Logger.h>

#include <cstring>
#include <list>
#include <utility>


using namespace yarp::os;
using namespace yarp::os::impl;


class NetworkClock::Private : public yarp::os::PortReader
{
public:
    Private();
    ~Private();

    bool read(ConnectionReader& reader) override;

    std::string clockName;

    using Waiters = std::list<std::pair<double, Semaphore*>>;
    Waiters* waiters;
    Port port;

    Mutex listMutex;
    Mutex timeMutex;

    std::int32_t sec{0};
    std::int32_t nsec{0};
    double _time{0};
    bool closing{false};
    bool initted{false};
};

NetworkClock::Private::Private() :
        waiters(new Waiters())
{
}

NetworkClock::Private::~Private()
{
    listMutex.lock();
    closing = true;
    port.interrupt();

    auto waiter_it = waiters->begin();
    while (waiter_it != waiters->end()) {
        Semaphore* waiterSemaphore = waiter_it->second;
        waiter_it = waiters->erase(waiter_it);
        if (waiterSemaphore) {
            waiterSemaphore->post();
        }
    }
    delete waiters;
    listMutex.unlock();

    yarp::os::ContactStyle style;
    style.persistent = true;
    NetworkBase::disconnect(clockName, port.getName(), style);
}

bool NetworkClock::Private::read(ConnectionReader& reader)
{
    Bottle bot;
    bool ok = bot.read(reader);

    if (closing) {
        _time = -1;
        return false;
    }

    if (!ok && !closing) {
        YARP_ERROR(Logger::get(), "Error reading clock port");
        return false;
    }

    timeMutex.lock();
    sec = bot.get(0).asInt32();
    nsec = bot.get(1).asInt32();
    _time = sec + (nsec * 1e-9);
    initted = true;
    timeMutex.unlock();

    listMutex.lock();
    auto waiter_it = waiters->begin();
    while (waiter_it != waiters->end()) {
        if (waiter_it->first - _time < 1E-12) {
            Semaphore* waiterSemaphore = waiter_it->second;
            waiter_it = waiters->erase(waiter_it);
            if (waiterSemaphore) {
                waiterSemaphore->post();
            }
        } else {
            ++waiter_it;
        }
    }
    listMutex.unlock();
    return true;
}


NetworkClock::NetworkClock() :
        mPriv(new Private)
{
}

NetworkClock::~NetworkClock()
{
    YARP_WARN(Logger::get(), "Destroying network clock");
    delete mPriv;
}

bool NetworkClock::open(const std::string& clockSourcePortName, std::string localPortName)
{
    mPriv->port.setReadOnly();
    mPriv->port.setReader(*mPriv);
    NestedContact nc(clockSourcePortName);
    mPriv->clockName = clockSourcePortName;
    yarp::os::ContactStyle style;
    style.persistent = true;

    if (localPortName == "") {
        const int MAX_STRING_SIZE = 255;
        char hostName[MAX_STRING_SIZE];
        yarp::os::gethostname(hostName, MAX_STRING_SIZE);

        yarp::os::SystemInfo::ProcessInfo processInfo = yarp::os::SystemInfo::getProcessInfo();

        localPortName = "/";
        // Ports may be anonymous to not pollute the yarp name list
        localPortName += std::string(hostName) + "/" + processInfo.name + "/" + std::string(std::to_string(processInfo.pid)) + "/clock:i";
    }

    // if receiving port cannot be opened, return false.
    bool ret = mPriv->port.open(localPortName);
    if (!ret) {
        return false;
    }

    if (nc.getNestedName() == "") {
        Contact src = NetworkBase::queryName(clockSourcePortName);

        ret = NetworkBase::connect(clockSourcePortName, mPriv->port.getName(), style);

        if (!src.isValid()) {
            fprintf(stderr, "Cannot find time port \"%s\"; for a time topic specify \"%s@\"\n", clockSourcePortName.c_str(), clockSourcePortName.c_str());
        }
    }

    return ret;
}

double NetworkClock::now()
{
    mPriv->timeMutex.lock();
    double result = mPriv->_time;
    mPriv->timeMutex.unlock();
    return result;
}

void NetworkClock::delay(double seconds)
{
    if (seconds <= 1E-12) {
        return;
    }

    mPriv->listMutex.lock();
    if (mPriv->closing) {
        // We are shutting down.  The time signal is no longer available.
        // Make a short delay and return.
        mPriv->listMutex.unlock();
        SystemClock::delaySystem(seconds);
        return;
    }

    std::pair<double, Semaphore*> waiter(now() + seconds, new Semaphore(0));
    mPriv->waiters->push_back(waiter);
    mPriv->listMutex.unlock();

    waiter.second->wait();
    delete waiter.second;
    waiter.second = nullptr;
}

bool NetworkClock::isValid() const
{
    return mPriv->initted;
}
