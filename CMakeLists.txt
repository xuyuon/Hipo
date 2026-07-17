cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(Hipo VERSION 0.1.0 LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(GNUInstallDirs)

find_package(Geant4 REQUIRED COMPONENTS ui_all vis_all)
include(${Geant4_USE_FILE})

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

file(GLOB sources CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/include/*.hh)
file(GLOB tests CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/test/*.cc)

add_library(hipo_core STATIC ${sources} ${headers})
target_include_directories(hipo_core
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(hipo_core PUBLIC ${Geant4_LIBRARIES})

add_executable(run run.cc)
target_link_libraries(run PRIVATE hipo_core)

enable_testing()
add_executable(run_test ${tests})
target_link_libraries(run_test PRIVATE hipo_core GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(run_test)

file(GLOB MACRO_FILES CONFIGURE_DEPENDS "*.mac")
file(GLOB INPUT_FILES CONFIGURE_DEPENDS "input/*")
file(GLOB PRIMARIES_FILES CONFIGURE_DEPENDS "primaries/*")
file(COPY ${MACRO_FILES} DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${INPUT_FILES} DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${PRIMARIES_FILES} DESTINATION ${CMAKE_BINARY_DIR})

install(TARGETS hipo_core run
  EXPORT HipoTargets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(DIRECTORY ${PROJECT_SOURCE_DIR}/input/ DESTINATION ${CMAKE_INSTALL_DATADIR}/hipo/input)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/primaries/ DESTINATION ${CMAKE_INSTALL_DATADIR}/hipo/primaries)

install(EXPORT HipoTargets
  FILE HipoTargets.cmake
  NAMESPACE Hipo::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Hipo
)

set(CPACK_PACKAGE_NAME "Hipo")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_IGNORE_FILES "/build/;/.git/;/.github/agents/")
include(CPack)

add_custom_target(Hipo DEPENDS run)
