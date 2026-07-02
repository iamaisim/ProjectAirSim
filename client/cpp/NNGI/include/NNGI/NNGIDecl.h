#pragma once

#ifndef NNGI_DECL
#ifdef NNGI_STATIC_LIB
	#define NNGI_DECL
#else
#ifdef _WIN32
	#ifdef NNGI_EXPORTS
		#define NNGI_DECL __declspec(dllexport)
	#else
		#define NNGI_DECL __declspec(dllimport)
	#endif
#else
	#define NNGI_DECL
#endif
#endif
#endif //NNGI_DECL
