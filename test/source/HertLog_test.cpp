#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <thread>

#include "Hert/HertLog.hpp"

#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace Hert;

// ========== HertLog 类测试 ==========

TEST_CASE("HertLog基本功能测试", "[HertLog][basic]")
{
  SECTION("初始化和关闭")
  {
    REQUIRE_FALSE(HertLog::is_initialized());

    LogSinkConfig config;
    config.console_enabled = true;
    config.file_enabled = false;  // 测试时不输出文件
    config.console_level = LogLevel::DEBUG;

    REQUIRE_NOTHROW(HertLog::initialize(config));
    REQUIRE(HertLog::is_initialized());

    REQUIRE_NOTHROW(HertLog::shutdown());
    REQUIRE_FALSE(HertLog::is_initialized());
  }

  SECTION("重复初始化应该安全")
  {
    LogSinkConfig config;
    config.console_enabled = true;
    config.file_enabled = false;

    REQUIRE_NOTHROW(HertLog::initialize(config));
    REQUIRE_NOTHROW(HertLog::initialize(config));  // 重复初始化

    HertLog::shutdown();
  }
}

TEST_CASE("HertLog日志级别测试", "[HertLog][levels]")
{
  LogSinkConfig config;
  config.console_enabled = true;
  config.file_enabled = false;
  config.console_level = LogLevel::DEBUG;

  HertLog::initialize(config);

  SECTION("各种日志级别输出")
  {
    REQUIRE_NOTHROW(HertLog::trace("这是一条trace消息"));
    REQUIRE_NOTHROW(HertLog::debug("这是一条debug消息"));
    REQUIRE_NOTHROW(HertLog::info("这是一条info消息"));
    REQUIRE_NOTHROW(HertLog::warn("这是一条warn消息"));
    REQUIRE_NOTHROW(HertLog::error("这是一条error消息"));
  }

  SECTION("格式化输出")
  {
    REQUIRE_NOTHROW(
        HertLog::info("格式化测试: 数字={}, 字符串={}", 42, "hello"));
    REQUIRE_NOTHROW(HertLog::debug("调试信息: {}", "test"));
    REQUIRE_NOTHROW(HertLog::warn("警告: {} + {} = {}", 1, 2, 3));
  }

  SECTION("带位置信息的日志")
  {
    REQUIRE_NOTHROW(HERT_LOG_INFO("带位置信息的info消息"));
    REQUIRE_NOTHROW(HERT_LOG_WARN("带位置信息的warn消息，参数: {}", 123));
    REQUIRE_NOTHROW(HERT_LOG_DEBUG("调试消息"));
    REQUIRE_NOTHROW(HERT_LOG_ERROR("错误消息"));
  }

  HertLog::shutdown();
}

TEST_CASE("HertLog文件输出测试", "[HertLog][file]")
{
  const std::string test_log_file = "test_hert_unit.log";

  // 清理之前的测试文件
  if (std::filesystem::exists(test_log_file)) {
    std::filesystem::remove(test_log_file);
  }

  LogSinkConfig config;
  config.console_enabled = false;
  config.file_enabled = true;
  config.file_path = test_log_file;
  config.file_level = LogLevel::DEBUG;

  HertLog::initialize(config);

  SECTION("文件日志输出")
  {
    HertLog::info("测试文件输出");
    HertLog::debug("调试信息写入文件");
    HertLog::warn("警告信息");

    // 刷新确保写入
    HertLog::flush();

    // 等待异步写入完成
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // 检查文件是否存在
    REQUIRE(std::filesystem::exists(test_log_file));

    // 检查文件内容
    std::ifstream file(test_log_file);
    REQUIRE(file.is_open());

    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();

    REQUIRE(content.find("测试文件输出") != std::string::npos);
    REQUIRE(content.find("调试信息写入文件") != std::string::npos);
    REQUIRE(content.find("警告信息") != std::string::npos);
  }

  HertLog::shutdown();

  // 清理测试文件
  if (std::filesystem::exists(test_log_file)) {
    std::filesystem::remove(test_log_file);
  }
}

TEST_CASE("HertLog自定义处理器测试", "[HertLog][handler]")
{
  LogSinkConfig config;
  config.console_enabled = false;
  config.file_enabled = false;

  HertLog::initialize(config);

  SECTION("添加和使用自定义处理器")
  {
    std::vector<std::string> captured_messages;
    std::mutex captured_mutex;

    HertLog::addHandler(
        [&captured_messages, &captured_mutex](LogLevel level,
                                              const std::string& message,
                                              const std::string& file,
                                              int line,
                                              const std::string& function)
        {
          std::lock_guard<std::mutex> lock(captured_mutex);
          captured_messages.push_back(message);
        });

    HertLog::info("测试消息1");
    HertLog::warn("测试消息2");
    HertLog::error("测试消息3");

    // 等待处理器处理
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    std::lock_guard<std::mutex> lock(captured_mutex);
    REQUIRE(captured_messages.size() == 3);
    REQUIRE(captured_messages[0] == "测试消息1");
    REQUIRE(captured_messages[1] == "测试消息2");
    REQUIRE(captured_messages[2] == "测试消息3");

    HertLog::clearHandlers();
  }

  HertLog::shutdown();
}

