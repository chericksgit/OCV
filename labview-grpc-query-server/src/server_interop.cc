//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <measurement_server.h>
#include <iostream>
#include <memory>
#include <string>
#include <map>
#include <mutex>
#include <thread>

#ifndef _WIN32
#include <dlfcn.h>
#endif

//---------------------------------------------------------------------
//---------------------------------------------------------------------
using namespace std;

//---------------------------------------------------------------------
//---------------------------------------------------------------------
typedef int (*NumericArrayResize_T)(int32_t, int32_t, void* handle, size_t size);
typedef int (*PostLVUserEvent_T)(LVUserEventRef ref, void *data);

//---------------------------------------------------------------------
//---------------------------------------------------------------------
static NumericArrayResize_T NumericArrayResize;
static PostLVUserEvent_T PostLVUserEvent;

#ifdef _WIN32

//---------------------------------------------------------------------
//---------------------------------------------------------------------
static void InitCallbacks()
{
	if (NumericArrayResize != NULL)
	{
		return;
	}

	auto lvModule = GetModuleHandle("LabVIEW.exe");
	if (lvModule == NULL)
	{
		lvModule = GetModuleHandle("lvffrt.dll");
	}
	if (lvModule == NULL)
	{
		lvModule = GetModuleHandle("lvrt.dll");
	}
	NumericArrayResize = (NumericArrayResize_T)GetProcAddress(lvModule, "NumericArrayResize");
	PostLVUserEvent = (PostLVUserEvent_T)GetProcAddress(lvModule, "PostLVUserEvent");
}

#else

//---------------------------------------------------------------------
//---------------------------------------------------------------------
static void InitCallbacks()
{
	if (NumericArrayResize != NULL)
	{
		return;
	}

	auto lvModule = dlopen("labview", RTLD_NOLOAD);
	if (lvModule == NULL)
	{
		lvModule = dlopen("liblvrt.so", RTLD_NOW);
	}
	NumericArrayResize = (NumericArrayResize_T)dlsym(lvModule, "NumericArrayResize");
	PostLVUserEvent = (PostLVUserEvent_T)dlsym(lvModule, "PostLVUserEvent");
}

