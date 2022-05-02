#
# CMAKE file for Solver
#
cmake_minimum_required(VERSION 3.16)

project(json CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_OSX_ARCHITECTURES x86_64)


# src
list(APPEND src src/jsondecoder.cpp src/jsondecoder.h)
list(APPEND src src/jsonencoder.cpp src/jsonencoder.h)
list(APPEND src src/JSONDecoderNew.cpp src/JSONDecoderNew.h)
list(APPEND src src/StringWriter.cpp src/StringWriter.h)
list(APPEND src src/VectorReader.cpp src/VectorReader.h)
list(APPEND src src/VectorWriter.cpp src/VectorWriter.h)
list(APPEND src src/Memfile.cpp src/Memfile.h)
list(APPEND src src/encoding.h)

# tests
list(APPEND tests tests/test_jsondecoder.cpp)
list(APPEND tests tests/test_jsonencoder.cpp)
list(APPEND tests tests/test_jsondecodernew.cpp)


#
# app
#
add_executable(app main.cpp ${src})
target_include_directories(app PRIVATE src/)

#
# Unit test runner (see: https://github.com/gnilk/testrunner )
#
if(WIN32)
    set(TRUN_CMD "c:/bin/trun.exe")
else(UNIX)
    set(TRUN_CMD "/usr/local/bin/trun")
endif()


#
# build unit test dynlib - the test runner requires tests to be in a dynamic library
#
add_library(jsonlib SHARED ${src} ${tests})
target_include_directories(jsonlib PRIVATE src/)



# link the stuff
target_link_libraries(app)
target_link_libraries(jsonlib)


#
# Custom target to execute the tests through the test runner
#
add_custom_target(
        tests ALL
        DEPENDS jsonlib
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
