#include <vector>
#include <string>
#include <map>
#include <memory>
#include <algorithm>
#include <thread>
#include <mutex>
#include <functional>
#include <atomic>
#include <cstdarg>

#ifdef _MSC_VER
    #define and     &&
    #define and_eq  &=
    #define bitand  &
    #define bitor   |
    #define compl   ~
    #define not     !
    #define not_eq  !=
    #define or      ||
    #define or_eq   |=
    #define xor     ^
    #define xor_eq  ^=
#endif
#include "pxr/usd/usd/modelAPI.h"
#include "pxr/usd/usd/timeCode.h"
#include "pxr/usd/usd/treeIterator.h"
#include "pxr/usd/usd/variantSets.h"
#include "pxr/usd/usdGeom/xform.h"
#include "pxr/usd/usdGeom/xformCommonAPI.h"
#include "pxr/usd/usdGeom/mesh.h"
#include "pxr/base/gf/transform.h"
#include "pxr/usd/ar/resolver.h"
