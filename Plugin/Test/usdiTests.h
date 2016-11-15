#pragma once

#ifdef _WIN32
    #ifdef testsImpl
        #define testsAPI __declspec(dllexport)
    #else
        #define testsAPI __declspec(dllimport)
    #endif
#else
    #define testsAPI
#endif

extern "C" {
testsAPI void RunTests();
} // extern "C"
