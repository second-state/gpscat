cmake_minimum_required(VERSION 3.8)

project(gpscat)

include(CTest)

set(CMAKE_CXX_STANDARD 17)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

include_directories(include)

find_program(LLVM_CONFIG llvm-config)

if(LLVM_CONFIG STREQUAL "LLVM_CONFIG-NOTFOUND")
    message(FATAL_ERROR "llvm-config was not found.")
endif()

execute_process(
    COMMAND ${LLVM_CONFIG} --version
    OUTPUT_VARIABLE LLVM_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

message(STATUS "LLVM version: ${LLVM_VERSION}")

execute_process(
    COMMAND ${LLVM_CONFIG} --cxxflags
    OUTPUT_VARIABLE LLVM_CXXFLAGS
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

string(REPLACE "-std=c++11" "-std=c++17" LLVM_CXXFLAGS ${LLVM_CXXFLAGS})
string(REPLACE "-fno-exceptions" "-fexceptions" LLVM_CXXFLAGS ${LLVM_CXXFLAGS})
string(REPLACE "-I" "-isystem " LLVM_CXXFLAGS ${LLVM_CXXFLAGS})

message(STATUS "LLVM_CXXFLAGS: ${LLVM_CXXFLAGS}")

execute_process(
    COMMAND ${LLVM_CONFIG} --libs irreader bitwriter ipo
    OUTPUT_VARIABLE LLVM_LIBS
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND ${LLVM_CONFIG} --system-libs
    OUTPUT_VARIABLE LLVM_SYSTEM_LIBS
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND ${LLVM_CONFIG} --ldflags
    OUTPUT_VARIABLE LLVM_LDFLAGS
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(LLVM_LIBS "${LLVM_LIBS} ${LLVM_SYSTEM_LIBS}")
string(STRIP ${LLVM_LIBS} LLVM_LIBS)

set(SYMENGINE_LIBS "-lsymengine -lgmp")

add_library(gpscat-libs STATIC
    lib/AssemblyCostModel.cpp
    lib/IRLocator.cpp
    lib/MappingExtractor.cpp
    lib/IRCostCalculator.cpp
    lib/CoFloCoWrapper.cpp
    lib/Utils.cpp
    lib/opt/Debugify.cpp
    include/gpscat/AssemblyCostModel.h
    include/gpscat/AssemblyInstruction.h
    include/gpscat/IRLocator.h
    include/gpscat/MappingExtractor.h
    include/gpscat/IRCostCalculator.h
    include/gpscat/CoFloCoWrapper.h
    include/gpscat/Utils.h
    include/csv-parser/csv.hpp
    include/opt/Debugify.h
)

set(SOURCES
    tools/gpscat-cost.cpp
)

add_executable(gpscat-cost ${SOURCES})

set(WARNING_FLAGS
    "-Wall -Wextra -pedantic"
)

set_target_properties(gpscat-cost gpscat-libs
    PROPERTIES COMPILE_FLAGS "${LLVM_CXXFLAGS} ${WARNING_FLAGS} -pthread"
)

target_link_libraries(gpscat-libs
    ${LLVM_LIBS} ${LLVM_LDFLAGS} ${SYMENGINE_LIBS}
)

target_link_libraries(gpscat-cost
    gpscat-libs ${LLVM_LIBS} ${LLVM_LDFLAGS} ${SYMENGINE_LIBS} Threads::Threads
)

add_executable(gpscat-score
    tools/gpscat-score.cpp
)

set_target_properties(gpscat-score
    PROPERTIES COMPILE_FLAGS "${LLVM_CXXFLAGS} ${WARNING_FLAGS} -pthread"
)

target_link_libraries(gpscat-score
    ${LLVM_LIBS} ${LLVM_LDFLAGS} ${SYMENGINE_LIBS} Threads::Threads
)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
