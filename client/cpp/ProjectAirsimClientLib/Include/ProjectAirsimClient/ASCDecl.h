// Copyright (C) 2025 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

#pragma once

#ifndef ASC_DECL
#ifdef PROJECTAIRSIMCLIENT_STATIC
	#define ASC_DECL
#else
#ifdef _WIN32
	#ifdef PROJECTAIRSIMCLIENT_LIB_BUILD
		#define ASC_DECL __declspec(dllexport)
	#else
		#define ASC_DECL __declspec(dllimport)
	#endif  // PROJECTAIRSIMCLIENT_LIB_BUILD
#else
	#define ASC_DECL
#endif
#endif  // PROJECTAIRSIMCLIENT_STATIC
#endif  // ASC_DECL
