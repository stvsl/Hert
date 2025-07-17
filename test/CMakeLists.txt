cmake_minimum_required(VERSION 3.14)

project(HertTests LANGUAGES CXX)

include(../cmake/project-is-top-level.cmake)
include(../cmake/folders.cmake)

# 包含测试报告配置
include(../cmake/test-reports-config.cmake)
include(../cmake/test-reports.cmake)

# ---- Dependencies ----

if(PROJECT_IS_TOP_LEVEL)
  find_package(Hert REQUIRED)
  enable_testing()
endif()

find_package(Catch2 REQUIRED)
include(Catch)

# ---- 自动发现和配置测试 ----

# 自动发现所有测试源文件
file(GLOB_RECURSE TEST_SOURCE_FILES 
    "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/**/*.cpp"
)

# 过滤出有效的测试文件（排除非测试文件）
set(VALID_TEST_FILES "")
foreach(test_file ${TEST_SOURCE_FILES})
    get_filename_component(filename ${test_file} NAME_WE)
    # 检查文件名是否包含 test 或 Test 字样
    if(filename MATCHES ".*[Tt]est.*" OR filename MATCHES ".*[Tt]EST.*")
        list(APPEND VALID_TEST_FILES ${test_file})
    endif()
endforeach()

# 输出发现的测试文件信息
message(STATUS "Auto-discovered test files:")
foreach(test_file ${VALID_TEST_FILES})
    get_filename_component(test_name ${test_file} NAME_WE)
    message(STATUS "  - ${test_name}: ${test_file}")
endforeach()

# 如果没有发现任何测试文件，则发出警告
if(NOT VALID_TEST_FILES)
    message(WARNING "No test files found in ${CMAKE_CURRENT_SOURCE_DIR}/source/")
    message(WARNING "Expected files with pattern *test*.cpp or *Test*.cpp")
endif()

# 为每个测试文件创建独立的可执行文件
set(ALL_TEST_TARGETS "")
foreach(test_file ${VALID_TEST_FILES})
    get_filename_component(test_name ${test_file} NAME_WE)
    
    # 创建测试可执行文件
    add_executable(${test_name} ${test_file})
    
    # 设置链接库
    target_link_libraries(${test_name} PRIVATE
        Hert::Hert
        Catch2::Catch2WithMain
    )
    
    # 设置C++标准
    target_compile_features(${test_name} PRIVATE cxx_std_20)
    
    # 添加到测试目标列表
    list(APPEND ALL_TEST_TARGETS ${test_name})
    
    # 注册测试目标
    register_test_target(${test_name})
    
    # Catch2 测试发现
    catch_discover_tests(${test_name} PROPERTIES TIMEOUT ${TEST_TIMEOUT})
    
    message(STATUS "Configured test target: ${test_name}")
endforeach()

# 验证测试报告配置
validate_test_reports_config()

# ---- 综合测试项目配置 ----

# 创建一个综合测试可执行文件（可选，包含所有测试）
if(ENABLE_UNIFIED_TEST_EXECUTABLE)
    add_executable(AllTests ${VALID_TEST_FILES})
    target_link_libraries(AllTests PRIVATE
        Hert::Hert
        Catch2::Catch2WithMain
    )
    target_compile_features(AllTests PRIVATE cxx_std_20)
    register_test_target(AllTests)
    catch_discover_tests(AllTests PROPERTIES TIMEOUT ${TEST_TIMEOUT})
    message(STATUS "Created unified test executable: AllTests")
endif()

# 设置综合测试报告
setup_unified_test_reports()

# 生成综合 HTML 报告
generate_unified_html_report()

# 创建自定义测试运行目标
add_custom_target(run-all-tests
    COMMENT "Running all discovered tests"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# 为每个测试目标添加运行命令
foreach(test_target ${ALL_TEST_TARGETS})
    add_custom_command(TARGET run-all-tests POST_BUILD
        COMMAND ${test_target}
        COMMENT "Running ${test_target}"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
    add_dependencies(run-all-tests ${test_target})
endforeach()

# 创建测试覆盖率目标（如果启用）
if(ENABLE_COVERAGE_REPORTS)
    find_program(GCOV_PATH gcov)
    find_program(LCOV_PATH lcov)
    find_program(GENHTML_PATH genhtml)
    
    if(GCOV_PATH AND LCOV_PATH AND GENHTML_PATH)
        add_custom_target(coverage
            COMMENT "Generating test coverage report"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
        
        # 添加覆盖率编译选项
        foreach(test_target ${ALL_TEST_TARGETS})
            target_compile_options(${test_target} PRIVATE --coverage)
            target_link_options(${test_target} PRIVATE --coverage)
        endforeach()
        
        add_custom_command(TARGET coverage POST_BUILD
            COMMAND ${LCOV_PATH} --directory . --capture --output-file coverage.info
            COMMAND ${LCOV_PATH} --remove coverage.info ${COVERAGE_EXCLUDES} --output-file coverage.info.cleaned
            COMMAND ${GENHTML_PATH} -o ${TEST_REPORTS_OUTPUT_DIR}/coverage coverage.info.cleaned
            COMMENT "Generating HTML coverage report"
        )
        
        add_dependencies(coverage run-all-tests)
        message(STATUS "Coverage reporting enabled")
    else()
        message(WARNING "Coverage tools not found. Coverage reporting disabled.")
    endif()
endif()

# 创建性能测试目标（如果有性能测试）
add_custom_target(run-performance-tests
    COMMENT "Running performance tests"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

foreach(test_target ${ALL_TEST_TARGETS})
    add_custom_command(TARGET run-performance-tests POST_BUILD
        COMMAND ${test_target} ${PERFORMANCE_TEST_TAGS} --reporter json --out ${TEST_REPORTS_OUTPUT_DIR}/${test_target}-performance.json
        COMMENT "Running performance tests for ${test_target}"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
    add_dependencies(run-performance-tests ${test_target})
endforeach()

# 打印测试项目配置信息
message(STATUS "=== Hert Test Project Configuration ===")
list(LENGTH ALL_TEST_TARGETS NUM_TEST_TARGETS)
message(STATUS "Total test targets discovered: ${NUM_TEST_TARGETS}")
message(STATUS "Test reports directory: ${TEST_REPORTS_OUTPUT_DIR}")
message(STATUS "Available test targets:")
foreach(target ${ALL_TEST_TARGETS})
    message(STATUS "  - ${target}")
endforeach()

# 打印可用的自定义目标
message(STATUS "Available custom targets:")
message(STATUS "  - run-all-tests:          Run all discovered tests")
message(STATUS "  - run-performance-tests:  Run performance tests")
message(STATUS "  - unified-junit:          Generate unified JUnit XML reports")
message(STATUS "  - unified-json:           Generate unified JSON reports")
message(STATUS "  - unified-html-report:    Generate unified HTML report")
message(STATUS "  - unified-all-reports:    Generate all report formats")
message(STATUS "  - clean-test-reports:     Clean test reports directory")
if(ENABLE_COVERAGE_REPORTS AND GCOV_PATH AND LCOV_PATH AND GENHTML_PATH)
    message(STATUS "  - coverage:               Generate test coverage report")
endif()

# 打印配置信息
print_test_reports_config()

# ---- End-of-file commands ----

add_folders(Test)
