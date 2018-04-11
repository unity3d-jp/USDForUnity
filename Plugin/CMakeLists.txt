cmake_minimum_required(VERSION 2.8)
project(usdi)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
find_package(PythonLibs 2.7 QUIET)
find_package(Boost COMPONENTS python system filesystem QUIET)
find_package(TBB QUIET)
find_package(OpenEXR QUIET)
find_package(USD QUIET)
include(ISPC)
include(AddPlugin)

option(USDI_ENABLE_DEBUG "Enable debug functionalities." ON)
option(USDI_ENABLE_TESTS "Build tests." ON)


add_definitions(-DBUILD_OPTLEVEL_OPT -DBUILD_COMPONENT_SRC_PREFIX="pxr/")
add_definitions(-DmuEnableHalf)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -std=c++11 -Wno-deprecated -DTF_NO_GNU_EXT")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")



include_directories(
    .
    ${CMAKE_SOURCE_DIR}
    ${PYTHON_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    ${TBB_INCLUDE_DIRS}
    ${OPENEXR_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIR}
    ${USD_INCLUDE_DIR}
)

add_subdirectory(usdiRT)
add_subdirectory(usdi)
add_subdirectory(Test)
