# 综合测试报告系统

本项目已升级为使用综合测试报告系统，不再为每个测试文件创建单独的构建目标和测试链，而是对所有测试进行统一管理并生成一个综合报告。

## 主要变化

### 旧系统（已弃用）

- 每个测试目标都有单独的报告目标：`Hert_test-junit`、`HertLog_test-junit` 等
- 需要手动管理多个报告文件
- 缺乏整体的测试概览

### 新系统（推荐）

- 所有测试统一注册和管理
- 生成单一的综合报告
- 更清晰的测试概览和统计信息

## 使用方法

### 1. 注册测试目标

在 `test/CMakeLists.txt` 中使用 `register_test_target()` 替代 `setup_test_reports()`：

```cmake
# 注册测试目标（新的综合报告系统）
register_test_target(Hert_test)
register_test_target(HertLog_test)

# 设置综合测试报告
setup_unified_test_reports()

# 生成综合 HTML 报告
generate_unified_html_report()
```

### 2. 可用的构建目标

#### 综合报告目标

- `unified-junit` - 生成统一的 JUnit XML 报告
- `unified-json` - 生成统一的 JSON 报告
- `unified-console` - 运行所有测试并显示控制台输出
- `unified-tap` - 生成统一的 TAP 报告
- `unified-html-report` - 生成美观的综合 HTML 报告
- `unified-all-reports` - 生成所有格式的统一报告

#### 实用工具目标

- `clean-test-reports` - 清理测试报告目录

### 3. 运行测试和生成报告

```bash
# 构建项目
cmake --build build

# 运行所有测试并生成控制台报告
cmake --build build --target unified-console

# 生成 JUnit XML 报告（适用于 CI/CD）
cmake --build build --target unified-junit

# 生成漂亮的 HTML 报告
cmake --build build --target unified-html-report

# 生成所有格式的报告
cmake --build build --target unified-all-reports
```

### 4. 报告输出位置

所有报告都生成在 `build/test-reports/` 目录下：

- `*-junit.xml` - JUnit XML 格式报告
- `*-results.json` - JSON 格式报告
- `*-results.tap` - TAP 格式报告
- `Hert-comprehensive-test-report.html` - 综合 HTML 报告

## 配置选项

在 `cmake/test-reports-config.cmake` 中可以配置：

```cmake
# 启用/禁用不同类型的报告
option(ENABLE_JUNIT_REPORTS "Enable JUnit XML reports" ON)
option(ENABLE_JSON_REPORTS "Enable JSON reports" ON)
option(ENABLE_HTML_REPORTS "Enable HTML reports" ON)
option(ENABLE_TAP_REPORTS "Enable TAP reports" OFF)
option(ENABLE_CONSOLE_REPORTS "Enable console reports" ON)

# 综合报告配置
option(ENABLE_UNIFIED_REPORTS "Enable unified comprehensive reports" ON)
set(UNIFIED_REPORT_NAME "${PROJECT_NAME}-comprehensive-test-report")
set(UNIFIED_REPORT_TITLE "${PROJECT_NAME} 综合测试报告")
```

## HTML 报告特性

综合 HTML 报告包含：

- **整体统计** - 总测试数、通过率、失败数、总耗时
- **测试套件视图** - 按测试可执行文件分组显示
- **详细测试用例** - 每个测试用例的状态、耗时、位置信息
- **错误详情** - 失败测试的详细错误信息
- **现代化 UI** - 响应式设计，支持移动设备

## 迁移指南

如果你之前使用旧系统，需要进行以下修改：

1. 将 `setup_test_reports(target)` 替换为 `register_test_target(target)`
2. 将 `generate_html_report(target)` 调用移除
3. 添加 `setup_unified_test_reports()` 和 `generate_unified_html_report()`
4. 更新 CI/CD 脚本使用新的目标名称

### CI/CD 集成示例

```yaml
# GitHub Actions 示例
- name: Run tests and generate reports
  run: |
    cmake --build build --target unified-all-reports
    
- name: Upload test reports
  uses: actions/upload-artifact@v3
  with:
    name: test-reports
    path: build/test-reports/
```

## 向后兼容性

为了保持向后兼容性，旧的 `setup_test_reports()` 函数仍然可用，但会自动调用 `register_test_target()`。不过建议迁移到新的 API 以获得更好的功能。
