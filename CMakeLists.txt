cmake_minimum_required(VERSION 3.10.2)

set(PROGRAM_NAME "differenceengine")
set(PROGRAM_VERSION 0.1.0)

project(${PROGRAM_NAME} VERSION ${PROGRAM_VERSION} DESCRIPTION "File Processing Engine (FPE)")

# Build Antik library

add_subdirectory(../Antik antik)

# FPE sources and includes

file(GLOB PROGRAM_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/Actions/*.cpp")
file(GLOB PROGRAM_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp" )

# FPE target

add_executable(${PROGRAM_NAME} ${PROGRAM_SOURCES} )
target_include_directories(${PROGRAM_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROGRAM_NAME} antik)

# Install FPE

install(TARGETS ${PROGRAM_NAME} DESTINATION bin)
