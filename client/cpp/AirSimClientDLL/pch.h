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

#define _CRT_SECURE_NO_WARNINGS
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING // <codecvt> is deprecated but there's no replacement yet (2/7/2023)
#define WINDOWS_LEAN_AND_MEAN
#define NOMINMAX

#include "framework.h"
#include "resource.h"

#include "AirSimClient.h"
#include "ASCDecl.h"
#include "AsyncResult.h"
#include "Client.h"
#include "Common.h"
#include "Drone.h"
#include "IRefCounted.h"
#include "Log.h"
#include "Message.h"
#include "Status.h"
#include "Types.h"
#include "World.h"

#include "../../src/AsyncResultInternal.h"
#include "../../src/ICommChannel.h"
#include "../../src/IMessageBuffer.h"
#include "../../src/JSONUtils.h"
#include "../../src/RefCounted.h"
#include "../../src/Topics.h"
#include "../../src/Utils.h"

#include "AirSimMessage/jsonc.hpp"
#include <AirSimMessage/request_message.hpp>
#include <AirSimMessage/response_message.hpp>

#include "core_sim/math_utils.hpp"

#include <NNGI/NNGI.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <chrono>
#include <codecvt>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

#endif //PCH_H
