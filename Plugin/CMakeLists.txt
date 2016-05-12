CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(USDPlugins)

OPTION (BUILD_SHARED_LIBS "Build Shared Libraries" ON)
OPTION (USE_EXTERNALS "Use Externals (Add externals/alembic etc to include & library path)" ON)

IF(MSVC)
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MT")
ELSE()
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g --std=c++11 -Wno-deprecated")
ENDIF()

SET( INCLUDES
    .
)
IF(USE_EXTERNALS)
    SET (INCLUDES
        ${INCLUDES}
        /opt/pixar/usd/include
        /opt/pixar/usd-vfx2015/include
        /opt/pixar/usd-vfx2015/include/python2.7
        /opt/pixar/usd-vfx2015/packages/openexr-2.2.0/include/OpenEXR
    )
ENDIF()
INCLUDE_DIRECTORIES(${INCLUDES})

IF(USE_EXTERNALS)
    LINK_DIRECTORIES (
        /opt/pixar/usd/lib
        /opt/pixar/usd-vfx2015/lib
    )
ENDIF()

ADD_DEFINITIONS(-DBUILD_OPTLEVEL_OPT -DBUILD_COMPONENT_SRC_PREFIX="USDPlugins")

FILE(GLOB USDI_CXX_FILES usdi/*.cpp)
FILE(GLOB USDI_H_FILES usdi/*.h)


SET( USDI_SOURCE_FILES
    ${USDI_CXX_FILES} ${USDI_H_FILES}
)
ADD_LIBRARY( usdi ${USDI_SOURCE_FILES} )


LINK_DIRECTORIES(/opt/pixar/usd/lib /opt/pixar/usd-vfx2015)
TARGET_LINK_LIBRARIES(usdi usd usdGeom usdUtils boost_python-mt)
