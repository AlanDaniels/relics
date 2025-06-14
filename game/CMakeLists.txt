cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    Relics
    VERSION 0.1.0
    DESCRIPTION "A simple Minecraft clone"
    HOMEPAGE_URL "https://alandaniels.com/relics"
    LANGUAGES CXX
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# ---- Declare library ----

add_library(
    Relics_lib OBJECT
    source/lib.cpp
)

target_include_directories(
    Relics_lib ${warning_guard}
    PUBLIC
    "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/source>"
)

target_compile_features(Relics_lib PUBLIC cxx_std_17)

find_package(fmt REQUIRED)
target_link_libraries(Relics_lib PRIVATE fmt::fmt)

# ---- Declare executable ----

add_executable(Relics_exe source/main.cpp)
add_executable(Relics::exe ALIAS Relics_exe)

set_property(TARGET Relics_exe PROPERTY OUTPUT_NAME Relics)

target_compile_features(Relics_exe PRIVATE cxx_std_17)

target_link_libraries(Relics_exe PRIVATE Relics_lib)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----

if(NOT Relics_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of Relics"
  )
endif()

include(cmake/dev-mode.cmake)
