# Test Reports Module
# This module provides functionality to generate various test reports

# 设置测试报告输出目录
set(TEST_REPORTS_DIR "${CMAKE_BINARY_DIR}/test-reports" CACHE PATH "Directory for test reports")

# 创建测试报告目录
file(MAKE_DIRECTORY ${TEST_REPORTS_DIR})

# 用于存储所有测试目标的全局变量
set_property(GLOBAL PROPERTY ALL_TEST_TARGETS "")

# 注册测试目标函数
function(register_test_target target_name)
    # 确保目标存在
    if(NOT TARGET ${target_name})
        message(FATAL_ERROR "Target ${target_name} does not exist")
    endif()

    # 将目标添加到全局列表
    get_property(current_targets GLOBAL PROPERTY ALL_TEST_TARGETS)
    list(APPEND current_targets ${target_name})
    set_property(GLOBAL PROPERTY ALL_TEST_TARGETS "${current_targets}")

    # 设置测试报告相关的编译定义
    target_compile_definitions(${target_name} PRIVATE 
        TEST_REPORTS_DIR="${TEST_REPORTS_DIR}"
    )
endfunction()

# 生成综合测试报告函数
function(setup_unified_test_reports)
    get_property(test_targets GLOBAL PROPERTY ALL_TEST_TARGETS)
    
    if(NOT test_targets)
        message(WARNING "No test targets registered for unified reports")
        return()
    endif()

    # 创建运行所有测试的脚本
    set(run_tests_script "${CMAKE_BINARY_DIR}/run_all_tests.cmake")
    file(WRITE ${run_tests_script} "# Auto-generated script to run all tests\n")
    file(APPEND ${run_tests_script} "set(TEST_REPORTS_DIR \"${TEST_REPORTS_DIR}\")\n")
    file(APPEND ${run_tests_script} "set(TEST_TARGETS \"${test_targets}\")\n")
    file(APPEND ${run_tests_script} "set(CMAKE_BINARY_DIR \"${CMAKE_BINARY_DIR}\")\n\n")

    # 生成 JUnit XML 综合报告
    if(ENABLE_JUNIT_REPORTS)
        add_custom_target(unified-junit
            COMMENT "Running all tests and generating unified JUnit XML report"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
        
        foreach(target ${test_targets})
            add_custom_command(TARGET unified-junit POST_BUILD
                COMMAND ${target} --reporter junit --out ${TEST_REPORTS_DIR}/${target}-junit.xml
                COMMENT "Running ${target} for unified JUnit report"
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            )
            add_dependencies(unified-junit ${target})
        endforeach()
    endif()

    # 生成 JSON 综合报告
    if(ENABLE_JSON_REPORTS)
        add_custom_target(unified-json
            COMMENT "Running all tests and generating unified JSON report"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
        
        foreach(target ${test_targets})
            add_custom_command(TARGET unified-json POST_BUILD
                COMMAND ${target} --reporter json --out ${TEST_REPORTS_DIR}/${target}-results.json
                COMMENT "Running ${target} for unified JSON report"
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            )
            add_dependencies(unified-json ${target})
        endforeach()
    endif()

    # 生成控制台综合报告
    if(ENABLE_CONSOLE_REPORTS)
        add_custom_target(unified-console
            COMMENT "Running all tests with console output"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
        
        foreach(target ${test_targets})
            add_custom_command(TARGET unified-console POST_BUILD
                COMMAND ${target} --reporter console
                COMMENT "Running ${target} with console output"
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            )
            add_dependencies(unified-console ${target})
        endforeach()
    endif()

    # 生成 TAP 综合报告
    if(ENABLE_TAP_REPORTS)
        add_custom_target(unified-tap
            COMMENT "Running all tests and generating unified TAP report"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
        
        foreach(target ${test_targets})
            add_custom_command(TARGET unified-tap POST_BUILD
                COMMAND ${target} --reporter tap --out ${TEST_REPORTS_DIR}/${target}-results.tap
                COMMENT "Running ${target} for unified TAP report"
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            )
            add_dependencies(unified-tap ${target})
        endforeach()
    endif()

    # 创建综合的所有报告目标
    add_custom_target(unified-all-reports
        COMMENT "Generating all unified test report formats"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
    
    # 让综合目标依赖于所有启用的报告类型
    if(ENABLE_JUNIT_REPORTS)
        add_dependencies(unified-all-reports unified-junit)
    endif()
    if(ENABLE_JSON_REPORTS)
        add_dependencies(unified-all-reports unified-json)
    endif()
    if(ENABLE_CONSOLE_REPORTS)
        add_dependencies(unified-all-reports unified-console)
    endif()
    if(ENABLE_TAP_REPORTS)
        add_dependencies(unified-all-reports unified-tap)
    endif()

endfunction()

# 传统的单独测试报告函数（保留以向后兼容）
function(setup_test_reports target_name)
    register_test_target(${target_name})
endfunction()

# 生成综合 HTML 测试报告的函数
function(generate_unified_html_report)
    find_program(PYTHON_EXECUTABLE python3 python)
    
    if(PYTHON_EXECUTABLE AND ENABLE_HTML_REPORTS)
        # 设置报告生成需要的变量
        set(TEST_REPORTS_DIR "${TEST_REPORTS_OUTPUT_DIR}")
        
        # 获取项目版本，尝试多种方式
        message(STATUS "Debug version info:")
        message(STATUS "  PROJECT_VERSION: ${PROJECT_VERSION}")
        message(STATUS "  CMAKE_PROJECT_VERSION: ${CMAKE_PROJECT_VERSION}")
        message(STATUS "  Hert_VERSION: ${Hert_VERSION}")
        message(STATUS "  ${PROJECT_NAME}_VERSION: ${${PROJECT_NAME}_VERSION}")
        
        if(DEFINED Hert_VERSION AND NOT "${Hert_VERSION}" STREQUAL "")
            set(REPORT_PROJECT_VERSION "${Hert_VERSION}")
            message(STATUS "Using Hert_VERSION: ${Hert_VERSION}")
        elseif(DEFINED PROJECT_VERSION AND NOT "${PROJECT_VERSION}" STREQUAL "")
            set(REPORT_PROJECT_VERSION "${PROJECT_VERSION}")
            message(STATUS "Using PROJECT_VERSION: ${PROJECT_VERSION}")
        elseif(DEFINED CMAKE_PROJECT_VERSION AND NOT "${CMAKE_PROJECT_VERSION}" STREQUAL "")
            set(REPORT_PROJECT_VERSION "${CMAKE_PROJECT_VERSION}")
            message(STATUS "Using CMAKE_PROJECT_VERSION: ${CMAKE_PROJECT_VERSION}")
        else()
            set(REPORT_PROJECT_VERSION "0.1.0")
            message(STATUS "Using default version: 0.1.0")
        endif()
        
        message(STATUS "Final REPORT_PROJECT_VERSION: ${REPORT_PROJECT_VERSION}")
        
        # 创建综合 HTML 报告生成脚本
        configure_file(
            ${CMAKE_SOURCE_DIR}/cmake/generate_unified_html_report.py.in
            ${CMAKE_BINARY_DIR}/generate_unified_html_report.py
            @ONLY
        )
        
        add_custom_target(unified-html-report
            COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_BINARY_DIR}/generate_unified_html_report.py
            DEPENDS unified-json
            COMMENT "Generating unified HTML test report for all tests"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
        
        # 添加到综合报告依赖
        if(TARGET unified-all-reports)
            add_dependencies(unified-all-reports unified-html-report)
        endif()
    else()
        if(NOT PYTHON_EXECUTABLE)
            message(WARNING "Python not found. HTML report generation will be disabled.")
        endif()
    endif()
endfunction()

# 传统的单独 HTML 报告生成函数（已弃用，但保留以向后兼容）
function(generate_html_report target_name)
    message(WARNING "generate_html_report(${target_name}) is deprecated. Use generate_unified_html_report() instead.")
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
    message(STATUS "  Available unified targets:")
    message(STATUS "    unified-junit         - Generate unified JUnit XML report")
    message(STATUS "    unified-json          - Generate unified JSON report")
    message(STATUS "    unified-tap           - Generate unified TAP report")
    message(STATUS "    unified-console       - Run all tests with console output")
    message(STATUS "    unified-html-report   - Generate unified HTML report")
    message(STATUS "    unified-all-reports   - Generate all unified report formats")
    message(STATUS "    clean-test-reports    - Clean reports directory")
    message(STATUS "  Note: Individual test target reports are deprecated")
endfunction()
