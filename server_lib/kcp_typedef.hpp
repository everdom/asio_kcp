#pragma once

#define ASIO_KCP_CONNECTION_TIMEOUT_TIME 10 * 1000 // default is 10 seconds.

#include <stdint.h>
#include <memory>
#include "kcp_buffer_data.hpp"

struct IKCPCB;
typedef struct IKCPCB ikcpcb;

#ifndef __IUINT64_DEFINED
#define __IUINT64_DEFINED
#if defined(_MSC_VER) || defined(__BORLANDC__)
typedef unsigned __int64 IUINT64;
#else
typedef unsigned long long IUINT64;
#endif
#endif

// indicate a converse between a client and connection_obj between server.
typedef IUINT64 kcp_conv_t;

namespace kcp_svr
{

    enum eEventType
    {
        eConnect,
        eDisconnect,
        eRcvMsg,
        eLagNotify,

        eCountOfEventType
    };

    const char* eventTypeStr(eEventType eventType);

    typedef struct{
      uint8_t from_srv;
      uint16_t msg_id;
      uint32_t data_len;
    }kcp_frame_head;

    typedef struct{
      kcp_frame_head head;
      char *data;
    }kcp_frame;

    typedef void(event_callback_t)(kcp_conv_t /*conv*/, eEventType /*event_type*/, kcp_buffer_data &);
    // typedef void(event_callback_t)(kcp_conv_t /*conv*/, eEventType /*event_type*/, std::shared_ptr<std::string> /*msg*/);
}
