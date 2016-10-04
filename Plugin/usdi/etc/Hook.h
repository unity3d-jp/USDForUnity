#pragma once

#ifdef _WIN32
    #include <windows.h>
#endif


#ifdef _WIN32

enum class MemoryFlags
{
    ExecuteRead,
    ReadWrite,
    ExecuteReadWrite,
};

int SetMemoryFlags(void *addr, size_t size, MemoryFlags flags);

void ForceWrite(void *dst, const void *src, size_t s);
template<class T> inline void ForceWrite(T &dst, const T &src);
template<class Body> inline void ForceWrite(void *dst, size_t size, const Body& body);



// emit jmp from "from" to "to" instructions.
// instructions will be 5 byte if relative address is < 32 bit, oterwise 14 byte.
// return next address of last written byte (from+5 or from+14).
void* EmitJumpInstruction(void* from, const void* to);

void* OverrideDLLImport(void *module, const char *target_module, const char *target_funcname, void *replacement);
void* OverrideDLLExportByName(void *module, const char *funcname, void *replacement);


void* FindSymbolByName(const char *name);


// impl

template<class T>
inline void ForceWrite(T &dst, const T &src)
{
    ForceWrite(&dst, &src, sizeof(T));
}

template<class Body>
inline void ForceWrite(void *dst, size_t size, const Body& body)
{
    int f = SetMemoryFlags(dst, size, MemoryFlags::ExecuteReadWrite);
    body();
    SetMemoryFlags(dst, size, (MemoryFlags)f);
}

#endif
