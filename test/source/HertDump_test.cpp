#include <chrono>
#include <functional>
#include <string>

#include "Hert/HertDump.hpp"

#include <catch2/catch_test_macros.hpp>

#include "Hert/Hert.hpp"

// ========== HertDump 类测试 ==========

TEST_CASE("HertDump initialization", "[HertDump][init]")
{
  SECTION("Can initialize HertDump")
  {
    REQUIRE_NOTHROW(HertDump::init());
  }

  SECTION("Can initialize HertDump with core directory")
  {
    const std::string testCoreDir = "/tmp/hert_test_cores";
    REQUIRE_NOTHROW(HertDump::init(testCoreDir));
  }

  SECTION("Multiple init calls are safe")
  {
    REQUIRE_NOTHROW(HertDump::init());
    REQUIRE_NOTHROW(HertDump::init());
    REQUIRE_NOTHROW(HertDump::init("/tmp/test"));
  }
}

TEST_CASE("HertDump core dump directory management", "[HertDump][coredir]")
{
  SECTION("Can set core dump directory")
  {
    const std::string testDir = "/tmp/hert_test_dump";
    REQUIRE_NOTHROW(HertDump::setCoreDumpDir(testDir));
  }

  SECTION("Can set empty core dump directory")
  {
    REQUIRE_NOTHROW(HertDump::setCoreDumpDir(""));
  }

  SECTION("Can handle invalid directory paths gracefully")
  {
    // 测试设置不存在的深层目录路径
    const std::string deepPath = "/tmp/very/deep/nonexistent/path/for/cores";
    REQUIRE_NOTHROW(HertDump::setCoreDumpDir(deepPath));
  }
}

TEST_CASE("HertDump crash callback", "[HertDump][callback]")
{
  SECTION("Can set and change crash callback")
  {
    bool callback1Called = false;
    bool callback2Called = false;

    auto callback1 = [&callback1Called]() { callback1Called = true; };
    auto callback2 = [&callback2Called]() { callback2Called = true; };

    REQUIRE_NOTHROW(HertDump::setCrashCallback(callback1));
    REQUIRE_NOTHROW(HertDump::setCrashCallback(callback2));

    // 注意：我们不能直接测试崩溃处理，因为那会终止程序
    // 这里只测试设置回调函数本身不会出错
  }

  SECTION("Can set null crash callback")
  {
    REQUIRE_NOTHROW(HertDump::setCrashCallback(nullptr));
  }

  SECTION("Can set lambda callback")
  {
    int callbackCounter = 0;
    auto callback = [&callbackCounter]() { callbackCounter++; };

    REQUIRE_NOTHROW(HertDump::setCrashCallback(callback));
  }

  SECTION("Can set function pointer callback")
  {
    auto simpleCallback = []()
    {
      // 简单的回调函数
    };

    REQUIRE_NOTHROW(HertDump::setCrashCallback(simpleCallback));
  }
}

TEST_CASE("HertDump stack trace printing", "[HertDump][stacktrace]")
{
  SECTION("printStacktrace does not throw")
  {
    // 这个测试确保 printStacktrace 能正常调用而不崩溃
    // 实际的输出会打印到 stderr，在单元测试中我们主要关心不会异常
    REQUIRE_NOTHROW(HertDump::printStacktrace());
  }

  SECTION("Multiple stacktrace calls are safe")
  {
    REQUIRE_NOTHROW(HertDump::printStacktrace());
    REQUIRE_NOTHROW(HertDump::printStacktrace());
    REQUIRE_NOTHROW(HertDump::printStacktrace());
  }
}

// ========== 集成测试 ==========

TEST_CASE("Integration test - Library components work together",
          "[integration]")
{
  SECTION("Can use Hert version info with HertDump")
  {
    // 初始化 HertDump
    HertDump::init("/tmp/integration_test");

    // 获取版本信息
    const char* version = Hert::version();
    REQUIRE(version != nullptr);

    // 设置一个简单的崩溃回调
    bool callbackSet = false;
    HertDump::setCrashCallback(
        [&callbackSet, version]()
        {
          // 在崩溃回调中使用版本信息
          std::string info = "Crash in version: ";
          info += version;
          callbackSet = true;
        });

    // 测试通过，说明组件可以协同工作
    SUCCEED("Components integrated successfully");
  }

  SECTION("Full initialization sequence")
  {
    // 模拟完整的初始化序列
    const char* version = Hert::version();
    REQUIRE(version != nullptr);

    // 设置核心转储目录
    HertDump::setCoreDumpDir("/tmp/full_init_test");

    // 设置崩溃回调
    HertDump::setCrashCallback(
        []()
        {
          // 崩溃时的清理工作
        });

    // 初始化转储系统
    HertDump::init("/tmp/full_init_test");

    // 测试堆栈追踪
    REQUIRE_NOTHROW(HertDump::printStacktrace());

    SUCCEED("Full initialization sequence completed");
  }
}

// ========== 性能测试 ==========

TEST_CASE("Performance tests", "[performance]")
{
  SECTION("Hert::version() performance")
  {
    const int iterations = 10000;
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; ++i) {
      [[maybe_unused]] const char* version = Hert::version();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    // 版本获取应该很快，这里设置一个合理的上限（比如每次调用不超过 10 微秒）
    const auto maxTimePerCall = static_cast<long>(iterations) * 10L;
    REQUIRE(duration.count() < maxTimePerCall);

    INFO("Average time per call: " << duration.count() / iterations
                                   << " microseconds");
  }

  SECTION("HertDump initialization performance")
  {
    const int iterations = 100;
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; ++i) {
      HertDump::init("/tmp/perf_test_" + std::to_string(i));
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // 初始化应该相对较快
    REQUIRE(duration.count() < 1000);  // 小于1秒

    INFO("Total initialization time: " << duration.count() << " milliseconds");
  }
}

// ========== 边界测试 ==========

TEST_CASE("Boundary and edge cases", "[boundary]")
{
  SECTION("HertDump with very long directory path")
  {
    // 测试非常长的目录路径
    std::string longPath = "/tmp/";
    for (int i = 0; i < 10; ++i) {
      longPath += "very_long_directory_name_that_might_cause_issues/";
    }

    REQUIRE_NOTHROW(HertDump::setCoreDumpDir(longPath));
    REQUIRE_NOTHROW(HertDump::init(longPath));
  }

  SECTION("HertDump with special characters in path")
  {
    const std::string specialPath = "/tmp/test_dir_with_特殊字符_and_spaces";
    REQUIRE_NOTHROW(HertDump::setCoreDumpDir(specialPath));
    REQUIRE_NOTHROW(HertDump::init(specialPath));
  }

  SECTION("Version string properties")
  {
    const char* version = Hert::version();
    std::string versionStr(version);

    // 版本字符串不应该为空
    REQUIRE_FALSE(versionStr.empty());

    // 版本字符串不应该包含空字符
    REQUIRE(versionStr.find_first_of('\0') == std::string::npos);

    // 版本字符串长度应该合理（不会太长）
    REQUIRE(versionStr.length() < 100);

    // 版本字符串应该是可打印字符
    bool allPrintable = true;
    for (char c : versionStr) {
      if (!std::isprint(c)) {
        allPrintable = false;
        break;
      }
    }
    REQUIRE(allPrintable == true);
  }
}
