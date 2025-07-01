if(PROJECT_IS_TOP_LEVEL)
  set(
      CMAKE_INSTALL_INCLUDEDIR "include/Hert-${PROJECT_VERSION}"
      CACHE STRING ""
  )
  set_property(CACHE CMAKE_INSTALL_INCLUDEDIR PROPERTY TYPE PATH)
endif()

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# find_package(<package>) call for consumers to find this project
# should match the name of variable set in the install-config.cmake script
set(package Hert)

install(
    DIRECTORY
    include/
    "${PROJECT_BINARY_DIR}/export/"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    COMPONENT Hert_Development
)

install(
    TARGETS Hert_Hert
    EXPORT HertTargets
    RUNTIME #
    COMPONENT Hert_Runtime
    LIBRARY #
    COMPONENT Hert_Runtime
    NAMELINK_COMPONENT Hert_Development
    ARCHIVE #
    COMPONENT Hert_Development
    INCLUDES #
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

write_basic_package_version_file(
    "${package}ConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion
)

# Allow package maintainers to freely override the path for the configs
set(
    Hert_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/${package}"
    CACHE STRING "CMake package config location relative to the install prefix"
)
set_property(CACHE Hert_INSTALL_CMAKEDIR PROPERTY TYPE PATH)
mark_as_advanced(Hert_INSTALL_CMAKEDIR)

install(
    FILES cmake/install-config.cmake
    DESTINATION "${Hert_INSTALL_CMAKEDIR}"
    RENAME "${package}Config.cmake"
    COMPONENT Hert_Development
)

install(
    FILES "${PROJECT_BINARY_DIR}/${package}ConfigVersion.cmake"
    DESTINATION "${Hert_INSTALL_CMAKEDIR}"
    COMPONENT Hert_Development
)

install(
    EXPORT HertTargets
    NAMESPACE Hert::
    DESTINATION "${Hert_INSTALL_CMAKEDIR}"
    COMPONENT Hert_Development
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
