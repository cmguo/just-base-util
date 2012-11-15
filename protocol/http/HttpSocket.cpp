// HttpClient.cpp

#include "util/Util.h"
#include "util/protocol/http/HttpSocket.h"

#include "util/stream/ChunkedSource.h"
#include "util/stream/ChunkedSink.h"
#include "util/stream/FilterSource.h"
#include "util/stream/FilterSink.h"

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
            , source_(&stream_)
            , sink_(&stream_)
        {
        }

        void HttpSocket::set_source(
            HttpHead const & head)
        {
            assert(source_ == &stream_);
            std::string encoding = head.transfer_encoding.get_value_or("");
            if ("chunked" == encoding) {
                source_ = new util::stream::ChunkedSource(*source_);
            }
            encoding = head.content_encoding.get_value_or("");
            if ("gzip" == encoding) {
                util::stream::FilterSource * filter_source = new util::stream::FilterSource(*source_);
                filter_source->push(boost::iostreams::gzip_decompressor());
                filter_source->complete();
                source_ = filter_source;
            }
        }

        void HttpSocket::reset_source(
            HttpHead const & head)
        {
            assert(source_ == &stream_);
            std::string encoding = head.content_encoding.get_value_or("");
            if ("gzip" == encoding) {
                util::stream::FilterSource * filter_source = (util::stream::FilterSource *)source_;
                source_ = &filter_source->source();
                filter_source->reset();
                delete filter_source;
            }
            encoding = head.transfer_encoding.get_value_or("");
            if ("chunked" == encoding) {
                util::stream::ChunkedSource * chunked_source = (util::stream::ChunkedSource *)source_;
                source_ = &chunked_source->source();
                delete chunked_source;
            }
            assert(source_ == &stream_);
        }

        void HttpSocket::set_sink(
            HttpHead const & head)
        {
            assert(sink_ == &stream_);
            std::string encoding = head.transfer_encoding.get_value_or("");
            if ("chunked" == encoding) {
                sink_ = new util::stream::ChunkedSink(*sink_);
            }
            encoding = head.content_encoding.get_value_or("");
            if ("gzip" == encoding) {
                util::stream::FilterSink * filter_sink = new util::stream::FilterSink(*sink_);
                filter_sink->push(boost::iostreams::gzip_compressor());
                filter_sink->complete();
                sink_ = filter_sink;
            }
        }

        void HttpSocket::reset_sink(
            HttpHead const & head)
        {
            std::string encoding = head.content_encoding.get_value_or("");
            if ("gzip" == encoding) {
                util::stream::FilterSink * filter_sink = (util::stream::FilterSink *)sink_;
                sink_ = &filter_sink->sink();
                filter_sink->reset();
                delete filter_sink;
            }
            encoding = head.transfer_encoding.get_value_or("");
            if ("chunked" == encoding) {
                util::stream::ChunkedSink * chunked_sink = (util::stream::ChunkedSink *)sink_;
                sink_ = &chunked_sink->sink();
                delete chunked_sink;
            }
            assert(sink_ == &stream_);
        }

        util::stream::Source & HttpSocket::source()
        {
            return *source_;
        }

        util::stream::Sink & HttpSocket::sink()
        {
            return *sink_;
        }


    } // namespace protocol
} // namespace util
