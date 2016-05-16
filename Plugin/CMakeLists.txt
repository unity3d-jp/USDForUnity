CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(usdi)

SET(CMAKE_BUILD_TYPE Release)

SET(USD_ROOT /opt/pixar/usd)
SET(USD_VFX_ROOT /opt/pixar/usd-vfx2015)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g --std=c++11 -Wno-deprecated")
SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-rpath,${USD_ROOT}/lib:${USD_VFX_ROOT}/lib")

SET(INCLUDES
    ${INCLUDES}
    .
    ${USD_ROOT}/include
    ${USD_VFX_ROOT}/include
    ${USD_VFX_ROOT}/include/python2.7
    ${USD_VFX_ROOT}/packages/openexr-2.2.0/include/OpenEXR
)
INCLUDE_DIRECTORIES(${INCLUDES})

LINK_DIRECTORIES (
    ${USD_ROOT}/lib
    ${USD_VFX_ROOT}/lib
)

ADD_DEFINITIONS(-DBUILD_OPTLEVEL_OPT -DBUILD_COMPONENT_SRC_PREFIX="USDPlugins")


FILE(GLOB USDI_CXX_FILES usdi/*.cpp)
FILE(GLOB USDI_H_FILES usdi/*.h)
SET(USDI_SOURCE_FILES ${USDI_CXX_FILES} ${USDI_H_FILES})
ADD_LIBRARY(usdi SHARED ${USDI_SOURCE_FILES})
TARGET_LINK_LIBRARIES(usdi usd usdGeom usdUtils boost_python-mt)


ADD_EXECUTABLE(test_import Test/usdiTestImport.cpp)
ADD_DEPENDENCIES(test_import usdi)
TARGET_LINK_LIBRARIES(test_import usdi)


ADD_EXECUTABLE(test_export Test/usdiTestExport.cpp)
ADD_DEPENDENCIES(test_export usdi)
TARGET_LINK_LIBRARIES(test_export usdi)
