/** @file version.h

	HITACHI MB-S1 Emulator 'EmuB-S1'
	Skelton for retropc emulator

	@author Sasaji
	@date   2014.12.08

	@brief [ version info ]
*/

#ifndef VERSION_H
#define VERSION_H

#define APP_NAME		"HITACHI MB-S1 model05 Emulator 'EmuB-S1'"
#define APP_FILENAME	"mbs1.exe"
#define APP_INTERNAME	"EmuB-S1"
#define APP_COPYRIGHT	"Copyright (C) 2011,2012-2023 Common Source Code Project, Sasaji"
#define APP_VERSION		"0.7.3.1118"
#define APP_VER_MAJOR	0
#define APP_VER_MINOR	7
#define APP_VER_REV		3
#define APP_VER_BUILD	1118

#if defined(__MINGW32__)
#if defined(x86_64) || defined(__x86_64)
#define PLATFORM "Windows(MinGW) 64bit"
#elif defined(i386) || defined(__i386)
#define PLATFORM "Windows(MinGW) 32bit"
#else
#define PLATFORM "Windows(MinGW)"
#endif
#elif defined(_WIN32)
#if defined(_WIN64) || defined(_M_X64)
#define PLATFORM "Windows 64bit"
#else
#define PLATFORM "Windows 32bit"
#endif
#elif defined(linux)
#ifdef __x86_64
#define PLATFORM "Linux 64bit"
#elif __i386
#define PLATFORM "Linux 32bit"
#else
#define PLATFORM "Linux"
#endif
#elif defined(__APPLE__) && defined(__MACH__)
#ifdef __x86_64
#define PLATFORM "MacOSX 64bit"
#elif __i386
#define PLATFORM "MacOSX 32bit"
#else
#define PLATFORM "MacOSX"
#endif
#elif defined(__FreeBSD__)
#ifdef __x86_64
#define PLATFORM "FreeBSD 64bit"
#elif __i386
#define PLATFORM "FreeBSD 32bit"
#else
#define PLATFORM "FreeBSD"
#endif
#else
#define PLATFORM "Unknown"
#endif

#endif /* VERSION_H */
