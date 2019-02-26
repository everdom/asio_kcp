#ifndef _ASIO_HILEIA_KCP_
#define _ASIO_HILEIA_KCP_

#include "kcp_client_wrap.hpp"

namespace asio_kcp {

  typedef struct{
    uint8_t from_srv;
    uint16_t msg_id;
    uint32_t data_len;
  }kcp_frame_head;

  typedef struct{
    kcp_frame_head head;
    const char *data;
  }kcp_frame;

  typedef void(event_callback_t)(kcp_conv_t /*conv*/, eEventType /*event_type*/, kcp_frame &kf, void* /*var*/);

  class hileia_kcp{
  public:
    hileia_kcp(void);
    ~hileia_kcp(void);

    int init_kcp(kcp_conv_t conv, int nodelay, int interval, int resend, int nc);

    void set_event_callback(const event_callback_t& event_callback_func, void* var);

    int connect_async(int udp_port_bind, const std::string& server_ip, const int server_port);



    int send_data(kcp_frame &kf);
    // int send_msg(const char *data, long size);

    int stop();

  private:
    static void client_event_callback_func(kcp_conv_t conv, eEventType event_type, kcp_buffer_data& msg, void* var);
    void handle_client_event_callback(kcp_conv_t conv, eEventType event_type, kcp_buffer_data& msg);
    // void send_msg();

    int send_data_partially(const char* data, uint64_t data_size);

  private:
    kcp_client_wrap kcp_client_wrap_;

    // client_event_callback_t* pevent_func_;
    event_callback_t* pevent_func_;
    void* event_func_var_;
    bool frame_begin;
    uint32_t data_pos;
    kcp_frame kf;
  };
}
#endif
