// HttpClient.cpp

#include "util/Util.h"
#include "util/protocol/http/HttpSocket.h"

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>

namespace util
{
    namespace protocol
    {

        HttpSocket::HttpSocket(
            boost::asio::io_service & io_svc)
            : super(io_svc)
            , non_block_(false)
            , stream_(*this)
            , chunked_source_(stream_)
            , chunked_sink_(stream_)
            , filter_source_(io_svc)
            , filter_sink_(io_svc)
        {
        }

        void HttpSocket::set_source(
            HttpHead const & head)
        {
            filter_source_.reset();
            std::string encoding = head.content_encoding.get_value_or("");
            if ("gzip" == encoding && filter_source_.empty()) {
                filter_source_.push(boost::iostreams::gzip_decompressor());
            }
            encoding = head.transfer_encoding.get_value_or("");
            if ("chunked" == encoding) {
                filter_source_.push((util::stream::Source &)chunked_source_);
            } else {
                filter_source_.push((util::stream::Source &)stream_);
            }
        }

        void HttpSocket::set_sink(
            HttpHead const & head)
        {
            filter_sink_.reset();
            std::string encoding = head.content_encoding.get_value_or("");
            if ("gzip" == encoding && filter_source_.empty()) {
                filter_sink_.push(boost::iostreams::gzip_compressor());
            }
            encoding = head.transfer_encoding.get_value_or("");
            if ("chunked" == encoding) {
                filter_sink_.push((util::stream::Sink &)chunked_sink_);
            } else {
                filter_sink_.push((util::stream::Sink &)stream_);
            }
        }

        util::stream::Source & HttpSocket::source()
        {
            return filter_source_;
        }

        util::stream::Sink & HttpSocket::sink()
        {
            return filter_sink_;
        }


    } // namespace protocol
} // namespace util
