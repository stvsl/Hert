#pragma once

#include <memory>
#include <mutex>

#include <Hert/Hert_export.hpp>

// 单例基类模板
template<typename T>
class HERT_EXPORT HertSingleton
{
public:
  // 获取单例实例
  static T& instance()
  {
    std::call_once(s_onceFlag, []() { s_instance.reset(new T()); });
    return *s_instance;
  }

  // 禁用拷贝构造和赋值
  HertSingleton(const HertSingleton&) = delete;
  HertSingleton& operator=(const HertSingleton&) = delete;

  // 禁用移动构造和赋值
  HertSingleton(HertSingleton&&) = delete;
  HertSingleton& operator=(HertSingleton&&) = delete;

protected:
  HertSingleton() = default;
  virtual ~HertSingleton() = default;

private:
  static std::unique_ptr<T> s_instance;
  static std::once_flag s_onceFlag;
};

// 静态成员定义
template<typename T>
std::unique_ptr<T> HertSingleton<T>::s_instance = nullptr;

template<typename T>
std::once_flag HertSingleton<T>::s_onceFlag;

// 单例宏 - 简化单例类的声明
#define HERT_SINGLETON(ClassName) \
  friend class HertSingleton<ClassName>; \
\
private: \
  ClassName(); \
\
public: \
  ~ClassName() = default; \
\
  /* 禁用拷贝构造和赋值 */ \
  ClassName(const ClassName&) = delete; \
  ClassName& operator=(const ClassName&) = delete; \
\
  /* 禁用移动构造和赋值 */ \
  ClassName(ClassName&&) = delete; \
  ClassName& operator=(ClassName&&) = delete;

// 获取单例实例的便捷宏
#define HERT_INSTANCE(ClassName) ClassName::instance()
