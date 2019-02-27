#ifndef _BS_CLIENT_WITH_ASIO_HPP_
#define _BS_CLIENT_WITH_ASIO_HPP_

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <unordered_map>
#include "../client_lib/hileia_kcp.hpp"
#include "../util/ikcp.h"

using namespace boost::asio::ip;
using namespace asio_kcp;
class client_with_asio_hileia
  : private boost::noncopyable
{
public:
    client_with_asio_hileia(boost::asio::io_service& io_service, int udp_port_bind,
            const std::string& server_ip, const int server_port, const size_t test_str_size);

    /// Stop all connections.
    void stop_all();

private:

private:
    bool stopped_;

    void print_recv_log(const std::string& msg);

    void handle_timer_send_msg(void);
    void hook_timer_send_msg(void);
    void send_test_msg(void);

  static void client_event_callback_func(kcp_conv_t conv, asio_kcp::eEventType event_type, kcp_frame& msg, void* var);
  void handle_client_event_callback(kcp_conv_t conv, asio_kcp::eEventType event_type, kcp_frame& msg);

    asio_kcp::hileia_kcp kcp_client_hileia_;

    size_t test_str_size_;
    boost::asio::deadline_timer client_timer_;
    boost::asio::deadline_timer client_timer_send_msg_;

    std::vector<uint64_t> recv_package_interval_;
    std::vector<uint64_t> recv_package_interval10_;
    std::vector<uint64_t> recv_package_interval100_;

    std::unordered_map<uint64_t /*package_send_time*/, size_t /*send_counter*/> g_package_send_counter;
    size_t g_count_send_udp_packet = 0;
    size_t g_count_send_kcp_packet = 0;

    uint64_t g_count_send_udp_size = 0;
    uint64_t g_count_send_kcp_size = 0;

    static inline void itimeofday(long *sec, long *usec);
    static inline IINT64 iclock64(void);
    static inline IUINT32 iclock();
    std::string get_milly_sec_time_str(void);
    std::string make_test_str(size_t test_str_size);
    std::string get_cur_time_str();
    uint64_t get_time_from_msg(const std::string& msg);
    uint64_t search_time_from_kcp_str(const std::string& kcp_str);
};

#endif // _BS_CLIENT_WITH_ASIO_HPP_
