// RtspServer.h

#ifndef _UTIL_PROTOCOL_RTSP_RTSP_SERVER_H_
#define _UTIL_PROTOCOL_RTSP_RTSP_SERVER_H_

#include "util/protocol/rtsp/RtspSession.h"
#include "util/stream/StreamTransfer.h"

#include <framework/network/ServerManager.h>

namespace util
{
    namespace protocol
    {

        class RtspServer
            : public RtspSession
        {
        public:
            RtspServer(
                boost::asio::io_service & io_svc);

            virtual ~RtspServer();

        protected:
            RtspResponse & response()
            {
                return resp_;
            }

            void response(
                boost::system::error_code const & ec);

        private:
            void start();

        private:
            template <
                typename HttpProxy, 
                typename Manager
            >
            friend class framework::network::ServerManager;

        private:
            RtspResponse resp_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTSP_RTSP_SERVER_H_
