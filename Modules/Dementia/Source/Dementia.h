#pragma once

#define ME_DISABLE_DEFAULT_CONSTRUCTOR(Class)	\
Class() = delete;

#define ME_DISABLE_COPY_CONSTRUCTOR(Class)		\
Class(const Class&) = delete;

#define ME_DISABLE_COPY_ASSIGNMENT(Class)		\
Class& operator=(const Class&) = delete;

#define ME_DISABLE_MOVE_CONSTRUCTOR(Class)		\
Class(Class&&) = delete;

#define ME_DISABLE_MOVE_ASSIGNMENT(Class)		\
Class& operator=(Class&&) = delete;

#define ME_NONCOPYABLE(Class) ME_DISABLE_COPY_CONSTRUCTOR(Class); ME_DISABLE_COPY_ASSIGNMENT(Class);

#define ME_NONMOVABLE(Class) ME_DISABLE_MOVE_CONSTRUCTOR(Class); ME_DISABLE_MOVE_ASSIGNMENT(Class);

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifdef ME_PLATFORM_WIN64
#define ME_PLATFORM_WIN64 1
#else
#define ME_PLATFORM_WIN64 0
#endif

#ifdef ME_PLATFORM_UWP
#define ME_PLATFORM_UWP 1
#else
#define ME_PLATFORM_UWP 0
#endif

#ifdef ME_DIRECTX
#define ME_DIRECTX 1
#else
#define ME_DIRECTX 0
#endif

#ifdef ME_OPENGL
#define ME_OPENGL 1
#else
#define ME_OPENGL 0
#endif

#ifdef ME_ENABLE_RENDERDOC
#define ME_ENABLE_RENDERDOC 1
#else
#define ME_ENABLE_RENDERDOC 1
#endif
