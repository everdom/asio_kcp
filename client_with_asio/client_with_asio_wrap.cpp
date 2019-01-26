#include "client_with_asio_wrap.hpp"
#include <algorithm>
#include <boost/bind.hpp>
#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <unordered_map>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "../essential/utility/strutil.h"
#include "test_util.h"
#include "../util/connect_packet.hpp"

#define PACKAGE_LOSE_RATIO 0
#define PACKAGE_CONTENT_DAMAGE_RATIO 0
#define SEND_TEST_MSG_INTERVAL 1000

/* get system time */
void client_with_asio_wrap::itimeofday(long *sec, long *usec)
{
	struct timeval time;
	gettimeofday(&time, NULL);
	if (sec) *sec = time.tv_sec;
	if (usec) *usec = time.tv_usec;
}

/* get clock in millisecond 64 */
IINT64 client_with_asio_wrap::iclock64(void)
{
    long s, u;
    IINT64 value;
    itimeofday(&s, &u);
    value = ((IINT64)s) * 1000 + (u / 1000);
    return value;
}


IUINT32 client_with_asio_wrap::iclock()
{
    return (IUINT32)(iclock64() & 0xfffffffful);
}

std::string client_with_asio_wrap::get_milly_sec_time_str(void)
{
    boost::posix_time::ptime ptime = boost::posix_time::microsec_clock::universal_time();
    return boost::posix_time::to_iso_extended_string(ptime);
}

#define CLOCK_START_STR "!ha"
#define CLOCK_INTERVAL_STR "_ha"
std::string client_with_asio_wrap::make_test_str(size_t test_str_size)
{
    std::ostringstream ostr;
    ostr << CLOCK_START_STR << iclock64();
    std::string msg_str = ostr.str();
    msg_str += test_str(CLOCK_INTERVAL_STR, test_str_size - msg_str.size());
    return msg_str;
}

std::string client_with_asio_wrap::get_cur_time_str()
{
    time_t tmpcal_ptr = {0};
    struct tm *tmp_ptr = NULL;
    tmpcal_ptr = time(NULL);
    tmp_ptr = localtime(&tmpcal_ptr);
    std::ostringstream osstrm;
    osstrm << tmp_ptr->tm_hour << ":" << tmp_ptr->tm_min << "." << tmp_ptr->tm_sec;
    return osstrm.str();
}

uint64_t client_with_asio_wrap::get_time_from_msg(const std::string& msg)
{
    std::size_t pos = msg.find(CLOCK_INTERVAL_STR);
    if (pos == std::string::npos)
    {
        std::cout << "wrong msg: " << msg << std::endl;
        return 0;
    }
    const std::string& time_str = msg.substr(0 + sizeof(CLOCK_START_STR) - 1, pos);
    //std::cout << "time_str: " << time_str << std::endl;
    return std::atoll(time_str.c_str());
}

uint64_t client_with_asio_wrap::search_time_from_kcp_str(const std::string& kcp_str)
{
    //std::cout << "udp send: " << kcp_str.size() << std::endl << Essential::ToHexDumpText(kcp_str, 32) << std::endl;

    auto end_iter = kcp_str.find(CLOCK_INTERVAL_STR);
    if (end_iter == std::string::npos)
        return 0;

    auto start_iter = kcp_str.find(CLOCK_START_STR);
    if (start_iter == std::string::npos)
        return 0;

    uint64_t ret = std::atoll(&kcp_str[start_iter] + sizeof(CLOCK_START_STR) -1);
    //std::cout << "search_time_from_kcp_str: " << ret << std::endl;
    return ret;
}



client_with_asio_wrap::client_with_asio_wrap(boost::asio::io_service& io_service, int udp_port_bind,
        const std::string& server_ip, const int server_port, const size_t test_str_size) :
    stopped_(false),
    test_str_size_(test_str_size),
    client_timer_(io_service),
    client_timer_send_msg_(io_service)
{
    kcp_client_wrap_.set_event_callback(client_event_callback_func, (void*)this);
    kcp_client_wrap_.connect_async(udp_port_bind, server_ip, server_port);
    kcp_client_wrap_.init_kcp(123456789, 1, 2, 20 ,1);
    hook_timer_send_msg();
}

void client_with_asio_wrap::stop_all()
{
  stopped_ = true;
}

void client_with_asio_wrap::send_test_msg(void)
{
    kcp_client_wrap_.send_msg(make_test_str(test_str_size_));
}

