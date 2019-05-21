# CMAKE for project OctoSpork - An eterogeneous distributed system for object tracking and fun stuff!

cmake_minimum_required(VERSION 3.0)
project(OctoSpork)


set(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

# Client executable
file(GLOB CLIENT_SOURCES
        "Client/*.h"
        "Client/*.cpp"
        "*.h"
        "*.cpp"
        )

add_executable(OctoSporkClient Client/main.cpp ${CLIENT_SOURCES} Client/CNode.cpp Client/CNode.h)

# Include OpenCV for client
include_directories( ${OpenCV_INCLUDE_DIRS} )

find_package( OpenCV REQUIRED )

# Server executable
file(GLOB SERVER_SOURCES
        "Server/*.h"
        "Server/*.cpp"
        "*.h"
        "*.cpp"
        )

add_executable(OctoSporkServer Server/main.cpp ${SERVER_SOURCES} Server/SNode.cpp Server/SNode.h Server/darknetCPP/DarknetCalculator.cpp Server/darknetCPP/DarknetCalculator.h Server/darknetCPP/darknet.c Server/darknetCPP/darknet.h Server/Instructions/Instruction.cpp Server/Instructions/Instruction.h Server/Instructions/bkgSubtraction.cpp Server/Instructions/bkgSubtraction.h Server/Instructions/Tracker.cpp Server/Instructions/Tracker.h Server/Planimetry.cpp Server/Planimetry.h)

#Include OpenCV
target_link_libraries( OctoSporkServer ${OpenCV_LIBS} )

# Include Darknet libraries for server
# Path to Darknet project folder.
set(DARKNET_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Server/darknet/")
set(PROJ_NAME OctoSporkServer)

# Give a custom name to shared library which is provided by Darknet when
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

# Additional executables
file(GLOB EXEC_SOURCES
        "*.h"
        "*.cpp"
        )
add_executable(bkgSubtraction Client/Executables/bkgndSubtraction.cpp ${EXEC_SOURCES})
target_link_libraries( bkgSubtraction ${OpenCV_LIBS} )

add_executable(nodeTracker Client/Executables/nodeTracker.cpp ${EXEC_SOURCES})
target_link_libraries(nodeTracker ${OpenCV_LIBS})



