// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.

// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#include "framework.h"

#include "pose_stamped_message.hpp"
#include "request_message.hpp"
#include "response_message.hpp"

#include <core_sim/message/response_failure_message.hpp>
#include <core_sim/message/response_success_message.hpp>
#include <core_sim/src/message/common_utils.hpp>
#include <core_sim/src/message/message_impl.hpp>

#include <sstream>
#include <fstream>
#include <iostream>
#include <sstream>

#include <json.hpp>
#include <msgpack.hpp>

#endif //PCH_H
