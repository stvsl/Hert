# 测试报告系统使用指南

Hert 项目集成了完整的自动化测试报告生成系统，支持多种报告格式和部署环境。

## 🎯 功能特性

### 支持的报告格式
- **JUnit XML**: 用于 CI/CD 系统集成
- **HTML**: 可视化的详细测试报告
- **JSON**: 用于自动化分析和处理
- **TAP**: Test Anything Protocol 格式
- **Console**: 彩色控制台输出
- **覆盖率报告**: 代码覆盖率分析（可选）

### 测试类型
- **单元测试**: 基础功能测试
- **集成测试**: 组件间协作测试
- **性能基准测试**: 性能分析和回归检测
- **边界测试**: 极限情况测试

## 🚀 快速开始

### 1. 构建项目
```bash
# 配置构建环境
cmake --preset=dev

# 构建项目
cmake --build build/dev
```

### 2. 运行测试并生成报告

#### 使用便捷脚本（推荐）
```bash
# 运行所有测试并生成所有格式的报告
./scripts/generate_test_reports.sh

# 清理旧报告后运行
./scripts/generate_test_reports.sh --clean

# 只运行性能基准测试
./scripts/generate_test_reports.sh --benchmark

# 包含覆盖率报告
./scripts/generate_test_reports.sh --coverage

# 显示帮助信息
./scripts/generate_test_reports.sh --help
```

#### 使用 CMake 目标
```bash
cd build/dev

# 生成所有格式的报告
make test-all-reports

# 生成特定格式的报告
make test-junit      # JUnit XML 报告
make test-html-report # HTML 报告
make test-json       # JSON 报告
make test-xml        # 详细 XML 报告
make test-tap        # TAP 报告

# 性能基准测试
make test-benchmark

# 覆盖率报告（需要先启用覆盖率）
make test-coverage-report

# 清理报告
make clean-test-reports
```

#### 直接使用测试可执行文件
```bash
cd build/dev

# 基本测试运行
./test/Hert_test

# 生成 JUnit XML 报告
./test/Hert_test --reporter junit --out test-reports/junit.xml

# 生成 HTML 友好的 JSON 报告
./test/Hert_test --reporter json --out test-reports/results.json

# 只运行性能测试
./test/Hert_test --reporter console "[performance]"

# 运行特定标签的测试
./test/Hert_test --reporter console "[integration]"
```

## 📊 报告输出

### 报告文件位置
所有报告都会生成到 `build/dev/test-reports/` 目录下：

```
test-reports/
├── junit-report.xml          # JUnit XML 报告
├── detailed-report.xml       # 详细 XML 报告
├── test-results.json         # JSON 格式结果
├── test-results.tap          # TAP 格式报告
├── console-report.txt        # 控制台输出
├── test-report.html          # HTML 可视化报告
├── benchmark-results.json    # 性能基准测试结果
├── coverage.info             # 覆盖率数据文件
└── coverage-html/            # HTML 覆盖率报告
    └── index.html
```

### HTML 报告特性
- 📈 **可视化统计**: 测试通过率、失败率图表
- 🎨 **现代化界面**: 响应式设计，支持移动设备
- 🔍 **详细信息**: 每个测试用例的详细结果
- ⚡ **性能数据**: 测试执行时间统计
- 🎯 **错误诊断**: 失败测试的详细错误信息

### 覆盖率报告
覆盖率报告提供详细的代码覆盖率分析：
- 文件级覆盖率统计
- 函数级覆盖率分析
- 行级覆盖率高亮
- 分支覆盖率统计

## ⚙️ 配置选项

### CMake 配置选项
```cmake
# 基本报告格式
option(ENABLE_JUNIT_REPORTS "Enable JUnit XML reports" ON)
option(ENABLE_JSON_REPORTS "Enable JSON reports" ON)
option(ENABLE_HTML_REPORTS "Enable HTML reports" ON)
option(ENABLE_TAP_REPORTS "Enable TAP reports" OFF)

# 高级功能
option(ENABLE_BENCHMARK_REPORTS "Enable benchmark reports" ON)
option(ENABLE_COVERAGE_REPORTS "Enable coverage reports" OFF)
option(AUTO_OPEN_HTML_REPORT "Auto open HTML report" ON)

# 性能配置
set(BENCHMARK_ITERATIONS "100" CACHE STRING "Benchmark iterations")
set(TEST_TIMEOUT "300" CACHE STRING "Test timeout in seconds")
```

