cmake_minimum_required(VERSION 3.0.0)

message("Begin LibRPM build")
message("Target platform: " ${RPM_PLATFORM})

IF (RPM_PLATFORM STREQUAL "ARMv5T")
message("Building for ARMv5T")

set(CMAKE_SYSTEM_NAME               Generic)
set(CMAKE_SYSTEM_PROCESSOR          arm)
set(CMAKE_C_FLAGS "--specs=nosys.specs")
set(CMAKE_CXX_FLAGS "--specs=nosys.specs")
set(CMAKE_EXE_LINKER_FLAGS "-r -march=armv5t -mthumb" CACHE INTERNAL "")

add_compile_options(-march=armv5t -mthumb)
add_compile_definitions(EXL_PLATFORM_GFL)

ELSEIF (RPM_PLATFORM STREQUAL "Win32")
message("Building for Win32")

ELSE ()
message( FATAL_ERROR "Invalid target platform!")

ENDIF ()

project(LibRPM VERSION 0.13.0)

add_compile_options(-fno-rtti -fno-exceptions -fvisibility=hidden)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../extlib)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
add_compile_definitions(DEBUG)
endif()

if (EXL_DESMUME)
add_compile_definitions(EXL_DESMUME)
endif()

file(GLOB DLL_SOURCES
    src/*.cpp
    include/*.h
    ../extlib/Util/exl_Print.*
)

add_library(LibRPM OBJECT ${DLL_SOURCES})
set_target_properties(LibRPM PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(LibRPM PUBLIC include)

add_library(LibRPM.Include OBJECT include/RPM_Api.h)
set_target_properties(LibRPM.Include PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(LibRPM.Include PUBLIC include)

IF (WIN32)
add_compile_definitions(EXL_PLATFORM_WIN32)
file(GLOB TESTS_SOURCES
    RPM_Tests.cpp
    ../extlib/ABI/*
    ../extlib/Heap/exl_HeapArea.*
    ../extlib/Heap/exl_MemOperators.*
    ../extlib/Heap/exl_Allocator.*
)

add_executable(RPMTests ${TESTS_SOURCES})
target_link_libraries(RPMTests LibRPM)

add_library(LibRPM.Static include/RPM_Api.h)
target_link_libraries(LibRPM.Static LibRPM)
ELSE ()
add_executable(LibRPM.DLL include/RPM_Api.h)
add_library(LibRPM.Static include/RPM_Api.h)
target_link_libraries(LibRPM.DLL LibRPM)
target_link_libraries(LibRPM.Static LibRPM)
ENDIF ()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
