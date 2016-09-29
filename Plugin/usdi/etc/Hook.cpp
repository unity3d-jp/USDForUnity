#include "pch.h"
#include "Hook.h"

namespace usdi {

#ifdef _WIN32

void ForceWrite(void *dst, const void *src, size_t s)
{
    DWORD old_flag;
    VirtualProtect(dst, s, PAGE_EXECUTE_READWRITE, &old_flag);
    memcpy(dst, src, s);
    VirtualProtect(dst, s, old_flag, &old_flag);
}

void* EmitJumpInstruction(void* from_, void* to_)
{
    BYTE *base, *from, *to;
    base = from = (BYTE*)from_;
    to = (BYTE*)to_;

    BYTE* jump_from = from + 5;
    size_t distance = jump_from > to ? jump_from - to : to - jump_from;

    // emit 5 byte jmp (0xe9 RVA) if relative address is less than 32 bit.
    // otherwise emit 14 byte long jmp (0xff 0x25 [memory] + target)
    if (distance <= 0x7fff0000) {
        from[0] = 0xe9;
        from += 1;
        *((DWORD*)from) = (DWORD)(to - jump_from);
        from += 4;
    }
    else {
        from[0] = 0xff;
        from[1] = 0x25;
        from += 2;
#ifdef _M_IX86
        *((DWORD*)from) = (DWORD)(from + 4);
#elif defined(_M_X64)
        *((DWORD*)from) = (DWORD)0;
#endif
        from += 4;
        *((DWORD_PTR*)from) = (DWORD_PTR)(to);
        from += 8;
    }
    ::FlushInstructionCache(nullptr, base, size_t(from - base));
    return from;
}

void* OverrideDLLImport(HMODULE module, const char *target_module, const char *target_funcname, void *replacement)
{
    if (!module) { return nullptr; }

    size_t ImageBase = (size_t)module;
    PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)ImageBase;
    PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS)(ImageBase + pDosHeader->e_lfanew);

    size_t RVAImports = pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
    IMAGE_IMPORT_DESCRIPTOR *pImportDesc = (IMAGE_IMPORT_DESCRIPTOR*)(ImageBase + RVAImports);
    while (pImportDesc->Name != 0) {
        if (stricmp((const char*)(ImageBase + pImportDesc->Name), target_module) == 0) {
            const char *dllname = (const char*)(ImageBase + pImportDesc->Name);
            IMAGE_IMPORT_BY_NAME **func_names = (IMAGE_IMPORT_BY_NAME**)(ImageBase + pImportDesc->Characteristics);
            void **import_table = (void**)(ImageBase + pImportDesc->FirstThunk);
            for (size_t i = 0; ; ++i) {
                if ((size_t)func_names[i] == 0) { break; }
                const char *funcname = (const char*)(ImageBase + (size_t)func_names[i]->Name);
                if (strcmp(funcname, target_funcname) == 0) {
                    void *before = import_table[i];
                    ForceWrite<void*>(import_table[i], replacement);
                    return before;
                }
            }
        }
        ++pImportDesc;
    }
    return nullptr;
}

void* OverrideDLLExportByName(HMODULE module, const char *funcname, void *replacement)
{
    if (!module) { return nullptr; }

    size_t ImageBase = (size_t)module;
    PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)ImageBase;
    if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) { return nullptr; }

    PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS)(ImageBase + pDosHeader->e_lfanew);
    DWORD RVAExports = pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
    if (RVAExports == 0) { return nullptr; }

    IMAGE_EXPORT_DIRECTORY *pExportDirectory = (IMAGE_EXPORT_DIRECTORY *)(ImageBase + RVAExports);
    DWORD *RVANames = (DWORD*)(ImageBase + pExportDirectory->AddressOfNames);
    WORD *RVANameOrdinals = (WORD*)(ImageBase + pExportDirectory->AddressOfNameOrdinals);
    DWORD *RVAFunctions = (DWORD*)(ImageBase + pExportDirectory->AddressOfFunctions);
    for (size_t i = 0; i < pExportDirectory->NumberOfFunctions; ++i) {
        char *pName = (char*)(ImageBase + RVANames[i]);
        if (strcmp(pName, funcname) == 0) {
            size_t ret = (size_t)module + RVAFunctions[RVANameOrdinals[i]];
            ForceWrite<DWORD>(RVAFunctions[RVANameOrdinals[i]], (DWORD)((size_t)replacement - ImageBase));
            return (void*)ret;
        }
    }
    return nullptr;
}

#endif

} // namespace usdi
