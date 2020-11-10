//---------------------------------------------------------------------
// Implementation objects for the LabVIEW implementation of the
// gRPC MeasurementService
//---------------------------------------------------------------------
#pragma once

#ifdef __WIN32__
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <measurement_service.grpc.pb.h>
#include <condition_variable>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::ServerWriter;
using namespace measurementservice;
using namespace std;

#ifdef _WIN32
    #define LIBRARY_EXPORT extern "C" __declspec(dllexport)
#else
    #define LIBRARY_EXPORT extern "C"
#endif

//---------------------------------------------------------------------
// LabVIEW definitions
//---------------------------------------------------------------------
typedef int32_t MagicCookie;
typedef MagicCookie LVRefNum;
typedef MagicCookie LVUserEventRef;

typedef struct {
	int32_t cnt; /* number of bytes that follow */
	char str[1]; /* cnt bytes */
} LStr, * LStrPtr, ** LStrHandle;


typedef struct {
	int32_t cnt; /* number of bytes that follow */
    int32_t padding;
	int8_t str[1]; /* cnt bytes */
} LV1DArray, * LV1DArrayPtr, ** LV1DArrayHandle;

//---------------------------------------------------------------------
// LabVIEW definitions
//---------------------------------------------------------------------
typedef void* LVgRPCid;
typedef void* LVgRPCServerid;

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class EventData
{
public:
    EventData(ServerContext* context);

private:
	mutex lockMutex;
	condition_variable lock;

public:
	ServerContext* context;

public:
    void WaitForComplete();
    void NotifyComplete();
};

class LabVIEWMeasurementServerInstance;

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class LabVIEWMeasurementServer final : public measurementservice::MeasurementService::Service
{
public:
    LabVIEWMeasurementServer(LabVIEWMeasurementServerInstance* instance);
    void SopServer();
    void RegisterEvent(string eventName, LVUserEventRef reference);

    // Overrides
    Status Invoke(ServerContext* context, const InvokeRequest* request, InvokeResponse* response) override;
    Status Query(ServerContext* context, const QueryRequest* request, QueryResponse* response) override; 
    Status Register(ServerContext* context, const RegistrationRequest* request, ServerWriter<ServerEvent>* writer) override;
    Status PerformOCVMeasurement(ServerContext* context, const OCVRequest* request, OCVData* response) override;
    Status StreamOCVMeasurement(ServerContext* context, const OCVRequest* request, ServerWriter<OCVRaw>* writer) override;

private:
    LabVIEWMeasurementServerInstance* m_Instance;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class ServerStartEventData : public EventData
{
public:
    ServerStartEventData();

public:
    int serverStartStatus;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class OCVMeasurementData : public EventData
{
public:
    OCVMeasurementData(ServerContext* context, const OCVRequest* request, ServerWriter<OCVRaw>* writer);
    OCVMeasurementData(ServerContext* context, const OCVRequest* request, OCVData* response);

public:
	const OCVRequest* request;
	grpc::ServerWriter<OCVRaw>* writer;
	OCVData* response;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class InvokeData : public EventData
{
public:
    InvokeData(ServerContext* context, const InvokeRequest* request, InvokeResponse* response);

public:
	const InvokeRequest* request;
	InvokeResponse* response;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class QueryData : public EventData
{
public:
    QueryData(ServerContext* context, const QueryRequest* request, QueryResponse* response);

public:
	const QueryRequest* request;
	QueryResponse* response;
};


//---------------------------------------------------------------------
//---------------------------------------------------------------------
class RegistrationRequestData : public EventData
{
public:
    RegistrationRequestData(ServerContext* context, const RegistrationRequest* request, ServerWriter<measurementservice::ServerEvent>* writer);

public:
    const measurementservice::RegistrationRequest* request;
    grpc::ServerWriter<measurementservice::ServerEvent>* eventWriter;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class LabVIEWMeasurementServerInstance
{
public:
    int Run(string address, string serverCertificatePath, string serverKeyPath);
    void StopServer();
    void RegisterEvent(string eventName, LVUserEventRef reference);
    void SendEvent(string name, EventData* data);

private:
    unique_ptr<Server> m_Server;
    map<string, LVUserEventRef> m_RegisteredServerMethods;

private:
    static void RunServer(string address, string serverCertificatePath, string serverKeyPath, LabVIEWMeasurementServerInstance* instance, ServerStartEventData* serverStarted);
};


//---------------------------------------------------------------------
//---------------------------------------------------------------------
struct LVErrorOutData
{
  int errCode;
  LStrHandle errMessage;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
struct LVOCVRawData
{
  float battery1voltage;
  float battery2voltage;
  float battery3voltage;
  float battery4voltage;
  float battery5voltage;
  float battery6voltage;
  float battery7voltage;
  float battery8voltage;
  float battery9voltage;
  float battery10voltage;
  float battery11voltage;
  float battery12voltage;
  float battery13voltage;
  float battery14voltage;
  float battery15voltage;
  float battery16voltage;
  float battery17voltage;
  float battery18voltage;
  float battery19voltage;
  float battery20voltage;
  float battery21voltage;
  float battery22voltage;
  float battery23voltage;
  float battery24voltage;
  LVErrorOutData error;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
struct LVOCVResponse
{
    LV1DArrayHandle results;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
struct LVInvokeRequest
{
    LStrHandle command;
    LStrHandle parameter;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
struct LVInvokeResponse
{
    int32_t status;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
struct LVQueryRequest
{
    LStrHandle query;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
struct LVQueryResponse
{
    LStrHandle message;
    int32_t status;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
struct LVRegistrationRequest
{
    LStrHandle eventName;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
struct LVServerEvent
{
    LStrHandle eventData;
    int32_t serverId;
    int32_t status;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void OccurServerEvent(LVUserEventRef event, EventData* data);
