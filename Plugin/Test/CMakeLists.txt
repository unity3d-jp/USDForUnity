if(USDI_ENABLE_TESTS)
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}")
    file(GLOB sources *.cpp *.h)
    file(GLOB main_cpp usdiTestsMain.cpp)
    list(REMOVE_ITEM sources ${main_cpp})

    add_executable(Test ${sources})
    add_dependencies(Test usdi usdiRT MeshUtils)
    target_link_libraries(Test usdi usdiRT MeshUtils)

    add_executable(TestMain ${main_cpp})
    add_dependencies(TestMain Test)
    target_link_libraries(TestMain
        usdiRT MeshUtils
        ${Boost_SYSTEM_LIBRARY_RELEASE}
        ${Boost_FILESYSTEM_LIBRARY_RELEASE}
        ${OPENEXR_Half_LIBRARY}
    )
endif()
