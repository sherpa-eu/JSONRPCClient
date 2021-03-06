// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ModuleManager.h"

#include <string>
#include <vector>
#include <map>

#include <Http.h>
#include <Json.h>

#if 0
#ifdef __linux__
#include <CoreObject.h>
#include <CoreUObject.h>
#include <UObjectBase.h>
#include <UObjectBaseUtility.h>
#include <UObject.h>
#include <UObjectArray.h>

#include <ThreadingBase.h>
#include <PThreadEvent.h>

typedef FPThreadEvent TEvent;

void initEvent(TEvent & ev);

void triggerEvent(TEvent & ev);

void waitEvent(TEvent & ev);

void resetEvent(TEvent & ev);

void destroyEvent(TEvent & ev);

#else
#include <Windows.h>
typedef void* TEvent;

void initEvent(TEvent & ev);

void triggerEvent(TEvent & ev);

void waitEvent(TEvent & ev);

void resetEvent(TEvent & ev);

void destroyEvent(TEvent & ev);
#endif
#endif

typedef struct
{
	std::string topicName;
	std::map<std::string, std::string> params;
}MessageData;

class JSONRPCCLIENT_API JSONRPCClient
{
	int currentId;

	std::string URL;

	//TEvent reqDone;

	bool allOk;
	bool gotResponse;
	std::vector<MessageData> response;

public:

	JSONRPCClient();

	~JSONRPCClient();

	bool haveResponse(void) const;
	bool isLastRequestOk(void) const;
	std::vector<MessageData>const& getResponse(void) const;

	void setURL(std::string const& url);

	void sendRPC(std::string const& method, std::map<std::string, std::string> const& params);

	void sendRPC(std::string const& method, std::vector<MessageData> const& params);

	void sendRPC(std::string const& method, std::vector<MessageData> const& params, std::vector<MessageData> & responseP);

private:
	void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};

class FJSONRPCClientModule : public IModuleInterface
{

public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
