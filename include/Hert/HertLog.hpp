#pragma once

#include <atomic>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>

#include <fmt/format.h>
#include <spdlog/async.h>
#include <spdlog/fmt/fmt.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#ifdef QT_CORE_LIB
#  include <QDebug>
#  include <QtLogging>
#endif

namespace Hert
{

/**
 * @brief 日志级别枚举
 */
enum class LogLevel : std::uint8_t
{
  TRACE = 0,
  DEBUG = 1,
  INFO = 2,
  WARN = 3,
  ERROR = 4,
  CRITICAL = 5,
  OFF = 6
};

/**
 * @brief 日志输出目标配置
 */
struct LogSinkConfig
{
  bool console_enabled = true;  // 是否启用控制台输出
  bool file_enabled = false;  // 是否启用文件输出
  std::string file_path = "hert.log";  // 文件路径
  size_t max_file_size = 1024UL * 1024UL * 10UL;  // 最大文件大小(10MB)
  size_t max_files = 3;  // 最大文件数量
  LogLevel console_level = LogLevel::INFO;  // 控制台日志级别
  LogLevel file_level = LogLevel::DEBUG;  // 文件日志级别
};

/**
 * @brief 自定义日志处理器类型
 */
using LogHandler = std::function<void(LogLevel level,
                                      const std::string& message,
                                      const std::string& file,
                                      int line,
                                      const std::string& function)>;

/**
 * @brief 高性能日志系统 - 类似于log4j的功能
 *
 * 特性：
 * - 线程安全的异步日志
 * - 支持控制台和文件输出
 * - 自定义格式和处理器
 * - Qt日志系统集成
 * - 标准输出重载
 */
class HertLog
{
public:
  /**
   * @brief 初始化日志系统
   * @param config 日志配置
   */
  static void initialize(const LogSinkConfig& config = LogSinkConfig {});

  /**
   * @brief 设置全局日志级别
   * @param level 日志级别
   */
  static void setLevel(LogLevel level);

  /**
   * @brief 设置日志模式
   * @param pattern 日志格式模式
   */
  static void setPattern(const std::string& pattern);

  /**
   * @brief 添加自定义日志处理器
   * @param handler 处理器函数
   */
  static void addHandler(const LogHandler& handler);

  /**
   * @brief 清除所有自定义处理器
   */
  static void clearHandlers();

  /**
   * @brief 刷新所有日志输出
   */
  static void flush();

  /**
   * @brief 关闭日志系统
   */
  static void shutdown();

  // ============ 主要日志接口 ============

  /**
   * @brief 输出INFO级别日志
   */
  template<typename... Args>
  static void info(fmt::format_string<Args...> format, Args&&... args)
  {
    log_internal(LogLevel::INFO, format, std::forward<Args>(args)...);
  }

  /**
   * @brief 输出ERROR级别日志
   */
  template<typename... Args>
  static void error(fmt::format_string<Args...> format, Args&&... args)
  {
    log_internal(LogLevel::ERROR, format, std::forward<Args>(args)...);
  }

  /**
   * @brief 输出WARN级别日志
   */
  template<typename... Args>
  static void warn(fmt::format_string<Args...> format, Args&&... args)
  {
    log_internal(LogLevel::WARN, format, std::forward<Args>(args)...);
  }

  /**
   * @brief 输出CRITICAL级别日志并退出程序
   */
  template<typename... Args>
  static void panic(fmt::format_string<Args...> format, Args&&... args)
  {
    log_internal(LogLevel::CRITICAL, format, std::forward<Args>(args)...);
    flush();
    std::abort();  // 模拟panic行为
  }

  /**
   * @brief 输出DEBUG级别日志
   */
  template<typename... Args>
  static void debug(fmt::format_string<Args...> format, Args&&... args)
  {
    log_internal(LogLevel::DEBUG, format, std::forward<Args>(args)...);
  }

  /**
   * @brief 输出TRACE级别日志
   */
  template<typename... Args>
  static void trace(fmt::format_string<Args...> format, Args&&... args)
  {
    log_internal(LogLevel::TRACE, format, std::forward<Args>(args)...);
  }

