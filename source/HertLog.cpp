#include <filesystem>
#include <iostream>

#include "Hert/HertLog.hpp"

#include <spdlog/async.h>
#include <spdlog/common.h>
#include <spdlog/pattern_formatter.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace Hert
{

// ============ 静态成员变量定义 ============

std::shared_ptr<spdlog::logger> HertLog::s_logger = nullptr;
std::vector<LogHandler> HertLog::s_handlers;
std::mutex HertLog::s_handlers_mutex;
std::atomic<bool> HertLog::s_initialized {false};
std::atomic<LogLevel> HertLog::s_current_level {LogLevel::INFO};

namespace
{
// 延迟初始化的配置获取函数
LogSinkConfig& get_config()
{
  static LogSinkConfig config;
  return config;
}
}  // anonymous namespace

#define s_config get_config()

#ifdef QT_CORE_LIB
QtMessageHandler HertLog::s_original_qt_handler = nullptr;
std::atomic<bool> HertLog::s_qt_redirect_enabled {false};
#endif

std::atomic<bool> HertLog::s_std_redirect_enabled {false};
std::streambuf* HertLog::s_original_cout_buf = nullptr;
std::streambuf* HertLog::s_original_cerr_buf = nullptr;
std::unique_ptr<HertLog::LogStreamBuf> HertLog::s_cout_redirect = nullptr;
std::unique_ptr<HertLog::LogStreamBuf> HertLog::s_cerr_redirect = nullptr;

// ============ 自定义流缓冲区实现 ============

class HertLog::LogStreamBuf : public std::streambuf
{
public:
  explicit LogStreamBuf(LogLevel level)
      : m_level(level)
  {
  }

protected:
  int overflow(int c) override
  {
    if (c != EOF) {
      m_buffer += static_cast<char>(c);
      if (c == '\n') {
        flush_buffer();
      }
    }
    return c;
  }

  int sync() override
  {
    flush_buffer();
    return 0;
  }

private:
  void flush_buffer()
  {
    if (!m_buffer.empty() && m_buffer.back() == '\n') {
      m_buffer.pop_back();  // 移除换行符
    }
    if (!m_buffer.empty()) {
      HertLog::log_message_internal(m_level, m_buffer);
      m_buffer.clear();
    }
  }

  LogLevel m_level;
  std::string m_buffer;
};

// ============ 核心实现方法 ============

void HertLog::initialize(const LogSinkConfig& config)
{
  if (s_initialized.load()) {
    return;  // 已经初始化
  }

  s_config = config;

  try {
    // 创建异步日志线程池
    if (!spdlog::get("async_pool")) {
      spdlog::init_thread_pool(8192, 1);  // 队列大小8192，1个后台线程
    }

    std::vector<spdlog::sink_ptr> sinks;

    // 配置控制台输出
    if (config.console_enabled) {
      auto console_sink =
          std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
      console_sink->set_level(convert_log_level(config.console_level));
      console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
      sinks.push_back(console_sink);
    }

    // 配置文件输出
    if (config.file_enabled) {
      // 确保日志目录存在
      std::filesystem::path log_path(config.file_path);
      if (log_path.has_parent_path()) {
        std::filesystem::create_directories(log_path.parent_path());
      }

      auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
          config.file_path, config.max_file_size, config.max_files);
      file_sink->set_level(convert_log_level(config.file_level));
      file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
      sinks.push_back(file_sink);
    }

    // 创建异步日志器
    s_logger = std::make_shared<spdlog::async_logger>(
        "hert_logger",
        sinks.begin(),
        sinks.end(),
        spdlog::thread_pool(),
        spdlog::async_overflow_policy::block);

    s_logger->set_level(
        spdlog::level::trace);  // 设置为最低级别，由sink控制具体级别
    s_logger->flush_on(spdlog::level::err);  // 错误级别自动刷新

    // 注册为默认日志器
    spdlog::set_default_logger(s_logger);

    // 设置全局日志级别为配置中的最低级别
    LogLevel min_level = LogLevel::OFF;
    if (config.console_enabled) {
      min_level = std::min(min_level, config.console_level);
    }
    if (config.file_enabled) {
      min_level = std::min(min_level, config.file_level);
    }
    if (min_level != LogLevel::OFF) {
      s_current_level.store(min_level);
    }

    s_initialized.store(true);

    // 输出初始化成功消息
    info("HertLog initialized successfully");

  } catch (const std::exception& e) {
    std::cerr << "Failed to initialize HertLog: " << e.what() << '\n';
    throw;
  }
}

