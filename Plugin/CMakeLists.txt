CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(usdi)

SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
FIND_PACKAGE(PythonLibs 2.7 QUIET)
FIND_PACKAGE(Boost COMPONENTS python system filesystem QUIET)
FIND_PACKAGE(TBB QUIET)
FIND_PACKAGE(OpenEXR QUIET)
FIND_PACKAGE(DoubleConversion QUIET)
FIND_PACKAGE(OpenGL QUIET)
FIND_PACKAGE(GLEW QUIET)

SET(USD_ROOT "/opt/pixar" CACHE PATH "Path to USD root")
SET(ISPC "/usr/local/bin/ispc" CACHE PATH "Path to Intel ISPC")
MARK_AS_ADVANCED(FORCE ISPC)

OPTION(USDI_ENABLE_UNITY_EXTENSION "Generate Unity extensions. This is must to use as Unity plugin." ON)
OPTION(USDI_ENABLE_ISPC "Use Intel ISPC to generate SIMDified code. It can significantly boost performance." ON)
OPTION(USDI_ENABLE_GRAPHICS_INTERFACE "Use OpenGL or Direct3D to update vertex buffers directry. It can significantly boost performance." ON)
OPTION(USDI_ENABLE_MONO_BINDING "Generate mono bindings to access C# objects from C++ directly. It can boost performance." ON)
OPTION(USDI_ENABLE_DEBUG "Enable debug functionalities." OFF)
OPTION(USDI_ENABLE_TESTS "Build tests." ON)



ADD_DEFINITIONS(-DBUILD_OPTLEVEL_OPT -DBUILD_COMPONENT_SRC_PREFIX="pxr/")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -std=c++11 -Wno-deprecated -DTF_NO_GNU_EXT")

