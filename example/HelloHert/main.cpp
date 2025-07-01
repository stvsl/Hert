#include <fmt/format.h>

#include "Hert/Hert.hpp"
#include "Hert/HertApplication.hpp"
#include "Hert/HertDump.hpp"

auto main(int argc, char** argv) -> int
{
  HertDump::init("./core_dumps");  // 初始化崩溃处理，指定 core 文件目录
  // 设置崩溃回调函数
  HertDump::setCrashCallback([]() { fmt::print("崩溃回调函数被调用！\n"); });
  fmt::print(fmt::runtime(Hert::version()));
  HertApplication app(argc, argv);
  app.setApplicationName("HelloHert");
  app.setApplicationVersion("1.0.0");

  app.exec();
  return 0;
}
