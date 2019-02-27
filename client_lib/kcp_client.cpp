#include "kcp_client.hpp"
#include <stdio.h>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <errno.h>
#include <sstream>
#include <fcntl.h>
#include <string.h>

#include "../util/ikcp.h"
#include "../util/connect_packet.hpp"
#include "kcp_client_util.h"

namespace asio_kcp {

kcp_client::kcp_client(void) :
    connect_start_time_(0),
    last_send_connect_msg_time_(0),
    connect_succeed_(false),
    pevent_func_(NULL),
    event_callback_var_(NULL),
    udp_port_bind_(0),
    server_port_(0),
    udp_socket_(-1),
    p_kcp_(NULL)
{
    memset(&servaddr_, 0 ,sizeof(servaddr_));
}

kcp_client::~kcp_client(void)
{
    stop();
}

void kcp_client::clean(void)
{
    if (p_kcp_)
    {
        ikcp_release(p_kcp_);
        p_kcp_ = NULL;
    }
}

void kcp_client::set_event_callback(const client_event_callback_t& event_callback_func, void* var)
{
    pevent_func_ = &event_callback_func;
    event_callback_var_ = var;
}

void kcp_client::stop()
{
  if(udp_socket_)
  {
    close(udp_socket_);
    udp_socket_ = NULL;
  }

  if (p_kcp_)
  {
    ikcp_release(p_kcp_);
    p_kcp_ = NULL;
  }

}

int kcp_client::init_kcp(kcp_conv_t conv, int nodelay, int interval, int resend, int nc)
{
    // printf("kcp_client::init_kcp\n");
    p_kcp_ = ikcp_create(conv, (void*)this);

    if(p_kcp_ == NULL){
      printf("kcp_client::init_kcp ikcp create failed\n");
      return -1;
    }

    p_kcp_->output = &kcp_client::udp_output;

    // 启动快速模式
    // 第二个参数 nodelay-启用以后若干常规加速将启动
    // 第三个参数 interval为内部处理时钟，默认设置为 10ms
    // 第四个参数 resend为快速重传指标，设置为2
    // 第五个参数 为是否禁用常规流控，这里禁止
    //ikcp_nodelay(p_kcp_, 1, 10, 2, 1);
    ikcp_nodelay(p_kcp_, nodelay, interval, resend, nc); // 设置成1次ACK跨越直接重传, 这样反应速度会更快. 内部时钟5毫秒.
    connect_succeed_ = true;

    return 0;
}

int kcp_client::connect_async(int udp_port_bind, const std::string& server_ip, const int server_port)
{
    if (udp_socket_ != -1)
        return KCP_ERR_ALREADY_CONNECTED;

    udp_port_bind_ = udp_port_bind;
    server_ip_ = server_ip;
    server_port_ = server_port;

    // init udp connect
    {
        int ret = init_udp_connect();
        if (ret < 0)
            return ret;
    }

    // do asio_kcp connect
    {
        connect_start_time_ = iclock64();
    }


    return 0;
}

void kcp_client::update()
{
    uint64_t cur_clock = iclock64();

    if (connect_succeed_)
    {
      // send the msg in SendMsgQueue
      do_send_msg_in_queue();

      // recv the udp packet.
      do_recv_udp_packet_in_loop();

      // ikcp_update
      //
      ikcp_update(p_kcp_, cur_clock);
    }
}



//
int kcp_client::init_udp_connect(void)
{
#if defined(WIN32) || defined(WIN64)
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;
    wVersionRequested = MAKEWORD(1, 1);
    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0)
    {
        perror("WSAStartup error");
    }
#endif

    // servaddr_
    {
        servaddr_.sin_family = AF_INET;
        servaddr_.sin_port = htons(server_port_);
        int ret = inet_pton(AF_INET, server_ip_.c_str(), &servaddr_.sin_addr);
        if (ret <= 0)
        {
            if (ret < 0) // errno set
                std::cerr << "inet_pton error return < 0, with errno: " << errno << " " << strerror(errno) << std::endl;
            else
                std::cerr << "inet_pton error return 0" << std::endl;
            return KCP_ERR_ADDRESS_INVALID;
        }
    }


    // set socket non-blocking
    {
#if defined(WIN32) || defined(WIN64)
        udp_socket_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
        if (udp_socket_ < 0)
        {
            std::cerr << "socket error return with errno: " << errno << " " << strerror(errno) << std::endl;
            return KCP_ERR_CREATE_SOCKET_FAIL;
        }

        unsigned long flag = 1;
        if (ioctlsocket(udp_socket_, FIONBIO, &flag) != 0){
            closesocket(udp_socket_);
            return KCP_ERR_SET_NON_BLOCK_FAIL;
        }
#else
        udp_socket_ = socket(AF_INET, SOCK_DGRAM, 0);
        if (udp_socket_ < 0)
        {
            std::cerr << "socket error return with errno: " << errno << " " << strerror(errno) << std::endl;
            return KCP_ERR_CREATE_SOCKET_FAIL;
        }

        int flag = fcntl(udp_socket_, F_GETFL, 0);
        if (flag == -1)
        {
            std::cerr << "get socket non-blocking: fcntl error return with errno: " << errno << " " << strerror(errno) << std::endl;
            return KCP_ERR_SET_NON_BLOCK_FAIL;
        }


        int ret = fcntl(udp_socket_, F_SETFL, flag | O_NONBLOCK);
        if (ret == -1)
        {
            std::cerr << "set socket non-blocking: fcntl error return with errno: " << errno << " " << strerror(errno) << std::endl;
            return KCP_ERR_SET_NON_BLOCK_FAIL;
        }
#endif
    }

    // set recv buf bigger

