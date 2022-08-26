#
# CMAKE file for Solver
#
cmake_minimum_required(VERSION 3.16)

project(json CXX)

set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "")
set(CMAKE_OSX_SYSROOT /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

# src
list(APPEND src src/JSONEncoder.cpp src/JSONEncoder.h)
list(APPEND src src/JSONDecoder.cpp src/JSONDecoder.h)
list(APPEND src src/StringWriter.cpp src/StringWriter.h)
list(APPEND src src/VectorReader.cpp src/VectorReader.h)
list(APPEND src src/VectorWriter.cpp src/VectorWriter.h)
list(APPEND src src/Memfile.cpp src/Memfile.h)
list(APPEND src src/Encoding.h)

# tests
list(APPEND tests tests/test_jsonencoder.cpp)
list(APPEND tests tests/test_jsondecoder.cpp)


#
# app
#
add_executable(app main.cpp ${src})
target_include_directories(app PRIVATE src/)

#
# build unit test dynlib - the test runner requires tests to be in a dynamic library
#
add_library(jsonlib SHARED ${src} ${tests})
target_include_directories(jsonlib PRIVATE src/)

#
# Unit test runner (see: https://github.com/gnilk/testrunner )
#
if(WIN32)
    set(TRUN_CMD "c:/bin/trun.exe")
else(UNIX)
    set(TRUN_CMD "/usr/local/bin/trun")
    target_include_directories(jsonlib PUBLIC /usr/local/include)
    target_compile_options(jsonlib PUBLIC -Wall -pedantic -Wextra -Wshadow)
    target_compile_options(jsonlib PUBLIC -Wno-unused-parameter)
endif()





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
