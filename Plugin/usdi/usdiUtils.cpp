#include "pch.h"
#include "usdiInternal.h"
#include "usdiUtils.h"

namespace usdi {


TempBuffer& GetTemporaryBuffer()
{
    static thread_local TempBuffer s_buf;
    return s_buf;
}


} // namespace usdi
