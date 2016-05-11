#include "pch.h"
#include "usdiInternal.h"
#include "usdiContext.h"



usdi::Context* usdiOpen(const char *path)
{
    auto* ret = new usdi::Context();
    if (!ret->open(path)) {
        delete ret;
        return nullptr;
    }

    return ret;
}
