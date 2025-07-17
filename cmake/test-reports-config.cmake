# Test Reports Configuration File
# 配置测试报告的各种选项和模板

# 报告输出目录配置
set(TEST_REPORTS_OUTPUT_DIR "${CMAKE_BINARY_DIR}/test-reports" CACHE PATH "Test reports output directory")

# 报告格式配置
option(ENABLE_JUNIT_REPORTS "Enable JUnit XML reports" ON)
option(ENABLE_JSON_REPORTS "Enable JSON reports" ON)
option(ENABLE_HTML_REPORTS "Enable HTML reports" ON)
option(ENABLE_TAP_REPORTS "Enable TAP reports" OFF)
option(ENABLE_CONSOLE_REPORTS "Enable console reports" ON)

# 综合报告配置
option(ENABLE_UNIFIED_REPORTS "Enable unified comprehensive reports" ON)
set(UNIFIED_REPORT_NAME "${PROJECT_NAME}-comprehensive-test-report" CACHE STRING "Name for unified test report")
set(UNIFIED_REPORT_TITLE "${PROJECT_NAME} 综合测试报告" CACHE STRING "Title for unified test report")

# 性能测试配置
option(ENABLE_BENCHMARK_REPORTS "Enable benchmark reports" ON)
set(BENCHMARK_ITERATIONS "100" CACHE STRING "Number of benchmark iterations")

# 覆盖率报告配置
option(ENABLE_COVERAGE_REPORTS "Enable coverage reports" OFF)
set(COVERAGE_EXCLUDES "*/test/*;*/build/*;*/vcpkg_installed/*" CACHE STRING "Coverage exclusion patterns")

# HTML 报告样式配置
set(HTML_REPORT_TITLE "${PROJECT_NAME} 测试报告" CACHE STRING "HTML report title")
set(HTML_REPORT_THEME "modern" CACHE STRING "HTML report theme (modern, classic, dark)")

# 报告通知配置
option(ENABLE_REPORT_NOTIFICATIONS "Enable desktop notifications for test results" OFF)
option(AUTO_OPEN_HTML_REPORT "Automatically open HTML report in browser" ON)

# CI/CD 集成配置
option(GENERATE_GITLAB_ARTIFACTS "Generate GitLab CI artifacts.yml" OFF)
option(GENERATE_GITHUB_ACTIONS "Generate GitHub Actions test summary" OFF)

# 测试超时配置
set(TEST_TIMEOUT "300" CACHE STRING "Test timeout in seconds")

# 统一测试可执行文件配置
option(ENABLE_UNIFIED_TEST_EXECUTABLE "Create a single executable containing all tests" OFF)

# 自动测试发现配置
option(ENABLE_AUTO_TEST_DISCOVERY "Automatically discover test files" ON)
set(TEST_FILE_PATTERNS "*test*.cpp;*Test*.cpp;*TEST*.cpp" CACHE STRING "Patterns for test file discovery")

# 自定义测试标签
set(PERFORMANCE_TEST_TAGS "[performance]" CACHE STRING "Tags for performance tests")
set(INTEGRATION_TEST_TAGS "[integration]" CACHE STRING "Tags for integration tests")
set(UNIT_TEST_TAGS "[unit]" CACHE STRING "Tags for unit tests")

# 报告保留配置
set(REPORTS_RETENTION_DAYS "30" CACHE STRING "Number of days to keep old reports")
option(ENABLE_REPORT_ARCHIVING "Enable automatic report archiving" OFF)

# 邮件通知配置（可选）
set(NOTIFICATION_EMAIL "" CACHE STRING "Email address for test result notifications")
set(SMTP_SERVER "" CACHE STRING "SMTP server for email notifications")

# 打印配置信息
function(print_test_reports_config)
    message(STATUS "")
    message(STATUS "=== Test Reports Configuration ===")
    message(STATUS "Output Directory: ${TEST_REPORTS_OUTPUT_DIR}")
    message(STATUS "")
    message(STATUS "Enabled Report Formats:")
    if(ENABLE_JUNIT_REPORTS)
        message(STATUS "  ✓ JUnit XML")
    endif()
    if(ENABLE_JSON_REPORTS)
        message(STATUS "  ✓ JSON")
    endif()
    if(ENABLE_HTML_REPORTS)
        message(STATUS "  ✓ HTML")
    endif()
    if(ENABLE_TAP_REPORTS)
        message(STATUS "  ✓ TAP")
    endif()
    if(ENABLE_CONSOLE_REPORTS)
        message(STATUS "  ✓ Console")
    endif()
    message(STATUS "")
    if(ENABLE_BENCHMARK_REPORTS)
        message(STATUS "Benchmark Reports: Enabled (${BENCHMARK_ITERATIONS} iterations)")
    endif()
    if(ENABLE_COVERAGE_REPORTS)
        message(STATUS "Coverage Reports: Enabled")
    endif()
    message(STATUS "Test Timeout: ${TEST_TIMEOUT} seconds")
    message(STATUS "===================================")
    message(STATUS "")
endfunction()

# 验证配置
function(validate_test_reports_config)
    # 检查输出目录是否可写
    if(NOT EXISTS "${TEST_REPORTS_OUTPUT_DIR}")
        file(MAKE_DIRECTORY "${TEST_REPORTS_OUTPUT_DIR}")
    endif()
    
    # 检查必要的工具
    if(ENABLE_COVERAGE_REPORTS)
        find_program(GCOV_EXECUTABLE gcov)
        find_program(LCOV_EXECUTABLE lcov)
        if(NOT GCOV_EXECUTABLE OR NOT LCOV_EXECUTABLE)
            message(WARNING "Coverage tools (gcov/lcov) not found. Coverage reports will be disabled.")
            set(ENABLE_COVERAGE_REPORTS OFF PARENT_SCOPE)
        endif()
    endif()
    
    if(ENABLE_HTML_REPORTS)
        find_program(PYTHON_EXECUTABLE python3 python)
        if(NOT PYTHON_EXECUTABLE)
            message(WARNING "Python not found. HTML reports may not work properly.")
        endif()
    endif()
    
    # 验证测试超时值
    if(TEST_TIMEOUT LESS 10)
        message(WARNING "Test timeout (${TEST_TIMEOUT}s) is very short. Consider increasing it.")
    endif()
    
    if(TEST_TIMEOUT GREATER 3600)
        message(WARNING "Test timeout (${TEST_TIMEOUT}s) is very long. Consider decreasing it.")
    endif()
endfunction()
