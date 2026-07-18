#pragma once

#include <string>
#include <unordered_map>

namespace cfg
{
	inline bool& bref(const char* key, bool def = false)
	{
		static std::unordered_map<std::string, bool> store;
		const std::string k(key);
		const auto it = store.find(k);
		if (it == store.end())
			return store.emplace(k, def).first->second;
		return it->second;
	}

	inline int& iref(const char* key, int def = 0)
	{
		static std::unordered_map<std::string, int> store;
		const std::string k(key);
		const auto it = store.find(k);
		if (it == store.end())
			return store.emplace(k, def).first->second;
		return it->second;
	}

	inline float& fref(const char* key, float def = 0.f)
	{
		static std::unordered_map<std::string, float> store;
		const std::string k(key);
		const auto it = store.find(k);
		if (it == store.end())
			return store.emplace(k, def).first->second;
		return it->second;
	}

	inline std::unordered_map<int, bool>& mapref(const char* key)
	{
		static std::unordered_map<std::string, std::unordered_map<int, bool>> store;
		return store[key];
	}

	inline int menuColor[4] = { 163, 212, 31, 255 };
}
