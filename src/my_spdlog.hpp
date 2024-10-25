// Define this file because SPDLOG_* does not respect if SPDLOG_ACTIVE_LEVEL is only defined in main.cpp
// ALso think this file is necessary because ACTIVE must be defined before spdlog
#if DEBUG
  #define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
  #else
  #define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO
#endif
#include "spdlog/spdlog.h"
