#pragma once

#include <QObject>

#include <Hert/HertSingleton.hpp>
#include <Hert/Hert_export.hpp>

HERT_EXPORT class HertDaemon
    : public QObject
    , public HertSingleton<HertDaemon>
{
  Q_OBJECT
  HERT_SINGLETON(HertDaemon)

public:
  // 初始化
  void initialize(int& argc, char** argv);

  // 停止
  void stop();

private:
  int* m_argc;
  char** m_argv;
  bool m_initialized;
};
