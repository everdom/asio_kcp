#include <iostream>
#include <memory.h>

#include "hileia_kcp.hpp"
#include "kcp_buffer_data.hpp"

namespace asio_kcp {
  hileia_kcp::hileia_kcp(void):
    pevent_func_(NULL),
    event_func_var_(NULL)
  {
    kcp_client_wrap_.set_event_callback(client_event_callback_func, (void*)this);
    frame_begin = false;
    data_pos = 0;
  }

  hileia_kcp::~hileia_kcp(void)
  {
  }

  int hileia_kcp::init_kcp(kcp_conv_t conv, int nodelay, int interval, int resend, int nc){
    return kcp_client_wrap_.init_kcp(conv, nodelay, interval, resend, nc);
  }

  int hileia_kcp::connect_async(int udp_port_bind, const std::string& server_ip, const int server_port){

    return kcp_client_wrap_.connect_async(udp_port_bind, server_ip, server_port);
  }

  int hileia_kcp::send_data(kcp_frame &kf){

    kcp_frame_head &kfh = kf.head;

    uint64_t data_buf_size = sizeof(kcp_frame_head) + kfh.data_len;
    char *kcp_msg_buf = (char *)malloc(data_buf_size);
    if(kcp_msg_buf == NULL){
      std::cerr << "send_msg error, not enough memory" << std::endl;
      return -1;
    }

    void *ptr = memcpy(kcp_msg_buf, &kfh, sizeof(kcp_frame_head));
    if(ptr == NULL){
      std::cerr << "send_msg error, memcpy error" << std::endl;
      free(kcp_msg_buf);
      return -2;
    }

    ptr = memcpy(kcp_msg_buf+sizeof(kcp_frame_head), kf.data, kfh.data_len);
    if(ptr == NULL){
      std::cerr << "send_msg error, memcpy error"<< std::endl;
      free(kcp_msg_buf);
      return -2;
    }

    int res = send_data_partially(kcp_msg_buf, data_buf_size);
    if(res != 0){
      std::cerr << "send_msg error, send_data_partially error:"<<res << std::endl;
      free(kcp_msg_buf);
      return -3;
    }

    free(kcp_msg_buf);
    return 0;
  }

  void hileia_kcp::set_event_callback(const event_callback_t& event_callback_func, void* var)
  {
    pevent_func_ = &event_callback_func;
    event_func_var_ = var;
  }

  void hileia_kcp::client_event_callback_func(kcp_conv_t conv, eEventType event_type, kcp_buffer_data& msg, void* var)
  {
    ((hileia_kcp*)var)->handle_client_event_callback(conv, event_type, msg);
  }

  void hileia_kcp::handle_client_event_callback(kcp_conv_t conv, eEventType event_type, kcp_buffer_data& msg)
  {
    std::cout << "hileia_kcp::handle_client_event_callback event_type: " << event_type << std::endl;

    kcp_frame_head &kfh = kf.head;

    // 解析帧头
    if(!frame_begin){
      data_pos = 0;

      void *ptr = memcpy(&kfh, msg.data(), sizeof(kcp_frame_head));
      if(ptr == NULL){
        std::cerr << "hileia_kcp::handle_client_event_callback error, memcpy error"<< std::endl;
        return;
      }

      kf.data = (char *)malloc(kfh.data_len);
      if(kf.data == NULL){
        std::cerr << "hileia_kcp::handle_client_event_callback error, memory not enough, data len:"<<kfh.data_len << std::endl;
        return;
      }

      frame_begin = true;

      uint32_t remain_size = msg.size() - sizeof(kcp_frame_head);

      // 如果数据小于一帧的大小
      if(kfh.data_len <= remain_size){
        ptr = memcpy(kf.data, msg.data()+data_pos, kfh.data_len);

        if(ptr == NULL){
          std::cerr << "hileia_kcp::handle_client_event_callback error, memcpy error"<< std::endl;
          return;
        }

        // 加上数据长度
        data_pos += kfh.data_len;

        frame_begin = false;
      }else{
        ptr = memcpy(kf.data, msg.data()+data_pos, remain_size);
        if(ptr == NULL){
          std::cerr << "hileia_kcp::handle_client_event_callback error, memcpy error"<< std::endl;
          return;
        }

        // 加上本帧剩余数据长度
        data_pos += kfh.data_len;
      }
    }

    // 接收中间分块数据
    if(frame_begin){
      uint32_t remain_data_size = kfh.data_len - data_pos;

      // 如果是最后的数据帧
      if(remain_data_size <= msg.size()){
        void * ptr = memcpy(kf.data+data_pos, msg.data(), remain_data_size);
        if(ptr == NULL){
          std::cerr << "hileia_kcp::handle_client_event_callback error, memcpy error" << std::endl;
          return;
        }

        data_pos += remain_data_size;
      }else{
        // 中间帧
        void *ptr = memcpy(kf.data+data_pos, msg.data(), msg.size());
        if(ptr == NULL){
          std::cerr << "hileia_kcp::handle_client_event_callback error, memcpy error"<< std::endl;
          return;
        }

        data_pos += msg.size();
      }


      if(data_pos == kfh.data_len){
        frame_begin = false;

        if (pevent_func_)
        {
          (*pevent_func_)(conv, event_type, kf, event_func_var_);
        }

        free(kf.data);
      }
    }


  }

  int hileia_kcp::send_data_partially(const char *data, uint64_t data_size){
    // 数据量太大分块处理
    uint32_t blockNum = ((data_size+MAX_DATA_SIZE-1) / MAX_DATA_SIZE);

    uint32_t i=0;
    for(i=0; i<blockNum-1; i++){
      kcp_buffer_data msg;
      int ret = msg.set_data(data+i*MAX_DATA_SIZE, MAX_DATA_SIZE);
      if(ret != 0){
        std::cerr << "send_msg set_data error: " << ret << std::endl;
        return ret;
      }
      kcp_client_wrap_.send_msg(msg.data(), msg.size());
    }

    // 最后一块数据
    kcp_buffer_data msg;
    int ret = msg.set_data(data+i*MAX_DATA_SIZE, data_size-i*MAX_DATA_SIZE);
    if(ret != 0){
      std::cerr << "send_msg set_data error: " << ret << std::endl;
      return ret;
    }
    kcp_client_wrap_.send_msg(msg.data(), msg.size());
    return 0;
  }

  int hileia_kcp::stop(){
    kcp_client_wrap_.stop();
    return 0;
  }
}