void HertLog::setLevel(LogLevel level)
{
  s_current_level.store(level);
  if (s_logger) {
    s_logger->set_level(convert_log_level(level));
  }
}

void HertLog::setPattern(const std::string& pattern)
{
  if (s_logger) {
    s_logger->set_pattern(pattern);
  }
}

void HertLog::addHandler(const LogHandler& handler)
{
  std::lock_guard<std::mutex> lock(s_handlers_mutex);
  s_handlers.push_back(handler);
}

void HertLog::clearHandlers()
{
  std::lock_guard<std::mutex> lock(s_handlers_mutex);
  s_handlers.clear();
}

void HertLog::flush()
{
  if (s_logger) {
    s_logger->flush();
  }
}

void HertLog::shutdown()
{
  if (!s_initialized.load()) {
    return;
  }

  info("Shutting down HertLog...");

  // 禁用重定向
  disableStdRedirect();
#ifdef QT_CORE_LIB
  disableQtLogRedirect();
#endif

  // 刷新并关闭日志器
  if (s_logger) {
    s_logger->flush();
    s_logger.reset();
  }

  // 关闭spdlog
  spdlog::shutdown();

  // 清除处理器
  clearHandlers();

  s_initialized.store(false);
}

void HertLog::log_message_internal(LogLevel level, const std::string& message)
{
  if (!should_log(level)) {
    return;
  }

  if (s_logger) {
    switch (level) {
      case LogLevel::TRACE:
        s_logger->trace(message);
        break;
      case LogLevel::DEBUG:
        s_logger->debug(message);
        break;
      case LogLevel::INFO:
        s_logger->info(message);
        break;
      case LogLevel::WARN:
        s_logger->warn(message);
        break;
      case LogLevel::ERROR:
        s_logger->error(message);
        break;
      case LogLevel::CRITICAL:
        s_logger->critical(message);
        break;
      default:
        break;
    }
  }

  // 调用自定义处理器
  call_custom_handlers(level, message, "", 0, "");
}

void HertLog::log_with_location_internal(LogLevel level,
                                         const char* file,
                                         int line,
                                         const char* function,
                                         const std::string& message)
{
  if (!should_log(level)) {
    return;
  }

  if (s_logger) {
    // 构建包含位置信息的消息
    std::string formatted_message;
    if (file && line > 0 && function) {
      // 提取文件名（不包含路径）
      std::string filename = file;
      size_t last_slash = filename.find_last_of("/\\");
      if (last_slash != std::string::npos) {
        filename = filename.substr(last_slash + 1);
      }

      formatted_message =
          fmt::format("[{}:{}] [{}] {}", filename, line, function, message);
    } else {
      formatted_message = message;
    }

    switch (level) {
      case LogLevel::TRACE:
        s_logger->trace(formatted_message);
        break;
      case LogLevel::DEBUG:
        s_logger->debug(formatted_message);
        break;
      case LogLevel::INFO:
        s_logger->info(formatted_message);
        break;
      case LogLevel::WARN:
        s_logger->warn(formatted_message);
        break;
      case LogLevel::ERROR:
        s_logger->error(formatted_message);
        break;
      case LogLevel::CRITICAL:
        s_logger->critical(formatted_message);
        break;
      default:
        break;
    }
  }

  // 调用自定义处理器
  call_custom_handlers(
      level, message, file ? file : "", line, function ? function : "");
}

bool HertLog::should_log(LogLevel level)
{
  return s_initialized.load() && level >= s_current_level.load();
}

spdlog::level::level_enum HertLog::convert_log_level(LogLevel level)
{
  switch (level) {
    case LogLevel::TRACE:
      return spdlog::level::trace;
    case LogLevel::DEBUG:
      return spdlog::level::debug;
    case LogLevel::INFO:
      return spdlog::level::info;
    case LogLevel::WARN:
      return spdlog::level::warn;
    case LogLevel::ERROR:
      return spdlog::level::err;
    case LogLevel::CRITICAL:
      return spdlog::level::critical;
    case LogLevel::OFF:
      return spdlog::level::off;
    default:
      return spdlog::level::info;
  }
}

