#pragma once
#include <functional>
#include <string>

/**
 * @brief 堆栈回溯与崩溃处理工具，基于 cpptrace 封装
 */
class HertDump
{
public:
  /**
   * @brief 初始化并安装信号/异常处理器，自动打印崩溃堆栈并转储 core 文件
   * @param coreDir core 文件保存目录（可选）
   */
  static void init(const std::string& coreDir = "");

  /**
   * @brief 手动打印当前堆栈信息
   */
  static void printStacktrace();

  /**
   * @brief 设置崩溃后执行的回调函数
   */
  static void setCrashCallback(std::function<void()> cb);

  /**
   * @brief 设置 core dump 文件保存目录
   */
  static void setCoreDumpDir(const std::string& dir);

private:
  static std::function<void()> crashCallback;
  static std::string coreDumpDir;
  static bool initialized;
  static void installSignalHandlers();
  static void signalHandler(int signum);
};
