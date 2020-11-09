#pragma once
#include "stdafx.h"

typedef void(*CCThreadMessageRun)(void* data);

class CCThreadMessageHandler;
class CCThreadMessageCenter
{
public:
	virtual CCThreadMessageHandler* CreateThreadMessageHandler(int id) { return nullptr; }
	virtual CCThreadMessageHandler* GetThreadMessageHandler(int id) { return nullptr; }
	virtual bool RigisterThreadMessageHandler(int id, CCThreadMessageHandler*handler) { return false; }
	virtual bool UnRigisterThreadMessageHandler(int id) { return false; }

	virtual bool RunOnThread(int id, void* data, CCThreadMessageRun run) { return false; }
	virtual bool RunOnUIThread(void* data, CCThreadMessageRun run) { return false; }
};

#define CCTHREAD_MSG_HANDLER_DEF_MESSAGE 0x5000

struct CCThreadMessageRunData {
	CCThreadMessageRun run;
	void* data;
};

class CCThreadMessageHandler
{
public:
	bool isUiThread = false;
	HWND targetWindow = nullptr;
	UINT targetWindowMessage = CCTHREAD_MSG_HANDLER_DEF_MESSAGE;
	UINT threadId = 0;
	int id;

	CCThreadMessageHandler(int id) {
		this->id = id;
	}
	virtual ~CCThreadMessageHandler() {}

	virtual bool RunNext() { return false; }
	virtual void AddRun(CCThreadMessageRunData* run) {}
	virtual void FreeRun(CCThreadMessageRunData* run) {}
};

#ifdef VR720_EXPORTS

#include <unordered_map>

class CCThreadMessageHandlerInternal : public CCThreadMessageHandler
{
public:
	CCThreadMessageHandlerInternal(int id) : CCThreadMessageHandler(id) {}
	~CCThreadMessageHandlerInternal();

	bool RunNext();
	void AddRun(CCThreadMessageRunData* run);
	void FreeRun(CCThreadMessageRunData* run);

private:
	std::list<CCThreadMessageRunData*> pendingRun;
};

class CCThreadMessageCenterInternal : public CCThreadMessageCenter
{
public:
	CCThreadMessageCenterInternal();
	~CCThreadMessageCenterInternal();

	CCThreadMessageHandler* CreateThreadMessageHandler(int id) override { return new CCThreadMessageHandlerInternal(id); }
	CCThreadMessageHandler* GetThreadMessageHandler(int id) override;

	bool RigisterThreadMessageHandler(int id, CCThreadMessageHandler* handler) override;
	bool UnRigisterThreadMessageHandler(int id) override;

	bool RunOnThread(int id, void* data, CCThreadMessageRun run) override;
	bool RunOnUIThread(void* data, CCThreadMessageRun run) override;

private:
	std::unordered_map<int, CCThreadMessageHandler*> handlers;
	int uiThreadHandlerId = 0;
};

#endif