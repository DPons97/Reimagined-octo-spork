# CMAKE for project OctoSpork - An eterogeneous distributed system for object tracking and fun stuff!

cmake_minimum_required(VERSION 3.0)
project(OctoSpork)


set(CMAKE_CXX_STANDARD 14)

# Client executable
file(GLOB CLIENT_SOURCES
        "Client/*.h"
        "Client/*.cpp"
        "*.h"
        "*.cpp"
        )

add_executable(OctoSporkClient Client/main.cpp ${CLIENT_SOURCES})

# Include OpenCV for client
include_directories( ${OpenCV_INCLUDE_DIRS} )

find_package( OpenCV REQUIRED )
target_link_libraries( OctoSporkClient ${OpenCV_LIBS} )

# Server executable
file(GLOB SERVER_SOURCES
        "Server/*.h"
        "Server/*.cpp"
        "*.h"
        "*.cpp"
        )

add_executable(OctoSporkServer Server/main.cpp ${SERVER_SOURCES})
# Include Darknet libraries for server
# Path to Darknet project folder.
set(DARKNET_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Server/darknet/")
set(PROJ_NAME OctoSporkServer)

# Give a custom name to shared library which is provided by Darkned when
# the one compiled.
set(DARKNET_LIB_NAME libdarknet)

# Link Darknet library' include folder.
include_directories(${DARKNET_PATH}/include)

# Define Darknet library' shared library.
add_library(${DARKNET_LIB_NAME} SHARED IMPORTED)

# Set a path to Darknet library' shared library
set_property( TARGET ${DARKNET_LIB_NAME} PROPERTY IMPORTED_LOCATION "${DARKNET_PATH}/libdarknet.so" )

# Link all libraries together.
target_link_libraries(${PROJ_NAME} ${DARKNET_LIB_NAME})