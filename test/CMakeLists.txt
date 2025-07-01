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

# ---- Tests ----

add_executable(Hert_test source/Hert_test.cpp)
target_link_libraries(
    Hert_test PRIVATE
    Hert::Hert
    Catch2::Catch2WithMain
)
target_compile_features(Hert_test PRIVATE cxx_std_20)

# 验证测试报告配置
validate_test_reports_config()

# 设置测试报告
setup_test_reports(Hert_test)

# 生成 HTML 报告
generate_html_report(Hert_test)

# 标准的 Catch2 测试发现
catch_discover_tests(Hert_test PROPERTIES TIMEOUT ${TEST_TIMEOUT})

# 打印配置信息
print_test_reports_config()

# ---- End-of-file commands ----

add_folders(Test)
