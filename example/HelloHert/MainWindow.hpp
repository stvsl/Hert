#pragma once

#include <QLabel>
#include <QObject>
#include <QPushButton>
#include <QThread>
#include <QVBoxLayout>
#include <QWidget>

// 工作对象类 - 将在后台线程中运行
class Worker : public QObject
{
  Q_OBJECT

public:
  explicit Worker(QObject* parent = nullptr);

signals:
  void threadInfoReady(QString threadInfo);

public slots:
  void doWork();
};

// 主窗口类
class MainWindow : public QWidget
{
  Q_OBJECT

public:
  MainWindow(Qt::HANDLE mainThreadId, QWidget* parent = nullptr);
  ~MainWindow();

private slots:
  void onButtonClicked();
  void onWorkerThreadInfoReady(const QString& info);

private:
  void setupUI();
  void setupWorkerThread();

private:
  Qt::HANDLE m_mainThreadId;
  QLabel* m_mainThreadLabel;
  QLabel* m_currentThreadLabel;
  QLabel* m_workerThreadLabel;
  QThread* m_workerThread;
  Worker* m_worker;
};
