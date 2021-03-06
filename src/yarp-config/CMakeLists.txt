# Copyright (C) 2006-2018 Istituto Italiano di Tecnologia (IIT)
# All rights reserved.
#
# This software may be modified and distributed under the terms of the
# BSD-3-Clause license. See the accompanying LICENSE file for details.


set(yarp_config_SRCS yarpconfig.cpp
                     yarpcontext.cpp
                     yarprobot.cpp
                     yarpcontextutils.cpp)

set(yarp_config_HDRS yarpcontext.h
                     yarprobot.h
                     yarpcontextutils.h)

add_executable(yarp-config ${yarp_config_SRCS}
                           ${yarp_config_HDRS})
target_include_directories(yarp-config SYSTEM PRIVATE ${ACE_INCLUDE_DIRS}
                                                      ${diff_match_patch_INCLUDE_DIRS})
target_link_libraries(yarp-config YARP::YARP_OS
                                  YARP::YARP_init
                                  ${ACE_LIBRARIES})
install(TARGETS yarp-config
        COMPONENT utilities
        DESTINATION ${CMAKE_INSTALL_BINDIR})

set_property(TARGET yarp-config PROPERTY FOLDER "Command Line Tools")
