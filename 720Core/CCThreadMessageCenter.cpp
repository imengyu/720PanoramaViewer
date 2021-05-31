#include "CCThreadMessageCenter.h"
#include "720Core.h"

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
		CCThreadMessageHandler* handler = handlers[id];
		if (handler->isUiThread && handler->id == uiThreadHandlerId)
			uiThreadHandlerId = 0;
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

CCThreadMessageHandlerInternal::~CCThreadMessageHandlerInternal()
{
	if (!pendingRun.empty()) {
		for (auto it = pendingRun.begin(); it != pendingRun.end(); ++it)
			delete (*it);
		pendingRun.clear();
	}
}

bool CCThreadMessageHandlerInternal::RunNext() {
	if (!pendingRun.empty()) {
		CCThreadMessageRunData* data = pendingRun.front();
		pendingRun.pop_front();
		//
		CCThreadMessageRun run = data->run;
		void* paramdata = data->data;
		FreeRun(data);
		//call
		run(paramdata);
		return true;
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
