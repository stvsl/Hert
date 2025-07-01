# Test Reports Module
# This module provides functionality to generate various test reports

# 设置测试报告输出目录
set(TEST_REPORTS_DIR "${CMAKE_BINARY_DIR}/test-reports" CACHE PATH "Directory for test reports")

# 创建测试报告目录
file(MAKE_DIRECTORY ${TEST_REPORTS_DIR})

# 定义测试报告生成函数
function(setup_test_reports target_name)
    # 确保目标存在
    if(NOT TARGET ${target_name})
        message(FATAL_ERROR "Target ${target_name} does not exist")
    endif()

    # 设置测试报告相关的编译定义
    target_compile_definitions(${target_name} PRIVATE 
        TEST_REPORTS_DIR="${TEST_REPORTS_DIR}"
    )

    # 添加自定义测试目标，生成不同格式的报告
    
    # JUnit XML 报告（用于 CI/CD 集成）
    add_custom_target(test-junit
        COMMAND ${target_name} --reporter junit --out ${TEST_REPORTS_DIR}/junit-report.xml
        DEPENDS ${target_name}
        COMMENT "Running tests and generating JUnit XML report"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

    # Console 报告（彩色输出）
    add_custom_target(test-console
        COMMAND ${target_name} --reporter console
        DEPENDS ${target_name}
        COMMENT "Running tests with console output"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

    # 详细的 XML 报告
    add_custom_target(test-xml
        COMMAND ${target_name} --reporter xml --out ${TEST_REPORTS_DIR}/detailed-report.xml
        DEPENDS ${target_name}
        COMMENT "Running tests and generating detailed XML report"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

    # JSON 报告（用于后续处理）
    add_custom_target(test-json
        COMMAND ${target_name} --reporter json --out ${TEST_REPORTS_DIR}/test-results.json
        DEPENDS ${target_name}
        COMMENT "Running tests and generating JSON report"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

    # TAP 报告
    add_custom_target(test-tap
        COMMAND ${target_name} --reporter tap --out ${TEST_REPORTS_DIR}/test-results.tap
        DEPENDS ${target_name}
        COMMENT "Running tests and generating TAP report"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

    # 综合测试报告目标，生成所有格式
    add_custom_target(test-all-reports
        COMMENT "Generating all test report formats"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
    
    # 让综合目标依赖于所有单独的报告目标
    add_dependencies(test-all-reports 
        test-junit 
        test-xml 
        test-json 
        test-tap
    )

    # 测试覆盖率报告（如果启用了覆盖率）
    if(ENABLE_COVERAGE)
        add_custom_target(test-coverage-report
            COMMAND ${target_name} --reporter console
            COMMAND gcov -r ${CMAKE_BINARY_DIR} || true
            COMMAND lcov --capture --directory ${CMAKE_BINARY_DIR} --output-file ${TEST_REPORTS_DIR}/coverage.info --quiet || true
            COMMAND genhtml ${TEST_REPORTS_DIR}/coverage.info --output-directory ${TEST_REPORTS_DIR}/coverage-html --quiet --show-details --legend || true
            DEPENDS ${target_name}
            COMMENT "Running tests and generating coverage report"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
        
        add_dependencies(test-all-reports test-coverage-report)
    endif()

    # 性能基准测试报告
    add_custom_target(test-benchmark
        COMMAND ${target_name} --reporter console "[performance]"
        COMMAND ${target_name} --reporter json --out ${TEST_REPORTS_DIR}/benchmark-results.json "[performance]"
        DEPENDS ${target_name}
        COMMENT "Running performance benchmarks"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endfunction()

# 生成 HTML 测试报告的函数
function(generate_html_report target_name)
    find_program(PYTHON_EXECUTABLE python3 python)
    
    if(PYTHON_EXECUTABLE)
        # 创建 HTML 报告生成脚本
        configure_file(
            ${CMAKE_SOURCE_DIR}/cmake/generate_html_report.py.in
            ${CMAKE_BINARY_DIR}/generate_html_report.py
            @ONLY
        )
        
        add_custom_target(test-html-report
            COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_BINARY_DIR}/generate_html_report.py
            DEPENDS test-json
            COMMENT "Generating HTML test report"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
        
        add_dependencies(test-all-reports test-html-report)
    else()
        message(WARNING "Python not found. HTML report generation will be disabled.")
    endif()
endfunction()

# 设置测试报告清理目标
add_custom_target(clean-test-reports
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${TEST_REPORTS_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${TEST_REPORTS_DIR}
    COMMENT "Cleaning test reports directory"
)

# 打印测试报告信息
function(print_test_reports_info)
    message(STATUS "Test Reports Configuration:")
    message(STATUS "  Reports Directory: ${TEST_REPORTS_DIR}")
    message(STATUS "  Available targets:")
    message(STATUS "    test-junit         - Generate JUnit XML report")
    message(STATUS "    test-xml           - Generate detailed XML report")
    message(STATUS "    test-json          - Generate JSON report")
    message(STATUS "    test-tap           - Generate TAP report")
    message(STATUS "    test-console       - Run tests with console output")
    message(STATUS "    test-benchmark     - Run performance benchmarks")
    message(STATUS "    test-all-reports   - Generate all report formats")
    message(STATUS "    clean-test-reports - Clean reports directory")
    if(ENABLE_COVERAGE)
        message(STATUS "    test-coverage-report - Generate coverage report")
    endif()
endfunction()
