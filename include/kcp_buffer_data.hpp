#ifndef _KCP_BUFFER_DATA__
#define _KCP_BUFFER_DATA__

#include <stdint.h>
#include <sys/types.h>


#define MAX_DATA_SIZE 10*1024

namespace asio_kcp {

  class kcp_buffer_data{
  public:
    kcp_buffer_data();
    ~kcp_buffer_data();
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
