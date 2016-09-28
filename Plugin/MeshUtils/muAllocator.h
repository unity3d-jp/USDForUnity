#pragma once

namespace mu {

void* AlignedMalloc(size_t size, size_t alignment);
void  AlignedFree(void *addr);

} // namespace mu
