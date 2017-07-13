# * USD_INCLUDE_DIR
# * USD_LIBRARY

set(CMAKE_PREFIX_PATH
    "/opt/pixar"
    "/opt/pixar/USD"
)

find_path(USD_INCLUDE_DIR NAMES pxr/pxr.h)
mark_as_advanced(USD_INCLUDE_DIR)

find_library(USD_LIBRARY NAMES usd_ms PATHS ${LIBRARY_PATHS})
mark_as_advanced(USD_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args("USD"
    DEFAULT_MSG
    USD_LIBRARY
    USD_INCLUDE_DIR
)
