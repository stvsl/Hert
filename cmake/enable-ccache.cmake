# Enable ccache if available and not explicitly disabled
if(NOT DEFINED ENV{CCACHE_DISABLE})
    find_program(CCACHE_PROGRAM ccache)
    if(CCACHE_PROGRAM)
        message(STATUS "ccache found: ${CCACHE_PROGRAM}")
        # Set launcher for C and C++ compilers
        set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
        set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
    else()
        message(STATUS "ccache not found. Skipping ccache configuration.")
    endif()
else()
    message(STATUS "Environment variable CCACHE_DISABLE is set. Not using ccache.")
endif()