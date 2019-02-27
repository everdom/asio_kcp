#include <cstring>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <stdio.h>
#include "../platform.h"

#include "connect_packet.hpp"

#define ASIO_KCP_CONNECT_PACKET "asio_kcp_connect_package get_conv"
#define ASIO_KCP_SEND_BACK_CONV_PACKET "asio_kcp_connect_back_package get_conv:"
#define ASIO_KCP_DISCONNECT_PACKET "asio_kcp_disconnect_package"

namespace asio_kcp {

std::string making_connect_packet(void)
{
    return std::string(ASIO_KCP_CONNECT_PACKET, sizeof(ASIO_KCP_CONNECT_PACKET));
}

bool is_connect_packet(const char* data, size_t len)
{
    return (len == sizeof(ASIO_KCP_CONNECT_PACKET) &&
        memcmp(data, ASIO_KCP_CONNECT_PACKET, sizeof(ASIO_KCP_CONNECT_PACKET) - 1) == 0);
}

bool is_send_back_conv_packet(const char* data, size_t len)
{
    return (len > sizeof(ASIO_KCP_SEND_BACK_CONV_PACKET) &&
        memcmp(data, ASIO_KCP_SEND_BACK_CONV_PACKET, sizeof(ASIO_KCP_SEND_BACK_CONV_PACKET) - 1) == 0);
}

std::string making_send_back_conv_packet(kcp_conv_t conv)
{
    char str_send_back_conv[256] = "";
    size_t n = snprintf(str_send_back_conv, sizeof(str_send_back_conv), "%s %llu", ASIO_KCP_SEND_BACK_CONV_PACKET, conv);
    return std::string(str_send_back_conv, n);
}

kcp_conv_t grab_conv_from_send_back_conv_packet(const char* data, size_t len)
{
    std::stringstream convStr;
    convStr<<(data + sizeof(ASIO_KCP_SEND_BACK_CONV_PACKET));

    kcp_conv_t conv;
    convStr>>conv;
    return conv;
}




std::string making_disconnect_packet(kcp_conv_t conv)
{
    char str_disconnect_packet[256] = "";
    size_t n = snprintf(str_disconnect_packet, sizeof(str_disconnect_packet), "%s %llu", ASIO_KCP_DISCONNECT_PACKET, conv);
    return std::string(str_disconnect_packet, n);
}

bool is_disconnect_packet(const char* data, size_t len)
{
    return (len > sizeof(ASIO_KCP_DISCONNECT_PACKET) &&
        memcmp(data, ASIO_KCP_DISCONNECT_PACKET, sizeof(ASIO_KCP_DISCONNECT_PACKET) - 1) == 0);
}

kcp_conv_t grab_conv_from_disconnect_packet(const char* data, size_t len)
{
    std::stringstream convStr;
    convStr<<(data + sizeof(ASIO_KCP_DISCONNECT_PACKET));

    kcp_conv_t conv;
    convStr>>conv;
    return conv;
}


} // namespace asio_kcp
