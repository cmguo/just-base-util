// RtspServer.cpp

#include "util/Util.h"
#include "util/protocol/rtsp/RtspServer.h"

#include <framework/logger/Logger.h>
#include <framework/logger/FormatRecord.h>

namespace util
{
    namespace protocol
    {

        FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("util.protocol.RtspServer", framework::logger::Warn);

        RtspServer::RtspServer(
            boost::asio::io_service & io_svc)
            : RtspSession(io_svc)
        {
        }

        RtspServer::~RtspServer()
        {
        }

        void RtspServer::start()
        {
            RtspSession::start();
        }

        void RtspServer::response(
            boost::system::error_code const & ec)
        {
            LOG_DEBUG("[response] ec = %s" % ec.message()); 

            if (ec) {
                fail(ec);
            } else {
                post(resp_);
            }
            resp_.clear();
        }

    } // namespace protocol
} // namespace util