TEST_CASE("HertLog多线程安全测试", "[HertLog][multithread]")
{
  LogSinkConfig config;
  config.console_enabled = true;
  config.file_enabled = false;
  config.console_level = LogLevel::INFO;

  HertLog::initialize(config);

  SECTION("多线程并发日志")
  {
    const int num_threads = 4;
    const int messages_per_thread = 10;
    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; ++i) {
      threads.emplace_back(
          [i, messages_per_thread]()
          {
            for (int j = 0; j < messages_per_thread; ++j) {
              HertLog::info("线程{} - 消息{}", i, j);
              std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
          });
    }

    for (auto& t : threads) {
      t.join();
    }

    // 如果程序没有崩溃，则认为线程安全测试通过
    REQUIRE(true);
  }

  HertLog::shutdown();
}

TEST_CASE("HertLog级别控制测试", "[HertLog][level_control]")
{
  LogSinkConfig config;
  config.console_enabled = false;
  config.file_enabled = false;

  HertLog::initialize(config);

  SECTION("设置日志级别")
  {
    // 设置为WARN级别，DEBUG和INFO应该被过滤
    HertLog::setLevel(LogLevel::WARN);

    // 这些调用应该不会出错，但可能不会输出
    REQUIRE_NOTHROW(HertLog::debug("这条debug消息应该被过滤"));
    REQUIRE_NOTHROW(HertLog::info("这条info消息应该被过滤"));
    REQUIRE_NOTHROW(HertLog::warn("这条warn消息应该显示"));
    REQUIRE_NOTHROW(HertLog::error("这条error消息应该显示"));

    // 恢复级别
    HertLog::setLevel(LogLevel::DEBUG);
    REQUIRE_NOTHROW(HertLog::debug("现在debug消息应该显示"));
  }

  HertLog::shutdown();
}

TEST_CASE("HertLog标准输出重定向测试", "[HertLog][std_redirect]")
{
  LogSinkConfig config;
  config.console_enabled = false;
  config.file_enabled = false;

  HertLog::initialize(config);

  SECTION("启用和禁用标准输出重定向")
  {
    // 测试重定向功能
    REQUIRE_NOTHROW(HertLog::enableStdRedirect());

    // 这些应该被重定向到日志系统
    std::cout << "重定向的cout消息" << std::endl;
    std::cerr << "重定向的cerr消息" << std::endl;

    REQUIRE_NOTHROW(HertLog::disableStdRedirect());

    // 现在应该正常输出
    std::cout << "正常的cout消息" << std::endl;
  }

  HertLog::shutdown();
}

#ifdef QT_CORE_LIB
TEST_CASE("HertLog Qt集成测试", "[HertLog][qt]")
{
  LogSinkConfig config;
  config.console_enabled = true;
  config.file_enabled = false;

  HertLog::initialize(config);

  SECTION("Qt日志重定向")
  {
    REQUIRE_NOTHROW(HertLog::enableQtLogRedirect());

    // 测试Qt日志输出（如果可用）
    qDebug() << "Qt Debug消息";
    qInfo() << "Qt Info消息";
    qWarning() << "Qt Warning消息";
    qCritical() << "Qt Critical消息";

    REQUIRE_NOTHROW(HertLog::disableQtLogRedirect());
  }

  HertLog::shutdown();
}
#endif

TEST_CASE("HertLog错误处理测试", "[HertLog][error_handling]")
{
  SECTION("未初始化时的安全调用")
  {
    // 确保未初始化
    if (HertLog::is_initialized()) {
      HertLog::shutdown();
    }

    // 这些调用应该安全，不会崩溃
    REQUIRE_NOTHROW(HertLog::info("未初始化时的消息"));
    REQUIRE_NOTHROW(HertLog::error("未初始化时的错误"));
    REQUIRE_NOTHROW(HertLog::flush());
    REQUIRE_NOTHROW(HertLog::setLevel(LogLevel::DEBUG));
  }

  SECTION("异常处理器安全")
  {
    LogSinkConfig config;
    config.console_enabled = false;
    config.file_enabled = false;

    HertLog::initialize(config);

    // 添加会抛异常的处理器
    HertLog::addHandler([](LogLevel level,
                           const std::string& message,
                           const std::string& file,
                           int line,
                           const std::string& function)
                        { throw std::runtime_error("处理器异常"); });

    // 日志调用应该不会因为处理器异常而崩溃
    REQUIRE_NOTHROW(HertLog::info("测试异常处理"));

    HertLog::clearHandlers();
    HertLog::shutdown();
  }
}

TEST_CASE("HertLog模式设置测试", "[HertLog][pattern]")
{
  LogSinkConfig config;
  config.console_enabled = true;
  config.file_enabled = false;

  HertLog::initialize(config);

  SECTION("自定义日志模式")
  {
    REQUIRE_NOTHROW(HertLog::setPattern("[%H:%M:%S] [%l] %v"));
    REQUIRE_NOTHROW(HertLog::info("自定义模式测试"));

    // 恢复默认模式
    REQUIRE_NOTHROW(HertLog::setPattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v"));
    REQUIRE_NOTHROW(HertLog::info("恢复默认模式测试"));
  }

  HertLog::shutdown();
}
