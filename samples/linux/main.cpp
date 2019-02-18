#include <string>
#include <iostream>
#include <chrono>
#include <thread>
#include <kcp_client_wrap.hpp>


using namespace std;
using namespace asio_kcp;

static kcp_client_wrap *g_kcp_client = NULL;


static void client_event_callback(kcp_conv_t conv, eEventType event_type, kcp_buffer_data& msg, void* var){
 
	cout << "client_event_callback conv=" << conv << ",event_type=" << event_type << ",msg=" << msg.data() << ",var=" << var << endl;
}

int sendMsg(char *msg,size_t len){
    return g_kcp_client->send_msg(msg, len);
}

int main(int argc, char* argv[])
{
	g_kcp_client = new kcp_client_wrap();
	int result  = g_kcp_client->connect_async(23445, "192.168.1.102", 12345);
	cout << "connect result = " << result << endl;
	result = g_kcp_client->init_kcp(123456,1,20,2,1);
	cout << "init result = " << result << endl;
	g_kcp_client->set_event_callback(client_event_callback,g_kcp_client);
	cout << "set_event_callback" << endl;
	result = sendMsg("aaa",3);
	cout << "sendMsg result = " << result  << endl;
	this_thread::sleep_for(chrono::milliseconds(10000));
	return 0;
}