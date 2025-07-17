#include <QApplication>
#include <QMutex>
#include <QTimer>

#include "MainWindow.hpp"

#include <fmt/format.h>

// Worker 类实现
Worker::Worker(QObject* parent)
    : QObject(parent)
{
}

void Worker::doWork()
{
  // 模拟一些工作
  QThread::msleep(100);  // 休眠100毫秒

  auto currentThreadId = QThread::currentThreadId();
  QString info = QString("Worker Thread ID: %1")
                     .arg(QString::number(quintptr(currentThreadId)));

  fmt::print("Work completed in thread: {}\n", currentThreadId);
  emit threadInfoReady(info);
}

// MainWindow 类实现
MainWindow::MainWindow(Qt::HANDLE mainThreadId, QWidget* parent)
    : QWidget(parent)
    , m_mainThreadId(mainThreadId)
{
  setupUI();
  setupWorkerThread();
}

MainWindow::~MainWindow()
{
  if (m_workerThread) {
    m_workerThread->quit();
    m_workerThread->wait();
  }
}

void MainWindow::onButtonClicked()
{
  auto currentThreadId = QThread::currentThreadId();
  m_currentThreadLabel->setText(
      QString("UI Thread ID: %1 (Button clicked)")
          .arg(QString::number(quintptr(currentThreadId))));

  fmt::print("Button clicked in UI thread: {}\n", currentThreadId);

  // 触发工作线程执行任务
  QMetaObject::invokeMethod(m_worker, "doWork", Qt::QueuedConnection);
}

void MainWindow::onWorkerThreadInfoReady(const QString& info)
{
  m_workerThreadLabel->setText(info);
}

void MainWindow::setupUI()
{
  setWindowTitle("Hello Hert - Multi-threaded");
  resize(800, 600);

  auto* layout = new QVBoxLayout(this);

  // 显示主线程ID
  m_mainThreadLabel =
      new QLabel(QString("Main Thread ID: %1")
                     .arg(QString::number(quintptr(m_mainThreadId))));
  m_mainThreadLabel->setStyleSheet(
      "font-size: 14px; margin: 10px; padding: 10px; background-color: "
      "#f0f0f0; border-radius: 5px;");
  layout->addWidget(m_mainThreadLabel);

  // 显示UI线程ID（应该与主线程相同）
  auto uiThreadId = QThread::currentThreadId();
  m_currentThreadLabel = new QLabel(
      QString("UI Thread ID: %1").arg(QString::number(quintptr(uiThreadId))));
  m_currentThreadLabel->setStyleSheet(
      "font-size: 14px; margin: 10px; padding: 10px; background-color: "
      "#e8f5e8; border-radius: 5px;");
  layout->addWidget(m_currentThreadLabel);

  // 显示工作线程信息
  m_workerThreadLabel =
      new QLabel("Worker Thread ID: (Click button to start work)");
  m_workerThreadLabel->setStyleSheet(
      "font-size: 14px; margin: 10px; padding: 10px; background-color: "
      "#e0e0ff; border-radius: 5px;");
  layout->addWidget(m_workerThreadLabel);

  // 按钮
  auto* button = new QPushButton("Start Background Work");
  button->setStyleSheet(
      "font-size: 14px; padding: 10px; margin: 10px; background-color: "
      "#4CAF50; color: white; border: none; border-radius: 5px;");
  connect(button, &QPushButton::clicked, this, &MainWindow::onButtonClicked);
  layout->addWidget(button);

  layout->addStretch();
}

void MainWindow::setupWorkerThread()
{
  m_workerThread = new QThread(this);
  m_worker = new Worker();  // Worker对象将移动到工作线程
  m_worker->moveToThread(m_workerThread);

  // 连接信号槽
  connect(m_worker,
          &Worker::threadInfoReady,
          this,
          &MainWindow::onWorkerThreadInfoReady);

  // 启动工作线程
  m_workerThread->start();

  fmt::print("Worker thread started\n");
}
