#pragma once

#include <QApplication>

#include <Hert/Hert_export.hpp>

HERT_EXPORT class HertApplication : public QApplication
{
  Q_OBJECT
public:
  /**
   * @brief 启动一个 Hert 应用程序
   *
   * @param argc
   * @param argv
   * @param flags
   */
  explicit HertApplication(int& argc,
                           char** argv,
                           int flags = ApplicationFlags);

  ~HertApplication() override;

  HertApplication(const HertApplication&) = delete;
  HertApplication& operator=(const HertApplication&) = delete;
  HertApplication(HertApplication&&) = delete;
  HertApplication& operator=(HertApplication&&) = delete;

  /**
   * @brief 进入 Qt 事件循环。
   * @return 应用程序退出代码
   */
  static int exec();

  /**
   * @brief 重载事件分发，捕获事件循环中的异常并安全退出。
   */
  bool notify(QObject* receiver, QEvent* event) override;

  /**
   * @brief 应用现代化的 Qt 设置，例如高 DPI 支持。
   */
  static void applyModernSettings();
};
