#pragma once

#ifdef _WIN32
    #include <windows.h>
#endif


namespace usdi {

#ifdef _WIN32

void ForceWrite(void *dst, const void *src, size_t s);
template<class T>
inline void ForceWrite(T &dst, const T &src) { ForceWrite(&dst, &src, sizeof(T)); }

void* EmitJumpInstruction(void* from, void* to);
void* OverrideDLLImport(HMODULE module, const char *target_module, const char *target_funcname, void *replacement);
void* OverrideDLLExportByName(HMODULE module, const char *funcname, void *replacement);
#endif

} // namespace usdi

