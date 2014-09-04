// RtspSocket.cpp

#include "util/Util.h"
#include "util/protocol/rtsp/RtspSocket.h"
#include "util/protocol/rtsp/RtspSocket.hpp"

#include <boost/asio/read_until.hpp>
#include <boost/asio/write.hpp>

namespace util
{
    namespace protocol
    {

        RtspSocket::RtspSocket(
            boost::asio::io_service & io_svc)
            : MessageSocket(io_svc, read_parser_, NULL)
        {
        }

        size_t RtspSocket::write_msgs(
            std::vector<RtspMessage> const & msgs, 
            boost::system::error_code & ec)
        {
            return write_msg(RtspMessageVector(msgs), ec);
        }

        void RtspMessageVector::to_data(
            StreamBuffer & buf, 
            void * vctx) const
        {
            for (size_t i = 0; i < msgs_.size(); ++i) {
                msgs_[i].to_data(buf, NULL);
            }
        }

    } // namespace protocol
} // namespace util
