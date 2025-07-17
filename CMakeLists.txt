cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    Hert
    VERSION 0.1.0
    DESCRIPTION "A Develop Tool Box Based On Qt"
    HOMEPAGE_URL "https://example.com/"
    LANGUAGES CXX
)

# 生成项目版本头文件
configure_file(
    ${CMAKE_SOURCE_DIR}/include/Hert/HertVersion.h.in
    ${CMAKE_BINARY_DIR}/include/Hert/HertVersion.h
    @ONLY
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)
include(cmake/enable-ccache.cmake)
include(cmake/file-collector.cmake)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# ---- Declare library ----

file_collector(
    HERT_SOURCES
    FOLDERS ${CMAKE_SOURCE_DIR}/source
    EXTENSIONS .cpp
)

file_collector(
    HERT_HEADERS
    FOLDERS ${CMAKE_SOURCE_DIR}/include
    EXTENSIONS .hpp .h
)

add_library(
    Hert_Hert
    ${HERT_SOURCES}
    ${HERT_HEADERS}
)
add_library(Hert::Hert ALIAS Hert_Hert)

include(GenerateExportHeader)
generate_export_header(
    Hert_Hert
    BASE_NAME Hert
    EXPORT_FILE_NAME export/Hert/Hert_export.hpp
    CUSTOM_CONTENT_FROM_VARIABLE pragma_suppress_c4251
)

if(NOT BUILD_SHARED_LIBS)
  target_compile_definitions(Hert_Hert PUBLIC HERT_STATIC_DEFINE)
endif()

set_target_properties(
    Hert_Hert PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
    VERSION "${PROJECT_VERSION}"
    SOVERSION "${PROJECT_VERSION_MAJOR}"
    EXPORT_NAME Hert
    OUTPUT_NAME Hert
)

target_include_directories(
    Hert_Hert ${warning_guard}
    PUBLIC
    "\$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>"
    "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)

target_include_directories(
    Hert_Hert SYSTEM
    PUBLIC
    "\$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/export>"
)

target_compile_features(Hert_Hert PUBLIC cxx_std_20)

find_package(fmt REQUIRED)
find_package(cpptrace CONFIG REQUIRED)
find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)
find_package(spdlog CONFIG REQUIRED)

target_link_libraries(Hert_Hert 
    PRIVATE 
    fmt::fmt
    cpptrace::cpptrace
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    spdlog::spdlog
)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----

if(NOT Hert_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of Hert"
  )
endif()

include(cmake/dev-mode.cmake)

add_subdirectory(example)
