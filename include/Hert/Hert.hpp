#pragma once

#include "Hert/Hert_export.hpp"

/**
 *  @brief Hert 版本相关信息
 *
 */
class HERT_EXPORT Hert
{
public:
  /**
   * @brief 获取 Hert 的版本号
   *
   * @return const char* 返回版本号字符串
   */
  static auto version() noexcept -> const char*;

private:
  HERT_SUPPRESS_C4251
};
