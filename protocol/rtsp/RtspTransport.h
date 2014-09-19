// RtspTransport.h

#ifndef _UTIL_PROTOCOL_RTSP_RTSP_TRANSPORT_H_
#define _UTIL_PROTOCOL_RTSP_RTSP_TRANSPORT_H_

#include "util/protocol/rtsp/RtspSocket.h"

#include <util/stream/Socket.h>

namespace util
{
    namespace protocol
    {

        extern bool create_transport(
            util::stream::Dual *& rtp_socket, 
            RtspSocket & rtsp_socket, 
            std::string const & in_transport, 
            std::string & out_transport, 
            boost::system::error_code & ec);

        extern bool connect_transport(
            util::stream::Dual * rtp_socket, 
            std::string const & transport, 
            boost::system::error_code & ec);

    } // namespace protocol
} // namespace util

#endif // _PPBOX_RTSPD_TRANSPORT_H_
