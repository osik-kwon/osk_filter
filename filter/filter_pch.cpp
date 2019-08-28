#include "filter_pch.h"

#ifndef LOAD_CRYPTPP_LIBRARY
#define LOAD_CRYPTPP_LIBRARY

#ifdef _WIN32
#ifdef _DEBUG
#  pragma comment ( lib, "cryptlib_mdd" )
#else
#  pragma comment ( lib, "cryptlib_md" )
#endif
#endif

// TODO: implement x64
// TODO: implement linux, apple
#endif