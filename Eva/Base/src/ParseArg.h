#pragma once
#include <string.h>
#include <vector>
#include <format>
namespace Eva
{
	enum class ArgType
	{
		// vm test mode
		TEST,
		// -bpath="path" for compiler
		// evc file path
		EVC_PATH,
		// -spath="path" for compiler
		// script path
		EVA_PATH

	};

	struct Arg
	{
		ArgType type;
		std::string value;

		bool operator==(ArgType otherType)
		{
			return type == otherType;
		}

	};

	std::vector<Arg> ParseArgsCmp(int argc, const char* argv[])
	{
		std::vector<Arg> args;
		args.reserve(argc);
		for (int i = 1; i < argc; i++)
		{
			std::string arg = argv[i];
			std::cout << std::format("arg[{}]: {}\n", i, arg);
			if (i + 1 != argc)
			{
				assert(arg[0] == '-');
				arg = arg.substr(1, arg.size() - 1);
			}

			size_t eq_pos = arg.find('=');
			if (eq_pos != std::string::npos)
			{
				auto key = arg.substr(0, eq_pos);
				auto value = arg.substr(eq_pos + 1);
				key.erase(0, key.find_first_not_of('-'));

				if (key == "bpath") {
					args.emplace_back(ArgType::EVC_PATH, value);
				}
				else if (key == "spath")
				{
					args.emplace_back(ArgType::EVA_PATH, value);
				}
			}
		}
		return args;
	}
	std::vector<Arg> ParseArgsVM(int argc, const char* argv[])
	{
		std::vector<Arg> args;
		args.reserve(argc);
		for (int i = 1; i < argc; i++)
		{
			std::string arg = argv[i];
			if (i + 1 != argc)
			{
				assert(arg[0] == '-');
				arg = arg.substr(1, arg.size() - 1);
			}

			if (arg == "test")
			{
				args.emplace_back(ArgType::TEST);
			}
			else
			{
				args.emplace_back(ArgType::EVC_PATH, arg);
			}
		}
		return args;
	}
}