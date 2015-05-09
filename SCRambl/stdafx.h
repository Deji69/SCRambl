/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#ifdef _MSC_VER
	#pragma warning(disable:4341)
#endif

#ifdef _WIN32
	#include <Windows.h>
#else
	#define DWORD unsigned long
#endif

#include <stdio.h>
#include <string>
#include <algorithm>
#include <limits>
#include <memory>
#include <sstream>
#include <map>
#include <unordered_map>
#include <exception>
#include <iostream>
#include <vector>
#include <list>
#include <assert.h>
#include <functional>
#include <typeinfo>
#include <typeindex>
#include <cctype>
#include <stack>
#include <queue>