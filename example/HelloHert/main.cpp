#include <QApplication>
#include <QThread>

#include <fmt/format.h>

#include "Hert/HertApplication.hpp"
#include "Hert/HertDaemon.hpp"
#include "Hert/HertDump.hpp"
#include "MainWindow.hpp"

auto main(int argc, char** argv) -> int
{
  HertDump::init("./core_dumps");  // 初始化崩溃处理，指定 core 文件目录
  // 设置崩溃回调函数
  HertDump::setCrashCallback([]() { fmt::print("崩溃回调函数被调用！\n"); });

  // 在创建 QApplication 之前应用现代化设置
  HertApplication::applyModernSettings();

  HertApplication app(argc, argv);
  app.setApplicationName("HelloHert");
  app.setApplicationVersion("1.0.0");

  // 获取main函数所在线程ID
  auto mainThreadId = QThread::currentThreadId();
  fmt::print("Main thread ID: {}\n", mainThreadId);

  HertDaemon::instance().initialize(argc, argv);

  // 创建主窗口（在主线程中）
  MainWindow window(mainThreadId);
  window.show();

  fmt::print("Application started\n");
  return app.exec();
}
