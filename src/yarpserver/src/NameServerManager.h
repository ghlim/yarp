// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
 * Copyright (C) 2009 Paul Fitzpatrick
 * CopyPolicy: Released under the terms of the GNU GPL v2.0.
 *
 */

#ifndef YARPDB_NAMESERVERMANAGER_INC
#define YARPDB_NAMESERVERMANAGER_INC

#include "NameServerConnectionHandler.h"

#include <yarp/os/PortReaderCreator.h>
#include <yarp/os/Semaphore.h>
#include <yarp/os/Port.h>

/**
 *
 * Manage the name server.
 *
 */
class NameServerManager : public NameService, 
                          public yarp::os::PortReaderCreator {
private:
    NameService& ns;
    yarp::os::Port *port;
    yarp::os::Semaphore mutex;
public:
    NameServerManager(NameService& ns, 
                      yarp::os::Port *port = NULL) : ns(ns),
                                                     port(port), mutex(1) {
    }

    void setPort(yarp::os::Port& port) {
        this->port = &port;
    }

    virtual void onEvent(yarp::os::Bottle& event) {
        ns.onEvent(event);
        if (port!=NULL) {
            port->write(event);
        }
    }

    virtual yarp::os::PortReader *create() {
        return new NameServerConnectionHandler(this);
    }

    virtual void lock() {
        mutex.wait();
    }

    virtual void unlock() {
        mutex.post();
    }

    virtual bool apply(yarp::os::Bottle& cmd, 
                       yarp::os::Bottle& reply, 
                       yarp::os::Bottle& event, 
                       yarp::os::Contact& remote) {
        bool ok = ns.apply(cmd,reply,event,remote);
        return ok;
    }
};

#endif
