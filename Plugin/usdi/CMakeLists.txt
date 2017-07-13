file(GLOB sources *.cpp etc/*.cpp ext/*.cpp ${CMAKE_SOURCE_DIR}/usdiRT/*.cpp *.h etc/*.h ext/*.h)
set(plugins_dir "${CMAKE_SOURCE_DIR}/../USDForUnity/Assets/UTJ/USDForUnity/Plugins/x86_64")
add_plugin(usdi SOURCES ${sources} PLUGINS_DIR ${plugins_dir})

add_dependencies(usdi MeshUtils)
target_include_directories(usdi PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(usdi
    MeshUtils
    ${USD_LIBRARY}
    ${PYTHON_LIBRARY}
    ${Boost_PYTHON_LIBRARY_RELEASE}
    ${Boost_FILESYSTEM_LIBRARY_RELEASE}
    ${TBB_LIBRARIES}
    ${OPENEXR_Half_LIBRARY}
)

add_definitions(-DusdiEnableBoostFilesystem)
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
