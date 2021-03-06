// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "JSONRPCClientPrivatePCH.h"

#define LOCTEXT_NAMESPACE "FJSONRPCClientModule"

void FJSONRPCClientModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FJSONRPCClientModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FJSONRPCClientModule, JSONRPCClient)



#if 0
#ifdef __linux__
void initEvent(TEvent & ev)
{
	//Nothing to do here on Linux;;
}

void triggerEvent(TEvent & ev)
{
	ev.Trigger();
}

void waitEvent(TEvent & ev)
{
	ev.Wait();
}

void resetEvent(TEvent & ev)
{
	ev.Reset();
}

void destroyEvent(TEvent & ev)
{
	//Nothing to do here on Linux;;
}

#else
void initEvent(TEvent & ev)
{
	ev = CreateEvent(
		NULL,               // default security attributes
		true,               // manual-reset event
		false,              // initial state is nonsignaled
		TEXT("WriteEvent")  // object name
	);

	if (ev == NULL)
	{
		UE_LOG(LogTemp, Error, TEXT("CreateEvent failed (%d)"), GetLastError());
	}
}

void triggerEvent(TEvent & ev)
{
	//SetEvent(ev);
	if (!SetEvent(ev))
	{
		UE_LOG(LogTemp, Warning, TEXT("SetEvent failed (%d)"), GetLastError());
		//return;
	}
}

void waitEvent(TEvent & ev)
{
	//WaitForSingleObject(
	//	ev, // event handle
	//	INFINITE);    // indefinite wait
	Sleep(1);
}

void resetEvent(TEvent & ev)
{
	ResetEvent(ev);
}

void destroyEvent(TEvent & ev)
{
	CloseHandle(ev);
}

#endif
#endif

bool runningRussianHaxx = true;

JSONRPCClient::JSONRPCClient()
{
	//initEvent(reqDone);
}

JSONRPCClient::~JSONRPCClient()
{
	runningRussianHaxx = false;
	//destroyEvent(reqDone);
}

bool JSONRPCClient::haveResponse(void) const
{
	return gotResponse;
}
bool JSONRPCClient::isLastRequestOk(void) const
{
	return allOk;
}
std::vector<MessageData>const& JSONRPCClient::getResponse(void) const
{
	return response;
}

void JSONRPCClient::setURL(std::string const& url)
{
	currentId = 0;
	URL = url;
	runningRussianHaxx = true;
}

void JSONRPCClient::sendRPC(std::string const& method, std::map<std::string, std::string> const& params)
{
	//resetEvent(reqDone);
	gotResponse = false;
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	TSharedPtr<FJsonObject> JsonParams = MakeShareable(new FJsonObject());

	for (std::map<std::string, std::string>::const_iterator it = params.begin();
		it != params.end(); it++)
		JsonParams->SetStringField(it->first.c_str(), it->second.c_str());

	JsonObject->SetStringField(TEXT("jsonrpc"), TEXT("2.0"));
	JsonObject->SetStringField(TEXT("method"), method.c_str());
	JsonObject->SetStringField(TEXT("id"), *FString::Printf(TEXT("%d"), currentId)); currentId++;
	JsonObject->SetObjectField(TEXT("params"), JsonParams);
	FString OutputString;
	TSharedRef<TJsonWriter<TCHAR>> JsonWriter = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter);

	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	//Request->OnProcessRequestComplete().BindUObject(this, &JSONClient::OnResponseReceived);
	Request->OnProcessRequestComplete().BindRaw(this, &JSONRPCClient::OnResponseReceived);
	Request->SetURL(URL.c_str());
	Request->SetVerb("POST");
	Request->SetHeader(TEXT("User-Agent"), "X-UnrealEngine-Agent");
	Request->SetHeader("Content-Type", "application/json");
	Request->SetContentAsString(OutputString);
	Request->ProcessRequest();
	// TODO: play a bit with signals to get the output from the request.
}