### 环境变量配置
```bash
# 设置报告输出目录
export TEST_REPORTS_DIR="/custom/path/to/reports"

# 设置测试超时时间
export CTEST_TEST_TIMEOUT=600

# 启用详细输出
export CTEST_OUTPUT_ON_FAILURE=1
```

## 🔧 CI/CD 集成

### GitHub Actions
项目包含了完整的 GitHub Actions 工作流 (`.github/workflows/test-reports.yml`)：

- ✅ 自动运行测试
- 📊 生成多种格式报告
- 📤 上传测试 artifacts
- 💬 PR 中自动评论测试结果
- 📈 性能基准测试追踪

### GitLab CI
```yaml
test_reports:
  stage: test
  script:
    - cmake --preset=dev -DENABLE_COVERAGE_REPORTS=ON
    - cmake --build build/dev
    - cd build/dev && make test-all-reports
  artifacts:
    reports:
      junit: build/dev/test-reports/junit-report.xml
      coverage_report:
        coverage_format: cobertura
        path: build/dev/test-reports/coverage.xml
    paths:
      - build/dev/test-reports/
    expire_in: 30 days
```

### Jenkins
```groovy
pipeline {
    agent any
    stages {
        stage('Test') {
            steps {
                sh '''
                    cmake --preset=dev
                    cmake --build build/dev
                    cd build/dev && make test-all-reports
                '''
            }
            post {
                always {
                    publishTestResults testResultsPattern: 'build/dev/test-reports/junit-report.xml'
                    publishHTML([
                        allowMissing: false,
                        alwaysLinkToLastBuild: true,
                        keepAll: true,
                        reportDir: 'build/dev/test-reports',
                        reportFiles: 'test-report.html',
                        reportName: 'Test Report'
                    ])
                }
            }
        }
    }
}
```

## 🎨 自定义和扩展

### 添加自定义测试标签
```cpp
TEST_CASE("Custom test", "[custom][important]") {
    // 测试代码
}
```

运行特定标签的测试：
```bash
./test/Hert_test --reporter console "[custom]"
```

### 自定义 HTML 报告样式
编辑 `cmake/generate_html_report.py.in` 文件中的 CSS 样式来自定义报告外观。

### 添加自定义报告格式
在 `cmake/test-reports.cmake` 中添加新的自定义目标：

```cmake
add_custom_target(test-custom-format
    COMMAND ${target_name} --reporter custom --out ${TEST_REPORTS_DIR}/custom-report.txt
    DEPENDS ${target_name}
    COMMENT "Generating custom format report"
)
```

## 🐛 故障排除

### 常见问题

1. **Python 脚本执行失败**
   ```bash
   # 确保 Python3 已安装
   python3 --version
   
   # 检查脚本权限
   chmod +x cmake/generate_html_report.py
   ```

2. **覆盖率报告生成失败**
   ```bash
   # 安装覆盖率工具
   sudo apt-get install lcov gcov
   
   # 或者在 macOS 上
   brew install lcov
   ```

3. **HTML 报告无法打开**
   - 检查 `test-reports/test-report.html` 文件是否存在
   - 确保 JSON 报告生成成功
   - 检查 Python 脚本的错误输出

4. **测试超时**
   ```bash
   # 增加超时时间
   cmake --preset=dev -DTEST_TIMEOUT=600
   ```

### 调试信息

启用详细输出：
```bash
# CMake 详细输出
cmake --preset=dev --verbose

# CTest 详细输出
cd build/dev && ctest --verbose --output-on-failure

# 测试可执行文件调试信息
./test/Hert_test --reporter console --verbosity high
```

## 📚 相关文档

- [Catch2 文档](https://github.com/catchorg/Catch2/blob/devel/docs/Readme.md)
- [CMake Testing](https://cmake.org/cmake/help/latest/manual/ctest.1.html)
- [lcov 覆盖率工具](http://ltp.sourceforge.net/coverage/lcov.php)

## 🤝 贡献

如果您想改进测试报告系统：

1. Fork 项目
2. 创建功能分支
3. 提交您的更改
4. 创建 Pull Request

我们欢迎以下类型的改进：
- 新的报告格式
- 更好的 HTML 模板
- CI/CD 集成改进
- 性能优化

## 📄 许可证

本测试报告系统遵循项目的整体许可证。
