#!/bin/bash

# Test Report Runner Script
# 自动运行测试并生成各种格式的报告

set -e

# 脚本配置
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
BUILD_DIR="${SCRIPT_DIR}/../build/dev"
REPORTS_DIR="${BUILD_DIR}/test-reports"
TEST_EXECUTABLE="${BUILD_DIR}/test/Hert_test"

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 打印带颜色的消息
print_message() {
    local color=$1
    local message=$2
    echo -e "${color}[$(date +'%Y-%m-%d %H:%M:%S')] ${message}${NC}"
}

print_success() {
    print_message "$GREEN" "✓ $1"
}

print_error() {
    print_message "$RED" "✗ $1"
}

print_warning() {
    print_message "$YELLOW" "⚠ $1"
}

print_info() {
    print_message "$BLUE" "ℹ $1"
}

# 检查依赖
check_dependencies() {
    print_info "检查依赖..."

    if [ ! -f "$TEST_EXECUTABLE" ]; then
        print_error "测试可执行文件不存在: $TEST_EXECUTABLE"
        print_info "请先构建项目: cd build/dev && make"
        exit 1
    fi

    print_success "测试可执行文件存在"
}

# 创建报告目录
setup_directories() {
    print_info "设置报告目录..."
    mkdir -p "$REPORTS_DIR"
    print_success "报告目录已创建: $REPORTS_DIR"
}

# 运行测试并生成报告
run_tests_with_reports() {
    print_info "开始运行测试并生成报告..."

    cd "$BUILD_DIR"

    # 1. 生成 JUnit XML 报告（用于 CI/CD）
    print_info "生成 JUnit XML 报告..."
    if "$TEST_EXECUTABLE" --reporter junit --out "$REPORTS_DIR/junit-report.xml"; then
        print_success "JUnit XML 报告生成成功"
    else
        print_warning "JUnit XML 报告生成失败，但继续执行其他报告"
    fi

    # 2. 生成详细 XML 报告
    print_info "生成详细 XML 报告..."
    if "$TEST_EXECUTABLE" --reporter xml --out "$REPORTS_DIR/detailed-report.xml"; then
        print_success "详细 XML 报告生成成功"
    else
        print_warning "详细 XML 报告生成失败"
    fi

    # 3. 生成 JSON 报告
    print_info "生成 JSON 报告..."
    if "$TEST_EXECUTABLE" --reporter json --out "$REPORTS_DIR/test-results.json"; then
        print_success "JSON 报告生成成功"
    else
        print_warning "JSON 报告生成失败"
    fi

    # 4. 生成 TAP 报告
    print_info "生成 TAP 报告..."
    if "$TEST_EXECUTABLE" --reporter tap --out "$REPORTS_DIR/test-results.tap"; then
        print_success "TAP 报告生成成功"
    else
        print_warning "TAP 报告生成失败"
    fi

    # 5. 生成控制台报告并保存到文件
    print_info "生成控制台报告..."
    if "$TEST_EXECUTABLE" --reporter console >"$REPORTS_DIR/console-report.txt" 2>&1; then
        print_success "控制台报告生成成功"
        echo ""
        print_info "测试摘要:"
        echo ""
        tail -n 20 "$REPORTS_DIR/console-report.txt"
    else
        print_warning "控制台报告生成失败"
    fi
}

# 运行性能基准测试
run_benchmarks() {
    print_info "运行性能基准测试..."

    cd "$BUILD_DIR"

    # 运行标记为 [performance] 的测试
    if "$TEST_EXECUTABLE" --reporter console "[performance]" >"$REPORTS_DIR/benchmark-console.txt" 2>&1; then
        print_success "性能测试控制台报告生成成功"
    fi

    if "$TEST_EXECUTABLE" --reporter json --out "$REPORTS_DIR/benchmark-results.json" "[performance]" 2>/dev/null; then
        print_success "性能测试 JSON 报告生成成功"
    fi
}

# 生成 HTML 报告
generate_html_report() {
    print_info "生成 HTML 报告..."

    local python_script="$BUILD_DIR/generate_html_report.py"

    if [ -f "$python_script" ]; then
        if command -v python3 &>/dev/null; then
            if python3 "$python_script"; then
                print_success "HTML 报告生成成功"
                print_info "HTML 报告位置: $REPORTS_DIR/test-report.html"
            else
                print_warning "HTML 报告生成失败"
            fi
        else
            print_warning "Python3 未找到，跳过 HTML 报告生成"
        fi
    else
        print_warning "HTML 报告生成脚本未找到，跳过 HTML 报告生成"
    fi
}

