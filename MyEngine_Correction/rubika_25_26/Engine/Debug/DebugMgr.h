#pragma once

#include <string>
#include <unordered_map>
#include <functional>

class IDebugable
{
public:
	virtual void DrawDebug() = 0;
};

class DebugMgr
{
public:
	void Draw();

	bool RegisterDebugableWindow(const std::string& debugName, IDebugable* pDebugable);
	void UnregisterDebugableWindow(const std::string& debugName);

	bool RegisterDebugAction(const std::string& debugName, std::function<void(void*)> callback, void* userData);
	void UnregisterDebugAction(const std::string& debugName);

private:

	struct sDebugAction
	{
		std::function<void(void*)> Callback;
		void* UserData;
	};

	std::unordered_map<std::string, IDebugable*> Debugables;
	std::unordered_map<std::string, sDebugAction> Actions;
};