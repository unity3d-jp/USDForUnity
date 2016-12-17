CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(usdi)

SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
FIND_PACKAGE(PythonLibs 2.7 QUIET)
FIND_PACKAGE(Boost COMPONENTS python system filesystem QUIET)
FIND_PACKAGE(TBB QUIET)
FIND_PACKAGE(OpenEXR QUIET)
FIND_PACKAGE(DoubleConversion QUIET)
FIND_PACKAGE(GLEW QUIET)

SET(USD_ROOT "/opt/pixar" CACHE PATH "path to USD root")
SET(ISPC "/usr/local/bin/ispc" CACHE PATH "path to Intel ISPC")

OPTION(USDI_BUILD_TESTS "" ON)
OPTION(USDI_ENABLE_DEBUG "" OFF)
OPTION(USDI_ENABLE_ISPC "" OFF)
OPTION(USDI_ENABLE_UNITY_EXTENSION "" ON)
OPTION(USDI_ENABLE_GRAPHICS_INTERFACE "" OFF)
OPTION(USDI_ENABLE_MONO_BINDING "" OFF)



ADD_DEFINITIONS(-DBUILD_OPTLEVEL_OPT -DBUILD_COMPONENT_SRC_PREFIX="pxr/")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11 -fPIC -Wno-deprecated -DTF_NO_GNU_EXT")

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
    SET(MUCORE_DIR ${CMAKE_CURRENT_BINARY_DIR}/MeshUtilsCore)
    ADD_CUSTOM_TARGET(MeshUtilsCore ALL
        COMMAND ${ISPC} ${CMAKE_CURRENT_SOURCE_DIR}/MeshUtils/MeshUtilsCore.ispc -o ${MUCORE_DIR}/MeshUtilsCore${CMAKE_CXX_OUTPUT_EXTENSION} -h ${MUCORE_DIR}/MeshUtilsCore.h --pic --target=sse2,sse4,avx --arch=x86-64 --opt=fast-masked-vload --opt=fast-math --opt=force-aligned-memory
    )
    SET(MUCORE_FILES
        ${MUCORE_DIR}/MeshUtilsCore.h
        ${MUCORE_DIR}/MeshUtilsCore${CMAKE_CXX_OUTPUT_EXTENSION}
        ${MUCORE_DIR}/MeshUtilsCore_sse2${CMAKE_CXX_OUTPUT_EXTENSION}
        ${MUCORE_DIR}/MeshUtilsCore_sse4${CMAKE_CXX_OUTPUT_EXTENSION}
        ${MUCORE_DIR}/MeshUtilsCore_avx${CMAKE_CXX_OUTPUT_EXTENSION}
    )

    EXECUTE_PROCESS(COMMAND mkdir -p ${MUCORE_DIR})
    FOREACH(F ${MUCORE_FILES})
        EXECUTE_PROCESS(COMMAND touch ${F})
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


# usdi
FILE(GLOB USDI_CXX_FILES usdi/*.cpp usdi/etc/*.cpp usdi/ext/*.cpp)
FILE(GLOB USDI_H_FILES usdi/*.h usdi/etc/*.h usdi/ext/*.h)
ADD_LIBRARY(usdi SHARED ${USDI_CXX_FILES} ${USDI_H_FILES})
ADD_DEPENDENCIES(usdi MeshUtils)
TARGET_INCLUDE_DIRECTORIES(usdi PUBLIC
    ./usdi
)
TARGET_LINK_LIBRARIES(usdi
    ar arch tf gf pcp vt sdf usd usdGeom usdUtils MeshUtils
    ${PYTHON_LIBRARY}
    ${Boost_PYTHON_LIBRARY_RELEASE}
    ${Boost_SYSTEM_LIBRARY_RELEASE}
    ${Boost_FILESYSTEM_LIBRARY_RELEASE}
    ${TBB_tbb_LIBRARY_RELEASE}
    ${OPENEXR_Half_LIBRARY}
    ${DOUBLE_CONVERSION_LIBRARY}
)
ADD_DEFINITIONS(-DusdiEnableBoostFilesystem)
ADD_DEFINITIONS(-DusdiEnableUnityExtension)
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
    TARGET_LINK_LIBRARIES(usdi GraphicsInterface GL ${GLEW_LIBRARY})
ENDIF()


# usdiRT
FILE(GLOB USDIRT_CXX_FILES usdiRT/*.cpp)
FILE(GLOB USDIRT_H_FILES usdiRT/*.h)
ADD_LIBRARY(usdiRT SHARED ${USDIRT_CXX_FILES} ${USDIRT_H_FILES})
TARGET_INCLUDE_DIRECTORIES(usdiRT PUBLIC
    ./usdiRT
)


# testsCMAKE_CURRENT_BINARY_DIR
IF(USDI_BUILD_TESTS)
    FILE(GLOB Tests_CXX_FILES Test/*.cpp)
    FILE(GLOB Tests_H_FILES Test/*.h)
    ADD_EXECUTABLE(usdiTests ${Tests_CXX_FILES} ${Tests_H_FILES})
    ADD_DEPENDENCIES(usdiTests usdi usdiRT MeshUtils)
    TARGET_LINK_LIBRARIES(usdiTests usdi usdiRT MeshUtils)
ENDIF()


# deploy
SET(ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../USDForUnity/Assets")
SET(PLUGINS_DIR "${ASSETS_DIR}/UTJ/Plugins/x86_64")
ADD_CUSTOM_TARGET(deploy ALL
    COMMAND cp $<TARGET_FILE:usdi> ${PLUGINS_DIR}
    COMMAND cp $<TARGET_FILE:usdiRT> ${PLUGINS_DIR}
    DEPENDS usdi
    DEPENDS usdiRT
)