  // ============ 带位置信息的日志宏 ============

#define HERT_LOG_INFO(format, ...) \
  Hert::HertLog::log_with_location(Hert::LogLevel::INFO, \
                                   __FILE__, \
                                   __LINE__, \
                                   __FUNCTION__, \
                                   format, \
                                   ##__VA_ARGS__)

#define HERT_LOG_ERROR(format, ...) \
  Hert::HertLog::log_with_location(Hert::LogLevel::ERROR, \
                                   __FILE__, \
                                   __LINE__, \
                                   __FUNCTION__, \
                                   format, \
                                   ##__VA_ARGS__)

#define HERT_LOG_WARN(format, ...) \
  Hert::HertLog::log_with_location(Hert::LogLevel::WARN, \
                                   __FILE__, \
                                   __LINE__, \
                                   __FUNCTION__, \
                                   format, \
                                   ##__VA_ARGS__)

#define HERT_LOG_DEBUG(format, ...) \
  Hert::HertLog::log_with_location(Hert::LogLevel::DEBUG, \
                                   __FILE__, \
                                   __LINE__, \
                                   __FUNCTION__, \
                                   format, \
                                   ##__VA_ARGS__)

#define HERT_LOG_PANIC(format, ...) \
  Hert::HertLog::log_with_location_panic( \
      __FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__)

  /**
   * @brief 带位置信息的日志输出
   */
  template<typename... Args>
  static void log_with_location(LogLevel level,
                                const char* file,
                                int line,
                                const char* function,
                                fmt::format_string<Args...> format,
                                Args&&... args)
  {
    if (!is_initialized() || !should_log(level)) {
      return;
    }

    try {
      std::string message = fmt::format(format, std::forward<Args>(args)...);
      log_with_location_internal(level, file, line, function, message);
    } catch (const std::exception& e) {
      // 格式化错误时的安全处理
      log_with_location_internal(LogLevel::ERROR,
                                 file,
                                 line,
                                 function,
                                 "Log format error: " + std::string(e.what()));
    }
  }

  /**
   * @brief 带位置信息的panic日志
   */
  template<typename... Args>
  static void log_with_location_panic(const char* file,
                                      int line,
                                      const char* function,
                                      fmt::format_string<Args...> format,
                                      Args&&... args)
  {
    log_with_location(LogLevel::CRITICAL,
                      file,
                      line,
                      function,
                      format,
                      std::forward<Args>(args)...);
    flush();
    std::abort();
  }

#ifdef QT_CORE_LIB
  // ============ Qt日志系统集成 ============

  /**
   * @brief 启用Qt日志重定向
   */
  static void enableQtLogRedirect();

  /**
   * @brief 禁用Qt日志重定向
   */
  static void disableQtLogRedirect();

  /**
   * @brief Qt日志消息处理器
   */
  static void qtMessageHandler(QtMsgType type,
                               const QMessageLogContext& context,
                               const QString& msg);
#endif

  // ============ 标准输出重载 ============

  /**
   * @brief 启用标准输出重定向
   */
  static void enableStdRedirect();

  /**
   * @brief 禁用标准输出重定向
   */
  static void disableStdRedirect();

  /**
   * @brief 获取是否已初始化
   */
  static bool is_initialized() { return s_initialized.load(); }

private:
  // 禁止实例化
  HertLog() = delete;
  ~HertLog() = delete;
  HertLog(const HertLog&) = delete;
  HertLog& operator=(const HertLog&) = delete;
  HertLog(HertLog&&) = delete;
  HertLog& operator=(HertLog&&) = delete;

  // 内部实现方法
  template<typename... Args>
  static void log_internal(LogLevel level,
                           fmt::format_string<Args...> format,
                           Args&&... args)
  {
    if (!is_initialized() || !should_log(level)) {
      return;
    }

    try {
      std::string message = fmt::format(format, std::forward<Args>(args)...);
      log_message_internal(level, message);
    } catch (const std::exception& e) {
      // 格式化错误时的安全处理
      log_message_internal(LogLevel::ERROR,
                           "Log format error: " + std::string(e.what()));
    }
  }

  static void log_message_internal(LogLevel level, const std::string& message);
  static void log_with_location_internal(LogLevel level,
                                         const char* file,
                                         int line,
                                         const char* function,
                                         const std::string& message);
  static bool should_log(LogLevel level);
  static spdlog::level::level_enum convert_log_level(LogLevel level);
  static void call_custom_handlers(LogLevel level,
                                   const std::string& message,
                                   const std::string& file,
                                   int line,
                                   const std::string& function);

  // 静态成员变量
  static std::shared_ptr<spdlog::logger> s_logger;
  static std::vector<LogHandler> s_handlers;
  static std::mutex s_handlers_mutex;
  static std::atomic<bool> s_initialized;
  static std::atomic<LogLevel> s_current_level;

#ifdef QT_CORE_LIB
  static QtMessageHandler s_original_qt_handler;
  static std::atomic<bool> s_qt_redirect_enabled;
#endif

  static std::atomic<bool> s_std_redirect_enabled;
  static std::streambuf* s_original_cout_buf;
  static std::streambuf* s_original_cerr_buf;

  // 自定义流缓冲区用于重定向标准输出
  class LogStreamBuf;
  static std::unique_ptr<LogStreamBuf> s_cout_redirect;
  static std::unique_ptr<LogStreamBuf> s_cerr_redirect;
};

}  // namespace Hert
