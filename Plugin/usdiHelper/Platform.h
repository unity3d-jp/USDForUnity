#pragma once

#if defined(_WIN32) && (_M_X64)
    #define __Windows_x86_64__
#elif defined(__linux__ ) && defined(__x86_64__)
    #define __Linux_x86_64__
#elif defined(__MACH__) && defined(__x86_64__)
    #define __Mac_x86_64__
#elif defined(__ANDROID__) && defined(__arm64__)
    #define __Android_ARM64__
#elif defined(__APPLE__) && defined(__arm64__)
    #define __iOS_ARM64__
// todo: PS4
#endif
