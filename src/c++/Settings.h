#pragma once

#include <Json2Settings.h>

class Settings
{
public:
	Settings() = delete;

	static inline bool LoadSettings(bool a_dumpParse = false)
	{
		auto [log, success] = Json2Settings::load_settings(FILE_NAME, a_dumpParse);
		if (!log.empty()) {
			logger::error("%s", log.c_str());
		}

		return success;
	}

	static inline Json2Settings::bSetting useThreshold{ "useThreshold", true };
	static inline Json2Settings::iSetting costThreshold{ "costThreshold", 500 };

private:
	static inline constexpr char FILE_NAME[] = "Data\\SKSE\\Plugins\\MumsTheWord.json";
};
