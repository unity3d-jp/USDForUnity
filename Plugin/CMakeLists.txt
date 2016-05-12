CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(USDPlugins)

OPTION (BUILD_SHARED_LIBS "Build Shared Libraries" ON)
OPTION (USE_EXTERNALS "Use Externals (Add externals/alembic etc to include & library path)" ON)

IF(MSVC)
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MT")
ELSE()
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11")
ENDIF()

SET( INCLUDES
    .
    Foundation
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

FILE(GLOB EXPORTER_CXX_FILES USDExporter/*.cpp)
FILE(GLOB EXPORTER_H_FILES USDExporter/*.h)

FILE(GLOB IMPORTER_CXX_FILES USDImporter/*.cpp)
FILE(GLOB IMPORTER_H_FILES USDImporter/*.h)


SET( EXPORTER_SOURCE_FILES
    ${EXPORTER_CXX_FILES} ${EXPORTER_H_FILES}
)
ADD_LIBRARY( USDExporter ${EXPORTER_SOURCE_FILES} )

SET( IMPORTER_SOURCE_FILES
    ${IMPORTER_CXX_FILES} ${IMPORTER_H_FILES}
)
ADD_LIBRARY( USDImporter ${IMPORTER_SOURCE_FILES} )


LINK_DIRECTORIES(/opt/pixar/usd/lib /opt/pixar/usd-vfx2015)
TARGET_LINK_LIBRARIES(USDImporter usd usdGeom usdUtils boost_python-mt)
