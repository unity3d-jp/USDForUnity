if(USDI_ENABLE_GRAPHICS_INTERFACE)
    file(GLOB sources *.cpp *.h)
    add_library(GraphicsInterface STATIC ${sources})
    target_include_directories(GraphicsInterface PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/External/Unity/include
    )
endif()
