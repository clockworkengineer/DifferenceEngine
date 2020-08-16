cmake_minimum_required(VERSION 3.10.2)

project("DifferenceEngine" VERSION 0.1.0 DESCRIPTION "File Processing Engine (FPE)")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wall -Wextra")

# Build Antik library

add_subdirectory(antik)

# FPE sources and includes

set (PROGRAM_SOURCES
    FPE.cpp
    FPE_ProcCmdLine.cpp
    FPE_TaskActions.cpp
    ./Actions/CopyFile.cpp
    ./Actions/EmailFile.cpp
    ./Actions/ImportCSVFile.cpp
    ./Actions/RunCommand.cpp
    ./Actions/VideoConversion.cpp
    ./Actions/ZipFile.cpp
)

set (PROGRAM_INCLUDES
    FPE_Actions.hpp
    FPE.hpp
    FPE_ProcCmdLine.hpp
    FPE_TaskAction.hpp
)

#file(GLOB PROGRAM_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/Actions/*.cpp")
#file(GLOB PROGRAM_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp" )

# FPE target

add_executable(${PROJECT_NAME} ${PROGRAM_SOURCES} )
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} antik)

# Install FPE

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
