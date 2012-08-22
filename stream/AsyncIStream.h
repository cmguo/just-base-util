// AsyncIStream.h

#ifndef _UTIL_TREAM_ASYNC_ISTREAM_H_
#define _UTIL_TREAM_ASYNC_ISTREAM_H_

#include <iostream>

#include <boost/asio.hpp>

#include "boost/iostreams/filtering_stream.hpp"
#include "boost/iostreams/categories.hpp"

#include "util/stream/StreamBuffers.h"
#include "util/stream/StreamHandler.h"
#include "util/stream/StreamErrors.h"
#include "util/stream/DummyFilter.h"
#include "util/stream/DeviceWrapper.h"

namespace util
{
    namespace stream
    {

        template < typename Ch, typename DeviceType >
        class async_basic_filtering_istream;

        template < typename Ch, typename DeviceType >
        class async_basic_filtering_istream_handler
        {
        public:
            typedef Ch   char_type;
            typedef void result_type;

        public:
            async_basic_filtering_istream_handler(
                async_basic_filtering_istream< Ch, DeviceType > * filter_stream,
                StreamMutableBuffers const & buffers,
                StreamHandler const & handler)
                : m_filtering_istream_(filter_stream)
                , m_buffers_(buffers)
                , m_handler_(handler)
                , m_bytes_transferred_(0)
            {}

            void start()
            {
                using namespace boost::asio;

                m_bytes_transferred_ = 0;
                m_iter_ = m_buffers_.begin();

                if (m_filtering_istream_->is_complete() && m_filtering_istream_->size() > 1) {
                    boost::asio::streambuf & recv_buf =
                        (m_filtering_istream_->component<basic_dummy_filter<char_type> >(m_filtering_istream_->size() - 2))->use_buffer();
                    (*m_filtering_istream_->component< DeviceType >(m_filtering_istream_->size() - 1))->async_read_some(
                        recv_buf.prepare(1024), boost::bind(boost::ref(*this), _1, _2));
                } else {
                    boost::system::error_code ec = util::stream::error::chain_is_not_complete;
                    m_filtering_istream_->get_io_service().post(
                        boost::asio::detail::bind_handler(m_handler_, ec, m_bytes_transferred_));
                    delete this;
                }
            }

            void operator()(
                boost::system::error_code const & ec,
                size_t bytes_transferred)
            {
                if (ec) {
                    m_filtering_istream_->get_io_service().post(
                        boost::asio::detail::bind_handler(m_handler_, ec, m_bytes_transferred_));
                    delete this;
                } else {
                    try {
                        m_filtering_istream_->read(m_buf_, m_size_);
                    } catch ( ... ) {
                        boost::system::error_code ec = util::stream::error::filter_source_error;
                        m_filtering_istream_->get_io_service().post(
                            boost::asio::detail::bind_handler(m_handler_, ec, m_bytes_transferred_));
                        delete this;
                        return;
                    }
                    // 将过滤后的buffer写到buffers中去，并返回
                    // todo something here
                    // bytes_transferred += ??
                    if (++m_iter_ == m_buffers_.end()) {
                        m_filtering_istream_->get_io_service().post(
                            boost::asio::detail::bind_handler(m_handler_, ec, m_bytes_transferred_));
                        delete this;
                    } else {
                        boost::asio::streambuf & recv_buf =
                            (m_filtering_istream_->component< basic_dummy_filter< char_type > >(m_filtering_istream_->size() - 2))->use_buffer();
                        (*m_filtering_istream_->component< DeviceType >(m_filtering_istream_->size() - 1))->async_read_some(
                            recv_buf.prepare(1024), boost::bind(boost::ref(*this), _1, _2));
                    }
                }
            }

        private:
            async_basic_filtering_istream< Ch, DeviceType > *   m_filtering_istream_;
            StreamMutableBuffers const &                        m_buffers_;
            StreamMutableBuffers::const_iterator                m_iter_;
            StreamHandler const                                 m_handler_;
            size_t                                              m_bytes_transferred_;
            char_type *                                         m_buf_;
            std::streamsize                                     m_size_;
        };

        template < typename Ch, typename DeviceType >
        class async_basic_filtering_istream
            : public boost::iostreams::filtering_istream
            , public Source
        {
        public:
            typedef Ch   char_type;
            typedef void result_type;

        public:
            async_basic_filtering_istream(boost::asio::io_service & ios)
                : Source(ios) {
                set_device_buffer_size(0);
                set_filter_buffer_size(0);
            }
            virtual ~async_basic_filtering_istream() {}            template < typename T >
            void push(const T & t, 
                typename T::category * = NULL)
            {
                using namespace boost::iostreams;
                typedef typename category_of<T>::type                             category;
                typedef typename boost::iostreams::detail::unwrap_ios<T>::type    policy_type;
                if (is_device<policy_type>::value) {
                    filtering_istream::push(
                        basic_dummy_filter< Ch >());
                }
                filtering_istream::push(t);
            }

            void push(
                Source & t)
            {
                using namespace boost::iostreams;
                filtering_istream::push(
                    basic_dummy_filter< char_type >());
                filtering_istream::push(basic_source_wrapper< char_type >(t));
            }
        private:
            // 内部filter设置类型为buffered_call
            virtual std::size_t private_read_some(
                StreamMutableBuffers const & buffers,
                boost::system::error_code & ec)
            {
                using namespace boost::asio;

                typedef StreamMutableBuffers::const_iterator const_iterator;
                std::size_t bytes_transferred = 0;

                if (is_complete() && size() > 1) {
                    (component< basic_dummy_filter< char_type > >(size() - 2))->set_call_type(
                        basic_dummy_filter< Ch >::buffered_call);

                    for (const_iterator iter = buffers.begin(); iter != buffers.end(); ++iter) {
                        try {
                            read(m_buf_, m_size_);
                            // 将过滤后的buffer写到buffers中去，并返回
                            // todo something here
                            // bytes_transferred += ??
                        } catch ( ... ) {
                            ec = util::stream::error::filter_source_error;
                            break;
                        }
                    }
                } else {
                    ec = util::stream::error::chain_is_not_complete;
                }

                return bytes_transferred;
            }

            // 内部filter设置类型为buffered_call
            virtual void private_async_read_some(
                StreamMutableBuffers const & buffers, 
                StreamHandler const & handler)
            {
                if (is_complete() && size() > 1) {
                    (component< basic_dummy_filter< char_type > >(size() - 2))->set_call_type(
                        basic_dummy_filter< Ch >::buffered_call);
                } else {
                    boost::system::error_code ec = util::stream::error::chain_is_not_complete;
                    get_io_service().post(
                        boost::asio::detail::bind_handler(handler, ec, 0));
                }
                async_basic_filtering_istream_handler< Ch, DeviceType > *process_handler = 
                    new async_basic_filtering_istream_handler< Ch, DeviceType >(this, buffers, handler);
                process_handler->start();
            }

        private:
            char_type *         m_buf_;
            std::streamsize     m_size_;
        };

        typedef async_basic_filtering_istream< char, basic_source_wrapper< char > > async_filtering_istream;

    } // stream
} // util

#endif // _UTIL_TREAM_ASYNC_ISTREAM_H_