IF(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    SET(USD_PLUGINS_DIR_NAME "plugins_linux")
ELSEIF(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    SET(USD_PLUGINS_DIR_NAME "plugins_mac")
    SET(CMAKE_MACOSX_RPATH ON)
ELSEIF(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    SET(USD_PLUGINS_DIR_NAME "plugins_win64")
ENDIF()
SET(ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../USDForUnity/Assets")
SET(PLUGINS_DIR "${ASSETS_DIR}/UTJ/Plugins/x86_64")
SET(USD_PLUGINS_DIR "${ASSETS_DIR}/StreamingAssets/UTJ/USDForUnity/${USD_PLUGINS_DIR_NAME}")


INCLUDE_DIRECTORIES(
    .
    ${INCLUDES}
    ${PYTHON_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    ${TBB_INCLUDE_DIRS}
    ${OPENEXR_INCLUDE_DIRS}
    ${DOUBLE_CONVERSION_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIR}
    ${USD_ROOT}/include
)

LINK_DIRECTORIES (
    ${USD_ROOT}/lib
)


# GraphicsInterface
IF(USDI_ENABLE_GRAPHICS_INTERFACE)
    FILE(GLOB GI_CXX_FILES GraphicsInterface/*.cpp)
    FILE(GLOB GI_H_FILES GraphicsInterface/*.h)
    ADD_LIBRARY(GraphicsInterface STATIC ${GI_CXX_FILES} ${GI_H_FILES})
    TARGET_INCLUDE_DIRECTORIES(GraphicsInterface PUBLIC
        ./GraphicsInterface
        ./External/Unity/include
    )
ENDIF()


# MeshUtils
IF(USDI_ENABLE_ISPC)
    if(NOT EXISTS ${ISPC})
        # try to download ISPC
        SET(ISPC_VERSION 1.9.1)
        IF(CMAKE_SYSTEM_NAME STREQUAL "Linux")
            SET(ISPC_DIR ispc-v${ISPC_VERSION}-linux)
        ELSEIF(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
            SET(ISPC_DIR ispc-v${ISPC_VERSION}-osx)
        ENDIF()
        SET(ISPC ${CMAKE_CURRENT_SOURCE_DIR}/External/${ISPC_DIR}/ispc)

        SET(ISPC_ARCHIVE ${ISPC_DIR}.tar.gz)
        FILE(DOWNLOAD http://downloads.sourceforge.net/project/ispcmirror/v${ISPC_VERSION}/${ISPC_ARCHIVE} ${CMAKE_CURRENT_BINARY_DIR}/${ISPC_ARCHIVE} SHOW_PROGRESS)
        EXECUTE_PROCESS(
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/External
            COMMAND tar -xzvf ${CMAKE_CURRENT_BINARY_DIR}/${ISPC_ARCHIVE}
        )
    ENDIF()

    SET(MUCORE_DIR ${CMAKE_CURRENT_BINARY_DIR}/MeshUtilsCore)
    ADD_CUSTOM_TARGET(MeshUtilsCore ALL
        # not use --opt=force-aligned-memory as I can't force USD to align memory to 0x20.
        # (Windows port is using with patched USD)
        COMMAND ${ISPC} ${CMAKE_CURRENT_SOURCE_DIR}/MeshUtils/MeshUtilsCore.ispc -o ${MUCORE_DIR}/MeshUtilsCore${CMAKE_CXX_OUTPUT_EXTENSION} -h ${MUCORE_DIR}/MeshUtilsCore.h --pic --target=sse2,sse4,avx --arch=x86-64 --opt=fast-masked-vload --opt=fast-math
    )
    SET(MUCORE_FILES
        ${MUCORE_DIR}/MeshUtilsCore.h
        ${MUCORE_DIR}/MeshUtilsCore${CMAKE_CXX_OUTPUT_EXTENSION}
        ${MUCORE_DIR}/MeshUtilsCore_sse2${CMAKE_CXX_OUTPUT_EXTENSION}
        ${MUCORE_DIR}/MeshUtilsCore_sse4${CMAKE_CXX_OUTPUT_EXTENSION}
        ${MUCORE_DIR}/MeshUtilsCore_avx${CMAKE_CXX_OUTPUT_EXTENSION}
    )

    # create dummy files to make cmake can find it
    FOREACH(F ${MUCORE_FILES})
        FILE(WRITE ${F} "")
    ENDFOREACH(F)
ENDIF()

FILE(GLOB MU_CXX_FILES MeshUtils/*.cpp MeshUtils/*.c)
FILE(GLOB MU_H_FILES MeshUtils/*.h)
ADD_LIBRARY(MeshUtils STATIC ${MU_CXX_FILES} ${MU_H_FILES} ${MUCORE_FILES})
TARGET_INCLUDE_DIRECTORIES(MeshUtils PUBLIC ./MeshUtils)
IF(USDI_ENABLE_ISPC)
    ADD_DEFINITIONS(-DmuEnableISPC)
    ADD_DEPENDENCIES(MeshUtils MeshUtilsCore)
    TARGET_INCLUDE_DIRECTORIES(MeshUtils PUBLIC ${MUCORE_DIR})
ENDIF()


# usdiRT
FILE(GLOB RT_CXX_FILES usdiRT/*.cpp)
FILE(GLOB RT_H_FILES usdiRT/*.h)
ADD_LIBRARY(usdiRT SHARED ${RT_CXX_FILES} ${RT_H_FILES})
TARGET_INCLUDE_DIRECTORIES(usdiRT PUBLIC
    ./usdiRT
)
IF(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    TARGET_LINK_LIBRARIES(usdiRT dl)
ENDIF()

# usdi
FILE(GLOB USDI_CXX_FILES usdi/*.cpp usdi/etc/*.cpp usdi/ext/*.cpp)
FILE(GLOB USDI_H_FILES usdi/*.h usdi/etc/*.h usdi/ext/*.h)
ADD_LIBRARY(usdi SHARED ${USDI_CXX_FILES} ${USDI_H_FILES})
ADD_DEPENDENCIES(usdi usdiRT MeshUtils)
TARGET_INCLUDE_DIRECTORIES(usdi PUBLIC
    ./usdi
)
TARGET_LINK_LIBRARIES(usdi
    ar arch tf gf pcp vt sdf usd usdGeom usdUtils usdiRT MeshUtils
    ${PYTHON_LIBRARY}
    ${Boost_PYTHON_LIBRARY_RELEASE}
    ${Boost_SYSTEM_LIBRARY_RELEASE}
    ${Boost_FILESYSTEM_LIBRARY_RELEASE}
    ${TBB_tbb_LIBRARY_RELEASE}
    ${OPENEXR_Half_LIBRARY}
    ${DOUBLE_CONVERSION_LIBRARY}
)

ADD_DEFINITIONS(-DusdiEnableBoostFilesystem)
IF(USDI_ENABLE_UNITY_EXTENSION)
    ADD_DEFINITIONS(-DusdiEnableUnityExtension)
ENDIF()
IF(USDI_ENABLE_DEBUG)
    ADD_DEFINITIONS(-DusdiDebug)
ENDIF()
IF(USDI_ENABLE_MONO_BINDING)
    ADD_DEFINITIONS(-DusdiEnableMonoBinding)
ENDIF()
IF(USDI_ENABLE_GRAPHICS_INTERFACE)
    ADD_DEFINITIONS(-DusdiEnableGraphicsInterface)
    ADD_DEPENDENCIES(usdi GraphicsInterface)
    TARGET_LINK_LIBRARIES(usdi
        GraphicsInterface
        ${OPENGL_gl_LIBRARY}
        ${GLEW_LIBRARY}
    )
ENDIF()


# tests
IF(USDI_ENABLE_TESTS)
    FILE(GLOB Tests_CXX_FILES Test/*.cpp)
    FILE(GLOB Tests_H_FILES Test/*.h)
    FILE(GLOB TestsMain_CXX_FILE Test/usdiTestsMain.cpp)
    LIST(REMOVE_ITEM Tests_CXX_FILES ${TestsMain_CXX_FILE})

    ADD_EXECUTABLE(usdiTests ${Tests_CXX_FILES} ${Tests_H_FILES})
    ADD_DEPENDENCIES(usdiTests usdi usdiRT MeshUtils)
    TARGET_LINK_LIBRARIES(usdiTests usdi usdiRT MeshUtils)

    ADD_EXECUTABLE(usdiTestsMain ${TestsMain_CXX_FILE})
    ADD_DEPENDENCIES(usdiTestsMain usdiTests)
    TARGET_LINK_LIBRARIES(usdiTestsMain
        usdiRT MeshUtils
        ${Boost_SYSTEM_LIBRARY_RELEASE}
        ${Boost_FILESYSTEM_LIBRARY_RELEASE}
    )
ENDIF()


# deploy
IF(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    # make bundle
ELSE()
    ADD_CUSTOM_TARGET(deploy ALL
        COMMAND cp $<TARGET_FILE:usdi> ${PLUGINS_DIR}
        COMMAND cp $<TARGET_FILE:usdiRT> ${PLUGINS_DIR}
        DEPENDS usdi
        DEPENDS usdiRT
    )
ENDIF()
