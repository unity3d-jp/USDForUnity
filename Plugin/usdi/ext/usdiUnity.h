#pragma once

namespace usdi {

void ForceAssignTRS(void *monoobj, const XformData& data);
void ForceAssignBounds(void *monoobj, const float3& center, const float3& extents);

void AddMonoFunctions();

} // namespace usdi
