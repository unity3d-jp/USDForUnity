if(ENABLE_ISPC)
    setup_ispc()

    set(ISPC_OUTDIR ${CMAKE_CURRENT_BINARY_DIR}/ISPC)
    set(ISPC_HEADERS
        "${CMAKE_CURRENT_SOURCE_DIR}/Foundation/ispcmath.h"
    )
    file(GLOB ISPC_SOURCES Foundation/*.ispc)
    add_ispc_targets(SOURCES ${ISPC_SOURCES} HEADERS ${ISPC_HEADERS} OUTDIR ${ISPC_OUTDIR})
    set(ISPC_OUTPUTS ${_ispc_outputs})
endif()

file(GLOB sources *.cpp *.h etc/*.cpp etc/*.h ext/*.cpp ext/*.h Foundation/*.cpp Foundation/*.h ${CMAKE_SOURCE_DIR}/usdiRT/*.cpp ${ISPC_OUTPUTS})
set(plugins_dir "${CMAKE_SOURCE_DIR}/../USDForUnity/Assets/UTJ/USDForUnity/Plugins/x86_64")
add_plugin(usdi SOURCES ${sources} PLUGINS_DIR ${plugins_dir})

# doto: make this selectable(boost::filesystem or std::filesystem)
#add_definitions(-DusdiEnableBoostFilesystem)

target_include_directories(usdi PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(usdi
    ${USD_LIBRARY}
    ${PYTHON_LIBRARY}
    ${Boost_PYTHON_LIBRARY_RELEASE}
    ${Boost_FILESYSTEM_LIBRARY_RELEASE}
    ${TBB_LIBRARIES}
    ${OPENEXR_Half_LIBRARY}
    -lstdc++fs
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(usdi -ldl -lpthread)
endif()

if(USDI_ENABLE_UNITY_EXTENSION)
    add_definitions(-DusdiEnableUnityExtension)
endif()
if(USDI_ENABLE_DEBUG)
    add_definitions(-DusdiDebug)
endif()
if(USDI_ENABLE_MONO_BINDING)
    add_definitions(-DusdiEnableMonoBinding)
endif()
if(USDI_ENABLE_GRAPHICS_INTERFACE)
    add_definitions(-DusdiEnableGraphicsInterface)
    add_dependencies(usdi GraphicsInterface)
    target_link_libraries(usdi
        GraphicsInterface
        ${OPENGL_gl_LIBRARY}
        ${GLEW_LIBRARY}
    )
endif()

install(TARGETS usdi DESTINATION .)
