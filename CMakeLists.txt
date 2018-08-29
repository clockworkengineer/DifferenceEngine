cmake_minimum_required(VERSION 3.10.2)

project("differenceengine" VERSION 0.1.0 DESCRIPTION "File Processing Engine (FPE)")

# Build Antik library

add_subdirectory(../Antik antik)

# FPE sources and includes

file(GLOB PROGRAM_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/Actions/*.cpp")
file(GLOB PROGRAM_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp" )

# FPE target

add_executable(${PROJECT_NAME} ${PROGRAM_SOURCES} )
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} antik)

# Install FPE

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
