#include <QDebug>
#include <exception>

#include "Hert/HertApplication.hpp"

HertApplication::HertApplication(int& argc, char** argv, int flags)
    : QApplication(argc, argv, flags)
{
  applyModernSettings();
}

HertApplication::~HertApplication() = default;

int HertApplication::exec()
{
  return QApplication::exec();
}

bool HertApplication::notify(QObject* receiver, QEvent* event)
{
  try {
    return QApplication::notify(receiver, event);
  } catch (const std::exception& e) {
    qCritical() << "Caught exception in event loop:" << e.what();
    // 在这里可以进行一些清理工作
    exit(1);  // 异常退出
    return false;
  } catch (...) {
    qCritical() << "Caught unknown exception in event loop.";
    exit(1);  // 异常退出
    return false;
  }
}

void HertApplication::applyModernSettings()
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0) \
    && QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
  QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
      Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif
}
