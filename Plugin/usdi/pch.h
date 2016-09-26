#include <vector>
#include <string>
#include <map>
#include <memory>
#include <algorithm>
#include <thread>
#include <mutex>
#include <future>
#include <functional>
#include <atomic>
#include <cstdarg>
#include <cmath>
#include <cctype>

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
#ifdef _WIN32
    #define BOOST_PYTHON_STATIC_LIB
    #define NOMINMAX

    #define BUILD_COMPONENT_SRC_PREFIX "pxr/"
    #define BUILD_OPTLEVEL_OPT
    #define TF_NO_GNU_EXT

    #define USD_ENABLE_CACHED_NEW
#endif

#include "pxr/usd/usd/modelAPI.h"
#include "pxr/usd/usd/timeCode.h"
#include "pxr/usd/usd/treeIterator.h"
#include "pxr/usd/usd/variantSets.h"
#include "pxr/usd/usdGeom/xform.h"
#include "pxr/usd/usdGeom/xformCommonAPI.h"
#include "pxr/usd/usdGeom/camera.h"
#include "pxr/usd/usdGeom/mesh.h"
#include "pxr/usd/usdGeom/points.h"
#include "pxr/base/gf/transform.h"
#include "pxr/base/gf/matrix4f.h"
#include "pxr/usd/ar/resolver.h"

//#define usdiWithVTune
#ifdef usdiWithVTune
    #include "ittnotify.h"
#endif

namespace usdi {
    class Context;
    class Attribute;
    class Schema;
    class Xform;
    class Camera;
    class Mesh;
    class Points;
} // namespace usdi
