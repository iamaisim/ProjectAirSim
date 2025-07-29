// Copyright (C) Microsoft Corporation.  
// Copyright (C) 2025 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

#pragma once

#ifndef ASC_DECL
#ifdef AIRSIMCLIENT_LIB_BUILD
#define ASC_DECL __declspec(dllexport)
#else
#define ASC_DECL __declspec(dllimport)
#endif  // AIRSIMCLIENT_LIB_BUILD
#endif  // ASC_DECL
