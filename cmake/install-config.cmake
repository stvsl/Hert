set(Hert_FOUND YES)

include(CMakeFindDependencyMacro)
find_dependency(fmt)

if(Hert_FOUND)
  include("${CMAKE_CURRENT_LIST_DIR}/HertTargets.cmake")
endif()