void client_with_asio_wrap::print_recv_log(const std::string& msg)
{
    static size_t static_good_recv_count = 0;
    static uint64_t static_last_refresh_time = 0;
    static size_t static_recved_bytes = 0;
    static_recved_bytes += msg.size();
    uint64_t cur_time = iclock64();
    uint64_t send_time = get_time_from_msg(msg);
    uint64_t interval = cur_time - send_time;

    if (static_good_recv_count == 0)
    {
        std::cout << std::endl;
        std::cout << get_cur_time_str() << " ";
    }

    static_good_recv_count++;
    recv_package_interval_.push_back(interval);
    recv_package_interval10_.push_back(interval);
    recv_package_interval100_.push_back(interval);

    //std::cout << interval << ":" << send_time << ":" << g_package_send_counter[send_time] << "\t";
    std::cout << interval << ":" << g_package_send_counter[send_time] << "\t";
    g_package_send_counter.erase(send_time);

    if (static_good_recv_count % 10 == 0)
    {
        int average10 = 0;
        for (int x : recv_package_interval10_)
            average10 += x;
        average10 = (average10 / 10);

        std::cout << "max:" << *std::max_element( recv_package_interval10_.begin(), recv_package_interval10_.end() ) <<
            " avrg10:" << average10;
        recv_package_interval10_.clear();
    }

    if (cur_time - static_last_refresh_time > 10 * 1000 && static_good_recv_count % 100 == 30 && static_good_recv_count != 30)
    {
        std::cout << " " << static_cast<double>(static_recved_bytes * 10 / (cur_time - static_last_refresh_time)) / 10 << "KB/s(in)";
        static_last_refresh_time = cur_time;
        static_recved_bytes = 0;
    }

    if (static_good_recv_count % 100 == 0 && static_good_recv_count != 0)
    {
        int average100 = 0;
        for (int x : recv_package_interval100_)
            average100 += x;
        average100 = (average100 / 100);

        int average_total = 0;
        for (int x: recv_package_interval_)
            average_total += x;
        average_total = average_total / recv_package_interval_.size();

        std::cout << " max100:" << *std::max_element( recv_package_interval100_.begin(), recv_package_interval100_.end() ) <<
            " avrg100:" << average100 <<
            " avrgall:" << average_total;
        recv_package_interval100_.clear();
    }

    if (static_good_recv_count % 100 == 10 && static_good_recv_count != 10)
    {
        std::cout << " ext_packet_snd:" << (g_count_send_udp_packet * 100 / g_count_send_kcp_packet) << "%"
            << " ext_size_snd:" << g_count_send_udp_size * 100 / g_count_send_kcp_size << "%";
    }

    if (static_good_recv_count % 10 == 0)
    {
        std::cout << std::endl;
        std::cout << get_cur_time_str() << " ";
    }

    std::cout.flush();
}


void client_with_asio_wrap::hook_timer_send_msg(void)
{
    if (stopped_)
        return;
    client_timer_send_msg_.expires_from_now(boost::posix_time::milliseconds(SEND_TEST_MSG_INTERVAL));
    client_timer_send_msg_.async_wait(std::bind(&client_with_asio_wrap::handle_timer_send_msg, this));
}

void client_with_asio_wrap::handle_timer_send_msg(void)
{
    send_test_msg();
    hook_timer_send_msg();
}

void client_with_asio_wrap::client_event_callback_func(kcp_conv_t conv, eEventType event_type, const std::string& msg, void* var)
{
    ((client_with_asio_wrap*)var)->handle_client_event_callback(conv, event_type, msg);
}

void client_with_asio_wrap::handle_client_event_callback(kcp_conv_t conv, eEventType event_type, const std::string& msg)
{
    switch (event_type)
    {
        case eConnect:
            std::cout << "connect success with conv:" << conv << std::endl;
            break;
        case eConnectFailed:
            std::cout << "connect failed with conv:" << conv << std::endl;
            stopped_ = true;
            break;
        case eRcvMsg:
            std::cout << "recv eRcvMsg with conv:" << conv << "  msg_count: " << msg.size() << std::endl;
            break;
        case eDisconnect:
            std::cout << "disconnect with conv:" << conv << " msg: " << msg << std::endl;
            stopped_ = true; // you can add boost::asio::io_service::work to prevent program quit
            break;
        default:
            ; // do nothing
    }
}


