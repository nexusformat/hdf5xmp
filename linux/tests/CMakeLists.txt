add_subdirectory(catch)

add_definitions("-fexceptions")

include_directories(${CATCH_INCLUDE_DIR})
enable_testing(true)  # Enables unit-testing.

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/testFile
    ${CMAKE_CURRENT_SOURCE_DIR}/test.xmp
    ${CMAKE_CURRENT_SOURCE_DIR}/test1.hdf5
    ${CMAKE_CURRENT_SOURCE_DIR}/test2.hdf5
    ${CMAKE_CURRENT_SOURCE_DIR}/test3.hdf5
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_library(Catch INTERFACE)
target_include_directories(Catch INTERFACE ${CATCH_INCLUDE_DIR})

add_executable (Test test_main.cpp test_thumbnailer.cpp)
target_link_libraries(Test Catch thumbnailerCore)

add_test (NAME TestThumbnailerCore COMMAND Test --success)