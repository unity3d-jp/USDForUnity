file(GLOB sources *.cpp *.h)
set(plugins_dir "${CMAKE_SOURCE_DIR}/../USDForUnity/Assets/UTJ/USDForUnity/Plugins/x86_64")
add_plugin(usdiRT SOURCES ${sources} PLUGINS_DIR ${plugins_dir})

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(usdiRT dl)
endif()
install(TARGETS usdiRT DESTINATION .)
