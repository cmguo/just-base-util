// HttpClient.cpp

#include "util/Util.h"
#include "util/protocol/http/HttpSocket.h"
#include "util/protocol/http/HttpSocket.hpp"

#include "util/stream/LimitedSource.h"
#include "util/stream/LimitedSink.h"
#include "util/stream/ChunkedSource.h"
#include "util/stream/ChunkedSink.h"
#include "util/stream/FilterSource.h"
#include "util/stream/FilterSink.h"

#include <boost/iostreams/filter/gzip.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/write.hpp>

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

        void HttpSocket::close()
        {
            snd_buf_.reset();
            rcv_buf_.reset();
            super::close();
        }

        size_t HttpSocket::write(
            HttpHead & head)
        {
            if (snd_buf_.size() == 0) {
                std::ostream os(&snd_buf_);
                boost::system::error_code ec;
                head.get_content(os, ec);
                assert(!ec);
            }
            return boost::asio::write((super &)*this, snd_buf_);
        }

        size_t HttpSocket::write(
            HttpHead & head, 
            boost::system::error_code & ec)
        {
            if (snd_buf_.size() == 0) {
                std::ostream os(&snd_buf_);
                head.get_content(os, ec);
                assert(!ec);
            }
            return boost::asio::write((super &)*this, snd_buf_, boost::asio::transfer_all(), ec);
        }

        size_t HttpSocket::read(
            HttpHead & head)
        {
            boost::asio::read_until((super &)*this, rcv_buf_, "\r\n\r\n");
            size_t old_size = rcv_buf_.size();
            std::istream is(&rcv_buf_);
            boost::system::error_code ec;
            head.set_content(is, ec);
            assert(!ec);
            return old_size - rcv_buf_.size();
        }

        size_t HttpSocket::read(
            HttpHead & head, 
            boost::system::error_code & ec)
        {
            boost::asio::read_until((super &)*this, rcv_buf_, "\r\n\r\n", ec);
            if (!ec) {
                size_t old_size = rcv_buf_.size();
                std::istream is(&rcv_buf_);
                head.set_content(is, ec);
                return old_size - rcv_buf_.size();
            }
            return 0;
        }

        boost::system::error_code HttpSocket::close(
            boost::system::error_code & ec)
        {
            snd_buf_.reset();
            rcv_buf_.reset();
            return super::close(ec);
        }

        void HttpSocket::set_source(
            HttpHead const & head)
      {
            assert(source_ == &stream_);
            boost::uint64_t content_length = head.content_length.get_value_or(boost::uint64_t(-1));
            if (content_length != boost::uint64_t(-1)) {
                source_ = new util::stream::LimitedSource(*source_, content_length);
            }
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
            boost::uint64_t content_length = head.content_length.get_value_or(boost::uint64_t(-1));
            if (content_length != boost::uint64_t(-1)) {
                util::stream::LimitedSource * limited_source = (util::stream::LimitedSource *)source_;
                source_ = &limited_source->source();
                delete limited_source;
            }
            assert(source_ == &stream_);
        }

        void HttpSocket::set_sink(
            HttpHead const & head)
        {
            assert(sink_ == &stream_);
            boost::uint64_t content_length = head.content_length.get_value_or(boost::uint64_t(-1));
            if (content_length != boost::uint64_t(-1)) {
                sink_ = new util::stream::LimitedSink(*sink_, content_length);
            }
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
            boost::uint64_t content_length = head.content_length.get_value_or(boost::uint64_t(-1));
            if (content_length != boost::uint64_t(-1)) {
                util::stream::LimitedSink * limited_sink = (util::stream::LimitedSink *)sink_;
                sink_ = &limited_sink->sink();
                delete limited_sink;
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
