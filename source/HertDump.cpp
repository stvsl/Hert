#include <atomic>
#include <csignal>
#include <filesystem>
#include <iostream>

#include "Hert/HertDump.hpp"

#include <cpptrace/cpptrace.hpp>

std::function<void()> HertDump::crashCallback = nullptr;
bool HertDump::initialized = false;
std::string HertDump::coreDumpDir;

void HertDump::init(const std::string& coreDir)
{
  if (initialized) {
    return;
  }
  setCoreDumpDir(coreDir);
  installSignalHandlers();
  initialized = true;
}

void HertDump::setCoreDumpDir(const std::string& dir)
{
  coreDumpDir = dir;
  if (!dir.empty()) {
    std::filesystem::create_directories(dir);
  }
}

void HertDump::setCrashCallback(std::function<void()> cb)
{
  crashCallback = std::move(cb);
}

void HertDump::printStacktrace()
{
  cpptrace::generate_trace(3).print_with_snippets(std::cerr, true);
}

void HertDump::installSignalHandlers()
{
  (void)std::signal(SIGSEGV, signalHandler);
  (void)std::signal(SIGABRT, signalHandler);
  (void)std::signal(SIGFPE, signalHandler);
  (void)std::signal(SIGILL, signalHandler);
#ifdef SIGBUS
  (void)std::signal(SIGBUS, signalHandler);
#endif
}

void HertDump::signalHandler(int signum)
{
  static std::atomic_bool handling{false};
  if (handling.exchange(true)) {
    // 已经在处理信号，直接退出，防止递归
    _exit(128 + signum);
  }
  std::cerr << "\n[HertDump] 崩溃信号: " << signum << std::endl;
  printStacktrace();
  if (!coreDumpDir.empty()) {
    std::string corePath = coreDumpDir + "/core_" + std::to_string(::getpid());
    std::cerr << "[HertDump] core dump 路径: " << corePath << std::endl;
  }
  if (crashCallback) {
    crashCallback();
  }
  _exit(128 + signum); // 直接退出，防止递归
}
