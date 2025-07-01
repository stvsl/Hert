#include "Hert/Hert.hpp"

#include <fmt/core.h>

#include "Hert/HertVersion.h"

auto Hert::version() noexcept -> const char*
{
  static const std::string versionStr = fmt::format("{}", PROJECT_VERSION_STR);
  return versionStr.c_str();
}
