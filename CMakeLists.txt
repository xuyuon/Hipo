#----------------------------------------------------------------------------
# Setup the project
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(G4Earth)

#----------------------------------------------------------------------------
# Find Geant4 package, activating all available UI and Vis drivers by default
find_package(Geant4 REQUIRED ui_all vis_all)

#----------------------------------------------------------------------------
# Setup Geant4 include directories and compile definitions
include(${Geant4_USE_FILE})

#----------------------------------------------------------------------------
# Locate sources and headers for this project
include_directories(${PROJECT_SOURCE_DIR}/include ${Geant4_INCLUDE_DIR})
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)
message("${headers}")

#----------------------------------------------------------------------------
# Add the executable, and link it to the Geant4 libraries
add_executable(run run.cc ${sources} ${headers})
target_link_libraries(run ${Geant4_LIBRARIES})


#----------------------------------------------------------------------------
# Copy all scripts to the build directory
file(GLOB MACRO_FILES
    "*.mac"
)
file(COPY ${MACRO_FILES} DESTINATION ${CMAKE_BINARY_DIR})

#----------------------------------------------------------------------------
# Add program to the project targets
# (this avoids the need of typing the program name after make)
add_custom_target(G4Earth DEPENDS run)