void JSONRPCClient::sendRPC(std::string const& method, std::vector<MessageData> const& params)
{
	//resetEvent(reqDone);
	gotResponse = false;
	unsigned int maxK = params.size();
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	TArray<TSharedPtr<FJsonValue>> JsonMsgArray;
	TSharedPtr<FJsonObject> JsonMsgArrayObject = MakeShareable(new FJsonObject());

	JsonMsgArray.Reserve(maxK);
	for (unsigned int k = 0; k < maxK; k++)
	{
		TSharedPtr<FJsonObject> JsonMsgParams = MakeShareable(new FJsonObject());
		TSharedPtr<FJsonObject> JsonMsg = MakeShareable(new FJsonObject());

		for (std::map<std::string, std::string>::const_iterator it = params[k].params.begin();
			it != params[k].params.end(); it++)
			JsonMsgParams->SetStringField(it->first.c_str(), it->second.c_str());

		JsonMsg->SetStringField(TEXT("topic"), params[k].topicName.c_str());
		JsonMsg->SetObjectField(TEXT("params"), JsonMsgParams);
		TSharedRef< FJsonValueObject > JsonValue = MakeShareable(new FJsonValueObject(JsonMsg));
		JsonMsgArray.Add(JsonValue);
	}

	JsonObject->SetStringField(TEXT("jsonrpc"), TEXT("2.0"));
	JsonObject->SetStringField(TEXT("method"), method.c_str());
	JsonObject->SetStringField(TEXT("id"), *FString::Printf(TEXT("%d"), currentId)); currentId++;
	JsonMsgArrayObject->SetArrayField(TEXT("params"), JsonMsgArray);
	JsonObject->SetObjectField(TEXT("params"), JsonMsgArrayObject);

	FString OutputString;
	TSharedRef<TJsonWriter<TCHAR>> JsonWriter = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter);

	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindRaw(this, &JSONRPCClient::OnResponseReceived);
	Request->SetURL(URL.c_str());
	Request->SetVerb("POST");
	Request->SetHeader(TEXT("User-Agent"), "X-UnrealEngine-Agent");
	Request->SetHeader("Content-Type", "application/json");
	Request->SetContentAsString(OutputString);
	Request->ProcessRequest();
	// TODO: play a bit with signals to get the output from the request.
}

void JSONRPCClient::sendRPC(std::string const& method, std::vector<MessageData> const& params, std::vector<MessageData> & responseP)
{
	sendRPC(method, params);
	responseP.clear();
	responseP = response;
	//Leave this commented out; this way, we can see via duplicate messages whether the comm is pushing too fast
	//response.clear();
}

void JSONRPCClient::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	//UE_LOG(LogTemp, Error, TEXT("Yay, have response %s %d."), *(Response->GetContentType()), bWasSuccessful);
	//if (bWasSuccessful && Response->GetContentType() == "application/json")
	if (!runningRussianHaxx)
		return;
	if (bWasSuccessful && Response->GetContentType() == "text/plain; charset=utf-8")
	{
		this->allOk = true;
		TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
		TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(Response->GetContentAsString());
		FJsonSerializer::Deserialize(JsonReader, JsonObject);
		TSharedPtr<FJsonObject> JsonMessagesObject = JsonObject->GetObjectField(TEXT("result"))->GetObjectField(TEXT("messages"));

		this->response.clear();
		//if(Response->GetContentLength())
		//    UE_LOG(LogTemp, Log, TEXT("!! %s"), *Response->GetContentAsString());
		for (auto it = JsonMessagesObject->Values.CreateIterator(); it; ++it)
		{
			MessageData aux;
			aux.topicName = std::string(TCHAR_TO_UTF8(*(it.Key())));
			UE_LOG(LogTemp, Log, TEXT("JSON Topic %s"), *(it.Key()));
			for (auto cit = it->Value->AsObject()->Values.CreateIterator(); cit; ++cit)
			{
				UE_LOG(LogTemp, Log, TEXT("    %s : %s"), *(cit.Key()), *(cit->Value->AsString()));
				aux.params.insert(std::pair<std::string, std::string>(std::string(TCHAR_TO_UTF8(*(cit.Key()))), std::string(TCHAR_TO_UTF8(*(cit->Value->AsString())))));
			}
			response.push_back(aux);
		}

		//FString DbgString;
		//TSharedRef<TJsonWriter<TCHAR>> JsonWriter = TJsonWriterFactory<>::Create(&DbgString);
		//FJsonSerializer::Serialize(JsonMessagesObject.ToSharedRef(), JsonWriter);
		//UE_LOG(LogTemp, Error, TEXT("Yay, have response %s."), *(DbgString));
		//SomeVariable = JsonObject->GetStringField("some_response_field");
	}
	else
	{
		// Handle error here
		this->allOk = false;
	}
	this->gotResponse = true;
	//triggerEvent(this->reqDone);
}
	