void HertLog::call_custom_handlers(LogLevel level,
                                   const std::string& message,
                                   const std::string& file,
                                   int line,
                                   const std::string& function)
{
  std::lock_guard<std::mutex> lock(s_handlers_mutex);
  for (const auto& handler : s_handlers) {
    try {
      handler(level, message, file, line, function);
    } catch (const std::exception& e) {
      // 处理器异常时输出到stderr，避免递归
      std::cerr << "Exception in log handler: " << e.what() << '\n';
    } catch (...) {
      std::cerr << "Unknown exception in log handler\n";
    }
  }
}

// ============ Qt日志系统集成 ============

#ifdef QT_CORE_LIB

void HertLog::enableQtLogRedirect()
{
  if (s_qt_redirect_enabled.load()) {
    return;
  }

  s_original_qt_handler = qInstallMessageHandler(qtMessageHandler);
  s_qt_redirect_enabled.store(true);

  debug("Qt log redirection enabled");
}

void HertLog::disableQtLogRedirect()
{
  if (!s_qt_redirect_enabled.load()) {
    return;
  }

  qInstallMessageHandler(s_original_qt_handler);
  s_qt_redirect_enabled.store(false);

  debug("Qt log redirection disabled");
}

void HertLog::qtMessageHandler(QtMsgType type,
                               const QMessageLogContext& context,
                               const QString& msg)
{
  if (!s_initialized.load()) {
    // 如果HertLog未初始化，使用原始处理器
    if (s_original_qt_handler) {
      s_original_qt_handler(type, context, msg);
    }
    return;
  }

  LogLevel level = LogLevel::INFO;
  switch (type) {
    case QtDebugMsg:
      level = LogLevel::DEBUG;
      break;
    case QtInfoMsg:
      level = LogLevel::INFO;
      break;
    case QtWarningMsg:
      level = LogLevel::WARN;
      break;
    case QtCriticalMsg:
      level = LogLevel::ERROR;
      break;
    case QtFatalMsg:
      level = LogLevel::CRITICAL;
      break;
    default:
      level = LogLevel::INFO;
      break;
  }

  std::string location_info;
  if (context.file) {
    location_info = fmt::format("{}:{}", context.file, context.line);
    if (context.function) {
      location_info += fmt::format(" in {}", context.function);
    }
  }

  std::string final_msg = msg.toStdString();
  if (!location_info.empty()) {
    final_msg = fmt::format("[Qt] {} ({})", final_msg, location_info);
  } else {
    final_msg = fmt::format("[Qt] {}", final_msg);
  }

  log_message_internal(level, final_msg);

  // 对于QtFatalMsg，调用原始处理器确保程序终止
  if (type == QtFatalMsg && s_original_qt_handler) {
    s_original_qt_handler(type, context, msg);
  }
}

#endif  // QT_CORE_LIB

// ============ 标准输出重定向 ============

void HertLog::enableStdRedirect()
{
  if (s_std_redirect_enabled.load()) {
    return;
  }

  // 保存原始缓冲区
  s_original_cout_buf = std::cout.rdbuf();
  s_original_cerr_buf = std::cerr.rdbuf();

  // 创建重定向缓冲区
  s_cout_redirect = std::make_unique<LogStreamBuf>(LogLevel::INFO);
  s_cerr_redirect = std::make_unique<LogStreamBuf>(LogLevel::ERROR);

  // 重定向标准输出
  std::cout.rdbuf(s_cout_redirect.get());
  std::cerr.rdbuf(s_cerr_redirect.get());

  s_std_redirect_enabled.store(true);

  debug("Standard output redirection enabled");
}

void HertLog::disableStdRedirect()
{
  if (!s_std_redirect_enabled.load()) {
    return;
  }

  // 恢复原始缓冲区
  if (s_original_cout_buf) {
    std::cout.rdbuf(s_original_cout_buf);
    s_original_cout_buf = nullptr;
  }
  if (s_original_cerr_buf) {
    std::cerr.rdbuf(s_original_cerr_buf);
    s_original_cerr_buf = nullptr;
  }

  // 清理重定向缓冲区
  s_cout_redirect.reset();
  s_cerr_redirect.reset();

  s_std_redirect_enabled.store(false);

  debug("Standard output redirection disabled");
}

}  // namespace Hert
