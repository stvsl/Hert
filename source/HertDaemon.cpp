#include <QDebug>

#include <Hert/HertDaemon.hpp>

HertDaemon::HertDaemon()
    : QObject(nullptr)
    , m_argc(nullptr)
    , m_argv(nullptr)
    , m_initialized(false)
{
  qDebug() << "HertDaemon已创建";
}

void HertDaemon::initialize(int& argc, char** argv)
{
  if (m_initialized) {
    qWarning() << "HertDaemon已经初始化";
    return;
  }

  m_argc = &argc;
  m_argv = argv;
  m_initialized = true;

  qDebug() << "HertDaemon初始化完成";
}

void HertDaemon::stop()
{
  if (!m_initialized) {
    return;
  }

  qDebug() << "正在停止HertDaemon";
  m_initialized = false;
  qDebug() << "HertDaemon已停止";
}
