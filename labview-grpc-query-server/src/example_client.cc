//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <measurement_service.grpc.pb.h>
#include <sstream>
#include <fstream>
#include <iostream>

//---------------------------------------------------------------------
//---------------------------------------------------------------------
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using namespace std;
using namespace measurementservice;

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class QueryClient
{
public:
    QueryClient(shared_ptr<Channel> channel);

public:
    void Invoke(const string& command, const string& parameters);
    string Query(const string &command);
    unique_ptr<grpc::ClientReader<ServerEvent>> Register(const string& eventName);

public:
    ClientContext m_context;
    unique_ptr<MeasurementService::Stub> m_Stub;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
QueryClient::QueryClient(shared_ptr<Channel> channel)
    : m_Stub(MeasurementService::NewStub(channel))
{        
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void QueryClient::Invoke(const string& command, const string& parameters)
{
    InvokeRequest request;
    request.set_command(command);
    request.set_parameter(parameters);

    ClientContext context;
    InvokeResponse reply;
    Status status = m_Stub->Invoke(&context, request, &reply);
    if (!status.ok())
    {
        cout << status.error_code() << ": " << status.error_message() << endl;
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
string QueryClient::Query(const string &command)
{
    QueryRequest request;
    request.set_query(command);
    QueryResponse reply;
    ClientContext context;

    Status status = m_Stub->Query(&context, request, &reply);

    if (status.ok())
    {
        return reply.message();
    }
    else
    {
        cout << status.error_code() << ": " << status.error_message() << endl;
        return "RPC failed";
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
unique_ptr<grpc::ClientReader<ServerEvent>> QueryClient::Register(const string& eventName)
{
    RegistrationRequest request;
    request.set_eventname(eventName);

    return m_Stub->Register(&m_context, request);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
string GetServerAddress(int argc, char** argv)
{
    string target_str;
    string arg_str("--target");
    if (argc > 1)
    {
        string arg_val = argv[1];
        size_t start_pos = arg_val.find(arg_str);
        if (start_pos != string::npos)
        {
            start_pos += arg_str.size();
            if (arg_val[start_pos] == '=')
            {
                target_str = arg_val.substr(start_pos + 1);
            }
            else
            {
                cout << "The only correct argument syntax is --target=" << endl;
                return 0;
            }
        }
        else
        {
            cout << "The only acceptable argument is --target=" << endl;
            return 0;
        }
    }
    else
    {
        target_str = "localhost:50051";
    }
    return target_str;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
string GetCertPath(int argc, char** argv)
{
    string cert_str;
    string arg_str("--cert");
    if (argc > 2)
    {
        string arg_val = argv[2];
        size_t start_pos = arg_val.find(arg_str);
        if (start_pos != string::npos)
        {
            start_pos += arg_str.size();
            if (arg_val[start_pos] == '=')
            {
                cert_str = arg_val.substr(start_pos + 1);
            }
            else
            {
                cout << "The only correct argument syntax is --cert=" << endl;
                return 0;
            }
        }
        else
        {
            cout << "The only acceptable argument is --cert=" << endl;
            return 0;
        }
    }
    return cert_str;
}

std::string read_keycert( const std::string& filename)
{	
	std::string data;
	std::ifstream file(filename.c_str(), std::ios::in);
	if (file.is_open())
	{
		std::stringstream ss;
		ss << file.rdbuf();
		file.close();
		data = ss.str();
	}
	return data;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
int main(int argc, char **argv)
{
    auto target_str = GetServerAddress(argc, argv);
    auto certificatePath = GetCertPath(argc, argv);

    shared_ptr<grpc::ChannelCredentials> creds;
    if (!certificatePath.empty())
    {
        std::string cacert = read_keycert(certificatePath);
        grpc::SslCredentialsOptions ssl_opts;
        ssl_opts.pem_root_certs=cacert;
        creds = grpc::SslCredentials(ssl_opts);
    }
    else
    {
        creds = grpc::InsecureChannelCredentials();
    }
    auto channel = grpc::CreateChannel(target_str, creds);
    QueryClient client(channel);

    auto period = client.Query("samplingperiod:100");
    cout << "Sampling period: " << period << endl;

    auto result = client.Query("Uptime");
    cout << "Server uptime: " << result << endl;

    auto reader = client.Register("Heartbeat");
    int count = 0;
    ServerEvent event;
    while (reader->Read(&event))
    {
        cout << "Server Event: " << event.eventdata() << endl;
        count += 1;
        if (count == 5)
        {
            client.Invoke("Reset", "");
        }
    }
    Status status = reader->Finish();
    cout << "Server notifications complete" << endl;

	auto iterations = client.Query("numiterations:20");
    cout << "numiterations: " << iterations << endl;

    cout << "Performing OCV measurement" << endl;
    {
        auto startTime = chrono::steady_clock::now();
        ClientContext ctx;
        OCVRequest request;
        OCVData data;
        client.m_Stub->PerformOCVMeasurement(&ctx, request, &data);
        auto measurements = data.data();
        auto endTime = chrono::steady_clock::now();
        auto elapsed = chrono::duration_cast<chrono::milliseconds>(endTime - startTime);
        cout << "OCV measurement took: " << elapsed.count() << " milliseconds" << endl;
        cout << "Received " << measurements.size() << " measurements." << endl;
        cout << "First Results: "  << endl;
        int x=0;
        for (auto it = measurements.begin();  it != measurements.end(); ++it)
        {
            cout << "  V1:" << (*it).battery1voltage() << " V2:" << (*it).battery2voltage() << " V3:" << (*it).battery3voltage() << " V4:" << (*it).battery4voltage() << " V5:" << (*it).battery5voltage() << " V6:" << (*it).battery6voltage() << endl;
            if (++x == 10)
            {
                break;
            }
        }
    }
    cout << "Performing Streaming OCV measurement" << endl;
    {
        auto startTime = chrono::steady_clock::now();
        ClientContext ctx;
        OCVRequest request;
        OCVRaw data;
        auto measurementReader = client.m_Stub->StreamOCVMeasurement(&ctx, request);
        int x=0;
        cout << "First OCV Results: "  << endl;
        while (measurementReader->Read(&data))
        {
            if (++x <= 500)
            {
                cout << "  V1:" << data.battery1voltage() << " V2:" << data.battery2voltage() << " V3:" << data.battery3voltage() << " V4:" << data.battery4voltage() << " V5:" << data.battery5voltage() << " V6:" << data.battery6voltage() << endl;
            }
			else
			{
				break;
			}
        }
		client.Invoke("stopstream", "");
        auto endTime = chrono::steady_clock::now();
        auto elapsed = chrono::duration_cast<chrono::milliseconds>(endTime - startTime);
        cout << "OCV measurement took: " << elapsed.count() << " milliseconds" << endl;
        cout << "Received " << x << " measurements." << endl;
    }
}
