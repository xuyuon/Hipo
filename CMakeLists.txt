#----------------------------------------------------------------------------
# Setup the project
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(G4Earth)

#----------------------------------------------------------------------------
# Set the build type 
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

#----------------------------------------------------------------------------
# GoogleTest requires at least C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#----------------------------------------------------------------------------
# Find Geant4 package, activating all available UI and Vis drivers by default
find_package(Geant4 REQUIRED ui_all vis_all)

#----------------------------------------------------------------------------
# Setup Geant4 include directories and compile definitions
include(${Geant4_USE_FILE})

#----------------------------------------------------------------------------
# Declare a dependency on GoogleTest
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

#----------------------------------------------------------------------------
# Locate sources and headers for this project
include_directories(${PROJECT_SOURCE_DIR}/include ${Geant4_INCLUDE_DIR})
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)
file(GLOB tests ${PROJECT_SOURCE_DIR}/test/*.cc)
message("${headers}")

#----------------------------------------------------------------------------
# Add the executable, and link it to the Geant4 libraries
add_executable(run run.cc ${sources} ${headers})
target_link_libraries(run ${Geant4_LIBRARIES})
enable_testing()
add_executable(run_test ${tests} ${sources} ${headers})
target_link_libraries(run_test ${Geant4_LIBRARIES} GTest::gtest_main)

#----------------------------------------------------------------------------
# Enable the test runner to discover the tests included in the binary
include(GoogleTest)
gtest_discover_tests(run_test)

#----------------------------------------------------------------------------
# Copy all scripts to the build directory
file(GLOB MACRO_FILES
    "*.mac"
)
file(COPY ${MACRO_FILES} DESTINATION ${CMAKE_BINARY_DIR})

#----------------------------------------------------------------------------
# Copy all input data file to the build directory

file(GLOB INPUT_FILES
    "input/*"
)
file(COPY ${INPUT_FILES} DESTINATION ${CMAKE_BINARY_DIR})

#----------------------------------------------------------------------------
# Copy all input data file to the build directory

#----------------------------------------------------------------------------
# Add program to the project targets
# (this avoids the need of typing the program name after make)
add_custom_target(G4Earth DEPENDS run)