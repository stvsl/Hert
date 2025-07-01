#!/bin/bash

# Test Reports Demo Script
# 演示和验证测试报告功能

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
PROJECT_ROOT="${SCRIPT_DIR}/.."
BUILD_DIR="${PROJECT_ROOT}/build/dev"

# 颜色定义
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}   Hert 测试报告系统演示${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# 检查项目是否已构建
if [ ! -f "${BUILD_DIR}/test/Hert_test" ]; then
    echo -e "${YELLOW}项目尚未构建，正在构建...${NC}"
    cd "$PROJECT_ROOT"

    if [ ! -f "build/dev/CMakeCache.txt" ]; then
        echo -e "${BLUE}配置 CMake...${NC}"
        cmake --preset=dev -DENABLE_HTML_REPORTS=ON -DENABLE_COVERAGE_REPORTS=ON
    fi

    echo -e "${BLUE}构建项目...${NC}"
    cmake --build build/dev
    echo -e "${GREEN}✓ 项目构建完成${NC}"
    echo ""
fi

cd "$BUILD_DIR"

echo -e "${BLUE}1. 运行基本测试...${NC}"
if ./test/Hert_test --reporter console; then
    echo -e "${GREEN}✓ 基本测试通过${NC}"
else
    echo -e "${RED}✗ 基本测试失败${NC}"
fi
echo ""

echo -e "${BLUE}2. 生成 JUnit XML 报告...${NC}"
if ./test/Hert_test --reporter junit --out test-reports/demo-junit.xml 2>/dev/null; then
    echo -e "${GREEN}✓ JUnit XML 报告生成成功${NC}"
    echo "   文件位置: test-reports/demo-junit.xml"
else
    echo -e "${RED}✗ JUnit XML 报告生成失败${NC}"
fi
echo ""

echo -e "${BLUE}3. 生成 JSON 报告...${NC}"
if ./test/Hert_test --reporter json --out test-reports/demo-results.json 2>/dev/null; then
    echo -e "${GREEN}✓ JSON 报告生成成功${NC}"
    echo "   文件位置: test-reports/demo-results.json"

    # 显示 JSON 报告的简要信息
    if command -v jq &>/dev/null; then
        echo "   测试统计信息:"
        jq -r '.summary | "     总计: \(.total // 0) | 通过: \(.passed // 0) | 失败: \(.failed // 0)"' test-reports/demo-results.json 2>/dev/null || echo "     无法解析统计信息"
    fi
else
    echo -e "${RED}✗ JSON 报告生成失败${NC}"
fi
echo ""

echo -e "${BLUE}4. 生成性能基准测试报告...${NC}"
if ./test/Hert_test --reporter console "[performance]" >test-reports/demo-benchmark.txt 2>&1; then
    echo -e "${GREEN}✓ 性能基准测试完成${NC}"
    echo "   报告位置: test-reports/demo-benchmark.txt"

    # 显示性能测试摘要
    if grep -q "test cases" test-reports/demo-benchmark.txt; then
        echo "   性能测试摘要:"
        grep "test cases\|assertions" test-reports/demo-benchmark.txt | head -2 | sed 's/^/     /'
    fi
else
    echo -e "${YELLOW}⚠ 性能基准测试未找到或失败${NC}"
fi
echo ""

echo -e "${BLUE}5. 测试 HTML 报告生成...${NC}"
if [ -f "generate_html_report.py" ] && command -v python3 &>/dev/null; then
    if python3 generate_html_report.py 2>/dev/null; then
        echo -e "${GREEN}✓ HTML 报告生成成功${NC}"
        echo "   文件位置: test-reports/test-report.html"

        # 检查 HTML 文件大小
        if [ -f "test-reports/test-report.html" ]; then
            size=$(du -h test-reports/test-report.html | cut -f1)
            echo "   文件大小: ${size}"
        fi
    else
        echo -e "${RED}✗ HTML 报告生成失败${NC}"
    fi
else
    echo -e "${YELLOW}⚠ Python3 未找到或脚本不存在，跳过 HTML 报告${NC}"
fi
echo ""

echo -e "${BLUE}6. 检查报告文件...${NC}"
if [ -d "test-reports" ]; then
    echo "   生成的报告文件:"
    find test-reports -name "*.xml" -o -name "*.json" -o -name "*.html" -o -name "*.txt" | sort | while read -r file; do
        if [ -f "$file" ]; then
            size=$(du -h "$file" | cut -f1)
            echo "     - $(basename "$file") (${size})"
        fi
    done
else
    echo -e "${RED}✗ 报告目录不存在${NC}"
fi
echo ""

echo -e "${BLUE}7. 验证 CMake 目标...${NC}"
echo "   可用的测试报告 CMake 目标:"

available_targets=()

# 检查各个目标是否存在
for target in test-junit test-xml test-json test-html-report test-benchmark test-all-reports; do
    if make -n "$target" &>/dev/null; then
        available_targets+=("$target")
        echo -e "     ${GREEN}✓${NC} $target"
    else
        echo -e "     ${RED}✗${NC} $target (不可用)"
    fi
done

if [ ${#available_targets[@]} -gt 0 ]; then
    echo ""
    echo -e "${BLUE}测试一个 CMake 目标...${NC}"
    if make "${available_targets[0]}" &>/dev/null; then
        echo -e "${GREEN}✓ CMake 目标 '${available_targets[0]}' 执行成功${NC}"
    else
        echo -e "${RED}✗ CMake 目标 '${available_targets[0]}' 执行失败${NC}"
    fi
fi
echo ""

echo -e "${BLUE}8. 检查脚本工具...${NC}"
script_path="${PROJECT_ROOT}/scripts/generate_test_reports.sh"
if [ -f "$script_path" ] && [ -x "$script_path" ]; then
    echo -e "${GREEN}✓ 测试报告生成脚本存在且可执行${NC}"
    echo "   位置: $script_path"

    # 测试脚本的帮助功能
    if "$script_path" --help &>/dev/null; then
        echo -e "${GREEN}✓ 脚本帮助功能正常${NC}"
    else
        echo -e "${YELLOW}⚠ 脚本帮助功能可能有问题${NC}"
    fi
else
    echo -e "${RED}✗ 测试报告生成脚本不存在或无执行权限${NC}"
fi
echo ""

echo -e "${BLUE}========================================${NC}"
echo -e "${GREEN}   演示完成！${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

echo "📊 测试报告功能摘要:"
echo "   ✅ 支持多种报告格式 (JUnit XML, JSON, HTML)"
echo "   ✅ 性能基准测试"
echo "   ✅ CMake 集成"
echo "   ✅ 便捷脚本工具"
echo ""

echo "🚀 下一步:"
echo "   1. 使用 './scripts/generate_test_reports.sh' 生成完整报告"
echo "   2. 使用 'make test-all-reports' 生成所有格式报告"
echo "   3. 查看 'test-reports/' 目录中的报告文件"
echo "   4. 在浏览器中打开 HTML 报告查看详细结果"
echo ""

if [ -f "test-reports/test-report.html" ]; then
    echo "🌐 HTML 报告已生成，您可以在浏览器中打开查看:"
    echo "   file://$PWD/test-reports/test-report.html"
fi
