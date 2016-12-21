# * USD_ROOT_DIR
# * USD_INCLUDE_DIR
# * USD_LIBRARIES
# * USD_<library>_LIBRARY

if(NOT USD_DIR)
    set(USD_DIR "/opt/pixar")
endif()

set(LIBRARY_PATHS
    ${USD_DIR}/lib
)
set(PLUGIN_PATHS
    ${USD_DIR}/plugin/usd
)


# find libs
find_library(USD_AR_LIBRARY NAMES ar PATHS ${LIBRARY_PATHS})
find_library(USD_ARCH_LIBRARY NAMES arch PATHS ${LIBRARY_PATHS})
find_library(USD_GF_LIBRARY NAMES gf PATHS ${LIBRARY_PATHS})
find_library(USD_JS_LIBRARY NAMES js PATHS ${LIBRARY_PATHS})
find_library(USD_KIND_LIBRARY NAMES kind PATHS ${LIBRARY_PATHS})
find_library(USD_PCP_LIBRARY NAMES pcp PATHS ${LIBRARY_PATHS})
find_library(USD_PLUG_LIBRARY NAMES plug PATHS ${LIBRARY_PATHS})
find_library(USD_SDF_LIBRARY NAMES sdf PATHS ${LIBRARY_PATHS})
find_library(USD_TF_LIBRARY NAMES tf PATHS ${LIBRARY_PATHS})
find_library(USD_TRACELITE_LIBRARY NAMES tracelite PATHS ${LIBRARY_PATHS})
find_library(USD_USD_LIBRARY NAMES usd PATHS ${LIBRARY_PATHS})
find_library(USD_USDGEOM_LIBRARY NAMES usdGeom PATHS ${LIBRARY_PATHS})
find_library(USD_USDHYDRA_LIBRARY NAMES usdHydra PATHS ${LIBRARY_PATHS})
find_library(USD_USDRI_LIBRARY NAMES usdRi PATHS ${LIBRARY_PATHS})
find_library(USD_USDSHADE_LIBRARY NAMES usdShade PATHS ${LIBRARY_PATHS})
find_library(USD_USDUI_LIBRARY NAMES usdUI PATHS ${LIBRARY_PATHS})
find_library(USD_USDUTILS_LIBRARY NAMES usdUtils PATHS ${LIBRARY_PATHS})
find_library(USD_VT_LIBRARY NAMES vt PATHS ${LIBRARY_PATHS})
find_library(USD_WORK_LIBRARY NAMES work PATHS ${LIBRARY_PATHS})

mark_as_advanced(USD_AR_LIBRARY)
mark_as_advanced(USD_ARCH_LIBRARY)
mark_as_advanced(USD_GF_LIBRARY)
mark_as_advanced(USD_JS_LIBRARY)
mark_as_advanced(USD_KIND_LIBRARY)
mark_as_advanced(USD_PCP_LIBRARY)
mark_as_advanced(USD_PLUG_LIBRARY)
mark_as_advanced(USD_SDF_LIBRARY)
mark_as_advanced(USD_TF_LIBRARY)
mark_as_advanced(USD_TRACELITE_LIBRARY)
mark_as_advanced(USD_USD_LIBRARY)
mark_as_advanced(USD_USDGEOM_LIBRARY)
mark_as_advanced(USD_USDHYDRA_LIBRARY)
mark_as_advanced(USD_USDRI_LIBRARY)
mark_as_advanced(USD_USDSHADE_LIBRARY)
mark_as_advanced(USD_USDUI_LIBRARY)
mark_as_advanced(USD_USDUTILS_LIBRARY)
mark_as_advanced(USD_VT_LIBRARY)
mark_as_advanced(USD_WORK_LIBRARY)

set(USD_LIBRARIES
    ${USD_AR_LIBRARY}
    ${USD_ARCH_LIBRARY}
    ${USD_GF_LIBRARY}
    ${USD_JS_LIBRARY}
    ${USD_KIND_LIBRARY}
    ${USD_PCP_LIBRARY}
    ${USD_PLUG_LIBRARY}
    ${USD_SDF_LIBRARY}
    ${USD_TF_LIBRARY}
    ${USD_TRACELITE_LIBRARY}
    ${USD_USD_LIBRARY}
    ${USD_USDGEOM_LIBRARY}
    ${USD_USDHYDRA_LIBRARY}
    ${USD_USDRI_LIBRARY}
    ${USD_USDSHADE_LIBRARY}
    ${USD_USDUI_LIBRARY}
    ${USD_USDUTILS_LIBRARY}
    ${USD_VT_LIBRARY}
    ${USD_WORK_LIBRARY}
)
get_filename_component(USD_LIBRARY_DIR ${USD_USD_LIBRARY} PATH)
mark_as_advanced(USD_LIBRARY_DIR)

# find plugins
find_library(USD_ABC_PLUGIN NAMES usdAbc PATHS ${PLUGIN_PATHS})
mark_as_advanced(USD_ABC_PLUGIN)
#set(USD_PLUGINS ${USD_ABC_PLUGIN})
#get_filename_component(USD_PLUGIN_DIR ${USD_ABC_LIBRARY} PATH)


# Find include dir
find_path(USD_INCLUDE_DIR pxr/usd/usd/prim.h
    PATHS ${USD_DIR}
    PATH_SUFFIXES include
)
mark_as_advanced(USD_INCLUDE_DIR)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args( "USD"
                  DEFAULT_MSG
                  USD_LIBRARIES
                  USD_LIBRARY_DIR
                  USD_INCLUDE_DIR
)

if(NOT USD_FOUND)
    message(WARNING "Try using -D USD_DIR=/path/to/usd")
endif()
