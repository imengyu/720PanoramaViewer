#include "CCThreadMessageCenter.h"

CCThreadMessageCenterInternal::CCThreadMessageCenterInternal()
{
}
CCThreadMessageCenterInternal::~CCThreadMessageCenterInternal()
{
	if (!handlers.empty()) {
		for (auto it = handlers.begin(); it != handlers.end(); ++it)
			delete (* it).second;
		handlers.clear();
	}
}

bool CCThreadMessageCenterInternal::RigisterThreadMessageHandler(int id, CCThreadMessageHandler* handler)
{
	if (handlers.find(id) == handlers.end()) {
		if (handler->isUiThread)
			uiThreadHandlerId = id;
		handler->threadId = GetCurrentThreadId();
		handlers.insert(std::pair<int, CCThreadMessageHandler*>(id, handler));
		return true;
	}
	return false;
}
bool CCThreadMessageCenterInternal::UnRigisterThreadMessageHandler(int id)
{
	if (handlers.find(id) != handlers.end()) {
		delete handlers[id];
		handlers.erase(id);
	}
	return false;
}
CCThreadMessageHandler* CCThreadMessageCenterInternal::GetThreadMessageHandler(int id) {
	if (handlers.find(id) != handlers.end())
		return handlers[id];
	return nullptr;
}

bool CCThreadMessageCenterInternal::RunOnThread(int id, void* data, CCThreadMessageRun run)
{
	if (run == nullptr)
		return false;
	auto* handler = GetThreadMessageHandler(id);
	if (handler) {
		auto* runData = new CCThreadMessageRunData();
		runData->data = data;
		runData->run = run;
		handler->AddRun(runData);
		return true;
	}
	return false;
}
bool CCThreadMessageCenterInternal::RunOnUIThread(void* data, CCThreadMessageRun run)
{
	return RunOnThread(uiThreadHandlerId, data, run);
}

bool CCThreadMessageHandlerInternal::RunNext() {
	if (!pendingRun.empty()) {
		CCThreadMessageRunData* data = pendingRun.back();
		pendingRun.pop_back();
		data->run(data->data);
		FreeRun(data);
	}
	return false;
}
void CCThreadMessageHandlerInternal::AddRun(CCThreadMessageRunData* run) {
	pendingRun.push_back(run);
	if (isUiThread && targetWindow)
		PostMessage(targetWindow, targetWindowMessage, (WPARAM)id, (LPARAM)threadId);
}
void CCThreadMessageHandlerInternal::FreeRun(CCThreadMessageRunData* run)
{
	delete run;
}
