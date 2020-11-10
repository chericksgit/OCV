//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <measurement_server.h>

//---------------------------------------------------------------------
//---------------------------------------------------------------------
using namespace std;
using namespace measurementservice;

//---------------------------------------------------------------------
//---------------------------------------------------------------------
EventData::EventData(ServerContext* _context)
{
    context = _context;    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void EventData::WaitForComplete()
{    
    std::unique_lock<std::mutex> lck(lockMutex);
    lock.wait(lck);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void EventData::NotifyComplete()
{    
	std::unique_lock<std::mutex> lck(lockMutex);
	lock.notify_all();
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
ServerStartEventData::ServerStartEventData()
    : EventData(NULL)
{
    serverStartStatus = 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
OCVMeasurementData::OCVMeasurementData(ServerContext* _context, const OCVRequest* _request, OCVData* _response)
    : EventData(_context)
{
    request = _request;
    writer = nullptr;
    response = _response;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
OCVMeasurementData::OCVMeasurementData(ServerContext* _context, const OCVRequest* _request, ::grpc::ServerWriter<OCVRaw>* _writer)
    : EventData(_context)
{
    request = _request;
    writer = _writer;
    response = nullptr;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
InvokeData::InvokeData(ServerContext* _context, const InvokeRequest* _request, InvokeResponse* _response)
    : EventData(_context)
{
    request = _request;
    response = _response;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
QueryData::QueryData(ServerContext* _context, const QueryRequest* _request, QueryResponse* _response)
    : EventData(_context)
{    
    request = _request;
    response = _response;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
RegistrationRequestData::RegistrationRequestData(ServerContext* _context, const RegistrationRequest* _request, ::grpc::ServerWriter<ServerEvent>* _writer)
    : EventData(_context)
{
    request = _request;
    eventWriter = _writer;
}

