/* ***** BEGIN LICENSE BLOCK Version: GPL 3.0 ***** 
 * Copyright (C) 2008-2011  Hayaki Saito <user@zuse.jp>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * ***** END LICENSE BLOCK ***** */

//////////////////////////////////////////////////////////////////////////////
//
// @file shell.cpp
// @brief ecmascript interperter shell
//

// configuration -->

#define ES_USE_DLL 1

// trace
//#define ES_TRACE_NODE 1
#define ES_TRACE_PARSING_TIME 1
#define ES_TRACE_RUNNING_TIME 1
#define ES_TRACE_COMPILE_TIME 1

// debug
#define ES_DEBUG

// configuration <--
#if defined(_DEBUG) && !defined(ES_DEBUG)
#    define ES_DEBUG 1
#endif  // _DEBUG

#ifdef _WIN32
#  if _MSC_VER
#    define _SECURE_SCL 0
#    define _SECURE_SCL_THROWS 0
#    define _HAS_ITERATOR_DEBUGGING 0
#    define _CRT_SECURE_NO_WARNINGS
#    pragma warning(disable: 4996)
#  endif // _MSC_VER
#  ifndef _UNICODE
#    define _UNICODE
#  endif
#  ifndef UNICODE
#    define UNICODE
#  endif
#  ifndef _MBCS
#    undef _MBCS
#  endif
#  ifndef MBCS
#    undef MBCS
#  endif
#endif

#if defined(_WIN32) 
#  define VC_EXTRALEAN
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif // _WIN32

namespace ecmascript {
    extern "C" int es_main(int argc, char* argv[]);
} // namespace ecmascript

int main(int argc, char* argv[])
{
    using namespace ecmascript;
    
#if defined(_WIN32) && defined(ES_USE_DLL)
    HMODULE hModule = LoadLibraryW(L"zuse.dll");
    if (hModule == NULL)
        return -1;
    typedef int main_func(int argc, char *argv[]);
    main_func *pmain = reinterpret_cast<main_func *>(GetProcAddress(hModule, "es_main"));
    if (pmain)
        pmain(argc, argv);
    FreeLibrary(hModule);
	return 0;
#else
    return es_main(argc, argv);
#endif // ES_USE_DLL
}