    // bind
    if (udp_port_bind_ != 0)
    {
        struct sockaddr_in bind_addr;
        bind_addr.sin_family = AF_INET;
        bind_addr.sin_port = htons(udp_port_bind_);
        bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        int ret_bind = ::bind(udp_socket_, (const struct sockaddr*)(&bind_addr), sizeof(bind_addr));
        if (ret_bind < 0){
          std::cerr << "setsockopt error return with errno: " << errno << " " << strerror(errno) << std::endl;
          return KCP_ERR_BIND_UDP_FAILED;
        }
    }

    // udp connect
    {
        int ret = ::connect(udp_socket_, (const struct sockaddr*)(&servaddr_), sizeof(servaddr_));
        if (ret < 0)
        {
            std::cerr << "connect error return with errno: " << errno << " " << strerror(errno) << std::endl;
            return KCP_ERR_CONNECT_FUNC_FAIL;
        }
    }

    return 0;
}

void kcp_client::do_recv_udp_packet_in_loop(void)
{
    char recv_buf[MAX_MSG_SIZE * 2] = ""; // udp packet will not twice bigger than kcp msg size.

    socklen_t addr_len= sizeof(servaddr_);
    const ssize_t ret_recv = recvfrom(udp_socket_, recv_buf, sizeof(recv_buf), 0, (struct sockaddr*)(&servaddr_), &addr_len);

    if (ret_recv < 0)
    {
        int err = errno;
        if (err == EAGAIN)
            return;
        std::ostringstream ostrm;
        std::string err_detail = ostrm.str();
        ostrm << "do_asio_kcp_connect recv error return with errno: " << err << " " << strerror(err);
        std::cerr << err_detail << std::endl;

        kcp_buffer_data msg(err_detail.c_str(), err_detail.size());
        (*pevent_func_)(p_kcp_->conv, eDisconnect, msg, event_callback_var_);
        return;
    }

    if (ret_recv == 0)
        return; // do nothing.   ignore the zero size packet.

    kcp_buffer_data msg(recv_buf, ret_recv);
    // ret_recv > 0
    handle_udp_packet(msg);
    return;
}

int kcp_client::udp_output(const char *buf, int len, ikcpcb *kcp, void *user)
{
    ((kcp_client*)user)->send_udp_package(buf, len);
	return 0;
}

void kcp_client::send_udp_package(const char *buf, int len)
{
    std::cerr << "send_udp_package" << std::endl;

    socklen_t addr_len= sizeof(servaddr_);
    const ssize_t send_ret = sendto(udp_socket_, buf, len, 0, (const sockaddr *)(&servaddr_),addr_len);
    if (send_ret < 0)
    {
        std::cerr << "send_udp_package error with errno: " << errno << " " << strerror(errno) << std::endl;
    }
    else if (send_ret != len)
    {
        std::cerr << "send_udp_package error: not all packet send. " << send_ret << " in " << len << std::endl;
    }
}

int kcp_client::send_msg(const char *data, long size)
{
    std::unique_lock<std::mutex>(mQueueMtx);
    kcp_buffer_data msg;

    std::cout << "do_send_msg_in_queue send_msg size=" << send_msg_queue_.size() <<std::endl;
    int ret = msg.set_data(data, size);
    if(ret != 0){
      std::cerr << "send_msg set_data error: " << ret << std::endl;
      return ret;
    }

    send_msg_queue_.push_back(msg);
    return 0;
}

void kcp_client::do_send_msg_in_queue(void)
{
    kcp_buffer_data msg;
    while (true)
    {
        mQueueMtx.lock();
        std::vector<kcp_buffer_data>::iterator it = send_msg_queue_.begin();
        if(it == send_msg_queue_.end()){
            mQueueMtx.unlock();  
            return;
        }else{
            msg = *it;
            msg.set_data(it->data(),it->size());
            it = send_msg_queue_.erase(it);
            mQueueMtx.unlock();  
        }
        int send_ret = ikcp_send(p_kcp_, msg.data(), msg.size());
        if (send_ret < 0)
        {
            std::cerr << "send_ret<0: " << send_ret << std::endl;
        }
    }
}

void kcp_client::handle_udp_packet(kcp_buffer_data& udp_packet)
{
    // if (is_disconnect_packet(udp_packet.data(), udp_packet.size()))
    // {
    //     if (pevent_func_ != NULL)
    //     {
    //         kcp_buffer_data msg(udp_packet);
    //         (*pevent_func_)(p_kcp_->conv, eDisconnect, msg, event_callback_var_);
    //     }
    //     return;
    // }


    ikcp_input(p_kcp_, udp_packet.data(), udp_packet.size());

    while (true)
    {
        kcp_buffer_data msg = recv_udp_package_from_kcp();
        if (msg.size() > 0)
        {
            // recved good msg.
          std::cerr << "recv good kcp msg: " << msg.data() << std::endl;
            if (pevent_func_ != NULL)
            {
                (*pevent_func_)(p_kcp_->conv, eRcvMsg, msg, event_callback_var_);
            }
            continue;
        }
        break;
    }
}

kcp_buffer_data kcp_client::recv_udp_package_from_kcp(void)
{
    char kcp_buf[MAX_MSG_SIZE] = "";
    int kcp_recvd_bytes = ikcp_recv(p_kcp_, kcp_buf, sizeof(kcp_buf));

    kcp_buffer_data result;
    if (kcp_recvd_bytes < 0)
    {
        //std::cerr << "kcp_recvd_bytes<0: " << kcp_recvd_bytes << std::endl;
      return result;
    }

    result.set_data(kcp_buf, kcp_recvd_bytes);
    // const std::string result(kcp_buf, kcp_recvd_bytes);
    return result;
}


} // namespace asio_kcp
