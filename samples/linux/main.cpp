#include <string>
#include <iostream>

#include <kcp_client_wrap.hpp>


using namespace std;
using namespace asio_kcp;

static kcp_client_wrap *g_kcp_client = NULL;

int main(int argc, char* argv[])
{
	g_kcp_client = new kcp_client_wrap();
	int result  = g_kcp_client->connect(23445, "192.168.1.102", 12345);
	cout << "connect result = " << result << endl;
	return 0;
}