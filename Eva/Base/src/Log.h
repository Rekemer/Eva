#pragma once
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <memory>
class Log
{
public:
	static std::shared_ptr<spdlog::logger> GetLog()
	{

		if (!isInit)
		{
			spdlog::set_pattern("%^[%T] %n: %v%$");
			s_Logger = spdlog::stdout_color_mt("Eva");
			s_Logger->set_level(spdlog::level::level_enum::trace);
			isInit = true;
		}
		return s_Logger;
	}
private:
	inline static bool isInit = false;
	inline static std::shared_ptr<spdlog::logger> s_Logger;
};

