
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently

#pragma once

#include "targetver.h"

// Dear Windows, I'm just printf-ing stuff. It's okay.
#define _CRT_SECURE_NO_WARNINGS

// Exclude rarely-used stuff from Windows headers.
#define WIN32_LEAN_AND_MEAN

// The Windows header.
#include <windows.h>

// C++ headers.
#include <array>
#include <bitset>
#include <cctype>
#include <fstream>
#include <iterator>
#include <memory>
#include <sstream>

#include <future>

// C run-time headers.
#include <assert.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

// GL Extension Wrangler.
// NOTE: Be mindful of OpenGL ES compatibility, but remember that it's not
// generally available for desktops (ES stands for "Embedded Systems").
#include <GL/glew.h>

// SFML headers.
#include <SFML/Graphics.hpp>


// TODO: This adds memory leak detection.
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

// Some macros to bullet-proof our classes.
#define DEFAULT_COPYING(ClassName) \
    ClassName(const ClassName &) = default; \
    ClassName &operator=(const ClassName &) = default;

#define DEFAULT_MOVING(ClassName) \
    ClassName(ClassName &&) = default; \
    ClassName &operator=(ClassName &&) = default;


#define FORBID_DEFAULT_CTOR(ClassName) \
    ClassName() = delete;

#define FORBID_COPYING(ClassName) \
    ClassName(const ClassName &) = delete; \
    void operator=(const ClassName &) = delete;

#define FORBID_MOVING(ClassName) \
    ClassName(ClassName &&) = delete; \
    void operator=(ClassName &&) = delete;
