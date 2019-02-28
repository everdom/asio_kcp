#ifndef _ASIO_KCP_BUFFER_DATA__
#define _ASIO_KCP_BUFFER_DATA__

#include <stdint.h>
#include <sys/types.h>
#include <string>

#define MAX_DATA_SIZE (10*1024)
// #define MAX_DATA_SIZE (50)

namespace kcp_svr {

  class kcp_buffer_data{
  public:
    kcp_buffer_data(void);
    ~kcp_buffer_data(void);
    // kcp_buffer_data(std::string str);
    kcp_buffer_data(const char *data, long size);
    int set_data(const char *data, long size);
    char *data();
    long size();
  private:
    char buffer_data[MAX_DATA_SIZE];
    long buffer_size;
  };

} // end of asio_kcp

#endif // _KCP_BUFFER_DATA__
