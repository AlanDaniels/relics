
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently.

#pragma once

// Dear Visual Studio: Everything will be okay.
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "targetver.h"

// Exclude rarely-used stuff from Windows headers.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// C headers.
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

// C++ headers.
#include <map>
#include <list>
#include <memory>
#include <vector>

#include <sstream>
#include <string>

// My headers.
#include "wx/wxprec.h"

#include "wx/dcbuffer.h"
#include "wx/filepicker.h"
#include "wx/gbsizer.h"
#include "wx/listctrl.h"
#include "wx/msgdlg.h"
#include "wx/rawbmp.h"
#include "wx/spinctrl.h"
#include "wx/time.h"