# 生成测试覆盖率报告
generate_coverage_report() {
    print_info "检查测试覆盖率功能..."

    if command -v gcov &>/dev/null && command -v lcov &>/dev/null; then
        print_info "生成测试覆盖率报告..."

        cd "$BUILD_DIR"

        # 运行测试以生成覆盖率数据
        "$TEST_EXECUTABLE" --reporter console >/dev/null 2>&1 || true

        # 收集覆盖率数据
        if lcov --capture --directory . --output-file "$REPORTS_DIR/coverage.info" --quiet 2>/dev/null; then
            # 生成 HTML 覆盖率报告
            if genhtml "$REPORTS_DIR/coverage.info" --output-directory "$REPORTS_DIR/coverage-html" --quiet --show-details --legend 2>/dev/null; then
                print_success "覆盖率报告生成成功"
                print_info "覆盖率 HTML 报告: $REPORTS_DIR/coverage-html/index.html"
            else
                print_warning "HTML 覆盖率报告生成失败"
            fi
        else
            print_warning "覆盖率数据收集失败"
        fi
    else
        print_warning "缺少覆盖率工具 (gcov/lcov)，跳过覆盖率报告"
    fi
}

# 打开报告
open_reports() {
    print_info "报告文件列表:"

    if [ -d "$REPORTS_DIR" ]; then
        find "$REPORTS_DIR" -name "*.xml" -o -name "*.json" -o -name "*.tap" -o -name "*.txt" -o -name "*.html" | sort | while read -r file; do
            local size
            size=$(du -h "$file" | cut -f1)
            echo "  - $(basename "$file") (${size})"
        done

        echo ""
        print_info "报告目录: $REPORTS_DIR"

        # 尝试在浏览器中打开 HTML 报告
        if [ -f "$REPORTS_DIR/test-report.html" ]; then
            print_info "尝试在浏览器中打开 HTML 报告..."
            if command -v xdg-open &>/dev/null; then
                xdg-open "$REPORTS_DIR/test-report.html" 2>/dev/null || true
            elif command -v open &>/dev/null; then
                open "$REPORTS_DIR/test-report.html" 2>/dev/null || true
            fi
        fi

        # 如果有覆盖率报告也尝试打开
        if [ -f "$REPORTS_DIR/coverage-html/index.html" ]; then
            print_info "覆盖率报告: $REPORTS_DIR/coverage-html/index.html"
        fi
    fi
}

# 清理旧报告
clean_reports() {
    if [ -d "$REPORTS_DIR" ]; then
        print_info "清理旧报告..."
        rm -rf "${REPORTS_DIR:?}"/*
        print_success "旧报告已清理"
    fi
}

# 显示帮助信息
show_help() {
    echo "Hert 测试报告生成器"
    echo ""
    echo "用法: $0 [选项]"
    echo ""
    echo "选项:"
    echo "  -h, --help          显示此帮助信息"
    echo "  -c, --clean         清理旧报告"
    echo "  -b, --benchmark     只运行性能基准测试"
    echo "  -C, --coverage      生成测试覆盖率报告"
    echo "  --no-html          跳过 HTML 报告生成"
    echo "  --no-open          不自动打开报告"
    echo ""
    echo "示例:"
    echo "  $0                 # 运行所有测试并生成所有报告"
    echo "  $0 --clean         # 清理旧报告后运行测试"
    echo "  $0 --benchmark     # 只运行性能测试"
    echo "  $0 --coverage      # 包含覆盖率报告"
}

# 主函数
main() {
    local clean_first=false
    local benchmark_only=false
    local include_coverage=false
    local generate_html=true
    local auto_open=true

    # 解析命令行参数
    while [[ $# -gt 0 ]]; do
        case $1 in
        -h | --help)
            show_help
            exit 0
            ;;
        -c | --clean)
            clean_first=true
            shift
            ;;
        -b | --benchmark)
            benchmark_only=true
            shift
            ;;
        -C | --coverage)
            include_coverage=true
            shift
            ;;
        --no-html)
            generate_html=false
            shift
            ;;
        --no-open)
            auto_open=false
            shift
            ;;
        *)
            print_error "未知选项: $1"
            show_help
            exit 1
            ;;
        esac
    done

    print_info "Hert 测试报告生成器启动"
    echo ""

    # 执行流程
    check_dependencies
    setup_directories

    if [ "$clean_first" = true ]; then
        clean_reports
    fi

    if [ "$benchmark_only" = true ]; then
        run_benchmarks
    else
        run_tests_with_reports
        run_benchmarks
    fi

    if [ "$generate_html" = true ]; then
        generate_html_report
    fi

    if [ "$include_coverage" = true ]; then
        generate_coverage_report
    fi

    echo ""
    print_success "所有报告生成完成！"
    echo ""

    if [ "$auto_open" = true ]; then
        open_reports
    fi
}

# 运行主函数
main "$@"