#endif

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void OccurServerEvent(LVUserEventRef event, EventData* data)
{
	auto error = PostLVUserEvent(event, &data);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void SetLVString(LStrHandle* lvString, string str)
{
    auto length = str.length();    
	auto error = NumericArrayResize(0x01, 1, lvString, length);
	memcpy((**lvString)->str, str.c_str(), length);
	(**lvString)->cnt = (int)length;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
string GetLVString(LStrHandle lvString)
{    
    if (lvString == NULL || *lvString == NULL)
    {
        return string();
    }

	auto count = (*lvString)->cnt;
	auto chars = (*lvString)->str;

	string result(chars, count);
    return result;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t LVCreateServer(LVgRPCServerid* id)
{
	InitCallbacks();
	auto server = new LabVIEWMeasurementServerInstance();
	*id = server;   
	return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t LVStartServer(char* address, char* serverCertificatePath, char* serverKeyPath, LVgRPCServerid* id)
{   
	LabVIEWMeasurementServerInstance* server = *(LabVIEWMeasurementServerInstance**)id;
	return server->Run(address, serverCertificatePath, serverKeyPath);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t LVStopServer(LVgRPCServerid* id)
{
	LabVIEWMeasurementServerInstance* server = *(LabVIEWMeasurementServerInstance**)id;
	server->StopServer();
	return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t RegisterServerEvent(const char* name, LVUserEventRef* item, LVgRPCServerid* id)
{
	LabVIEWMeasurementServerInstance* server = *(LabVIEWMeasurementServerInstance**)id;
	server->RegisterEvent(name, *item);
	return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t OCVMeasurementSetResponse(LVgRPCid id, LVOCVResponse* response)
{
	OCVMeasurementData* data = *(OCVMeasurementData**)id;
	LVOCVRawData* lvData = (LVOCVRawData*)((*response->results)->str);
	for (int x=0; x < (*response->results)->cnt; ++x)
	{
		auto rawData = data->response->add_data();
		rawData->set_battery1voltage(lvData->battery1voltage);
		rawData->set_battery2voltage(lvData->battery2voltage);
		rawData->set_battery3voltage(lvData->battery3voltage);
		rawData->set_battery4voltage(lvData->battery4voltage);
		rawData->set_battery5voltage(lvData->battery5voltage);
		rawData->set_battery6voltage(lvData->battery6voltage);
		rawData->set_battery7voltage(lvData->battery7voltage);
		rawData->set_battery8voltage(lvData->battery8voltage);
		rawData->set_battery9voltage(lvData->battery9voltage);
		rawData->set_battery10voltage(lvData->battery10voltage);
		rawData->set_battery11voltage(lvData->battery11voltage);
		rawData->set_battery12voltage(lvData->battery12voltage);
		rawData->set_battery13voltage(lvData->battery13voltage);
		rawData->set_battery14voltage(lvData->battery14voltage);
		rawData->set_battery15voltage(lvData->battery15voltage);
		rawData->set_battery16voltage(lvData->battery16voltage);
		rawData->set_battery17voltage(lvData->battery17voltage);
		rawData->set_battery18voltage(lvData->battery18voltage);
		rawData->set_battery19voltage(lvData->battery19voltage);
		rawData->set_battery20voltage(lvData->battery20voltage);
		rawData->set_battery21voltage(lvData->battery21voltage);
		rawData->set_battery22voltage(lvData->battery22voltage);
		rawData->set_battery23voltage(lvData->battery23voltage);
		rawData->set_battery24voltage(lvData->battery24voltage);
		rawData->mutable_error()->set_errcode(lvData->error.errCode);
		rawData->mutable_error()->set_errmessage(GetLVString(lvData->error.errMessage));
		lvData += 1;
	}
	data->NotifyComplete();
	return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t OCVMeasurementSetStreamResponse(LVgRPCid id, LVOCVRawData* response)
{
	OCVMeasurementData* measurementData = *(OCVMeasurementData**)id;

	measurementservice::OCVRaw data;
	data.set_battery1voltage(response->battery1voltage);
	data.set_battery2voltage(response->battery2voltage);
	data.set_battery3voltage(response->battery3voltage);
	data.set_battery4voltage(response->battery4voltage);
	data.set_battery5voltage(response->battery5voltage);
	data.set_battery6voltage(response->battery6voltage);
	data.set_battery7voltage(response->battery7voltage);
	data.set_battery8voltage(response->battery8voltage);
	data.set_battery9voltage(response->battery9voltage);
	data.set_battery10voltage(response->battery10voltage);
	data.set_battery11voltage(response->battery11voltage);
	data.set_battery12voltage(response->battery12voltage);
	data.set_battery13voltage(response->battery13voltage);
	data.set_battery14voltage(response->battery14voltage);
	data.set_battery15voltage(response->battery15voltage);
	data.set_battery16voltage(response->battery16voltage);
	data.set_battery17voltage(response->battery17voltage);
	data.set_battery18voltage(response->battery18voltage);
	data.set_battery19voltage(response->battery19voltage);
	data.set_battery20voltage(response->battery20voltage);
	data.set_battery21voltage(response->battery21voltage);
	data.set_battery22voltage(response->battery22voltage);
	data.set_battery23voltage(response->battery23voltage);
	data.set_battery24voltage(response->battery24voltage);
	data.mutable_error()->set_errcode(response->error.errCode);
	data.mutable_error()->set_errmessage(GetLVString(response->error.errMessage));

	measurementData->writer->Write(data);
	return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t OCVMeasurementComplete(LVgRPCid id)
{
	OCVMeasurementData* measurementData = *(OCVMeasurementData**)id;
	measurementData->NotifyComplete();
	return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t InvokeGetRequest(LVgRPCid id, LVInvokeRequest* request)
{
	InvokeData* data = *(InvokeData**)id;
    SetLVString(&request->command, data->request->command());
    SetLVString(&request->parameter, data->request->parameter());
	return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t InvokeSetResponse(LVgRPCid id, LVInvokeResponse* response)
{
	InvokeData* data = *(InvokeData**)id;
    data->response->set_status(response->status);
    data->NotifyComplete();
	return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t QueryGetRequest(LVgRPCid id, LVQueryRequest* request)
{
	QueryData* data = *(QueryData**)id;
    SetLVString(&request->query, data->request->query());
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t QuerySetResponse(LVgRPCid id, LVQueryResponse* response)
{
	QueryData* data = *(QueryData**)id;
    data->response->set_message(GetLVString(response->message));
    data->response->set_status(response->status);
    data->NotifyComplete();
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t RegisterGetRequest(LVgRPCid id, LVRegistrationRequest* request)
{
	RegistrationRequestData* data = *(RegistrationRequestData**)id;
    SetLVString(&request->eventName, data->request->eventname());
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t NotifyServerEvent(LVgRPCid id, LVServerEvent* event)
{
	RegistrationRequestData* data = *(RegistrationRequestData**)id;
    measurementservice::ServerEvent e;
    e.set_eventdata(GetLVString(event->eventData));
    e.set_serverid(event->serverId);
    e.set_status(event->status);
    data->eventWriter->Write(e);
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t CloseServerEvent(LVgRPCid id)
{
	RegistrationRequestData* data = *(RegistrationRequestData**)id;
    data->NotifyComplete();
    return 0;
}
