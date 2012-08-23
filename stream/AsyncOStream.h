// AsyncOStream.h

#ifndef _UTIL_TREAM_ASYNC_OSTREAM_H_
#define _UTIL_TREAM_ASYNC_OSTREAM_H_

#include <iostream>

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <boost/static_assert.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/categories.hpp>

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
        class async_basic_filtering_ostream;

        template < typename Ch, typename DeviceType >
        class async_basic_filtering_ostream_handler
        {
        public:
            typedef Ch          char_type;
            typedef void        result_type;
            typedef DeviceType  device_type;

        public:
            async_basic_filtering_ostream_handler(
                async_basic_filtering_ostream< char_type, device_type > * filter_stream,
                StreamConstBuffers const & buffers,
                StreamHandler const & handler)
                : m_filtering_ostream_(filter_stream)
                , m_buffers_(buffers)
                , m_handler_(handler)
                , m_bytes_transferred_(0)
            {}

            void start()
            {
                using namespace boost::asio;

                m_bytes_transferred_ = 0;
                typedef StreamConstBuffers::const_iterator const_iterator;

                if (m_filtering_ostream_->is_complete() && m_filtering_ostream_->size() > 1) {
                    (m_filtering_ostream_->component< basic_dummy_filter< char_type > >(m_filtering_ostream_->size() - 2))->set_call_type(
                        basic_dummy_filter< char_type >::buffered_call);
                    for (const_iterator iter = m_buffers_.begin(); iter != m_buffers_.end(); ++iter) {
                        try {
                            m_filtering_ostream_->write(
                                (const char *)boost::asio::detail::buffer_cast_helper(*iter),
                                boost::asio::detail::buffer_size_helper(*iter));
                        } catch ( ... ) {
                            // 记录过滤中间的错误码
                            m_ec_ = util::stream::error::filter_sink_error;
                            boost::asio::streambuf const & send_buf =
                                (m_filtering_ostream_->component< basic_dummy_filter< char_type > >(m_filtering_ostream_->size() - 2))->get_buffer();
                            // 中间出错，获取已经填充的过滤缓冲区一起发送发送
                            (*m_filtering_ostream_->component< device_type >(m_filtering_ostream_->size() - 1))->async_write_some(
                                send_buf.data(), boost::bind(boost::ref(*this), _1, _2));
                            return;
                        }
                        m_bytes_transferred_ += boost::asio::detail::buffer_size_helper(*iter);
                    }
                    boost::asio::streambuf const & send_buf =
                        (m_filtering_ostream_->component< basic_dummy_filter< char_type > >(m_filtering_ostream_->size() - 2))->get_buffer();
                    // 过滤完毕，获取已经填充的过滤缓冲区一起发送发送
                    (*m_filtering_ostream_->component< device_type >(m_filtering_ostream_->size() - 1))->async_write_some(
                        send_buf.data(), boost::bind(boost::ref(*this), _1, _2));
                } else {
                    boost::system::error_code ec = util::stream::error::chain_is_not_complete;
                    m_filtering_ostream_->get_io_service().post(
                        boost::asio::detail::bind_handler(m_handler_, ec, 0));
                    delete this;
                }
            }

            void operator()(
                boost::system::error_code const & ec,
                size_t bytes_transferred)
            {
                boost::asio::streambuf & send_buf =
                    (m_filtering_ostream_->component< basic_dummy_filter< char_type > >(m_filtering_ostream_->size() - 2))->use_buffer();
                send_buf.commit(bytes_transferred);

                if (ec) { // 发送出错，优先将网络错误码抛出
                    m_filtering_ostream_->get_io_service().post(
                        boost::asio::detail::bind_handler(m_handler_, ec, 0));
                } else {
                    if (m_ec_) {
                        // 抛出过滤错误，发送的字节数以处理前为准
                        m_filtering_ostream_->get_io_service().post(
                            boost::asio::detail::bind_handler(m_handler_, m_ec_, m_bytes_transferred_));
                    } else {
                        m_filtering_ostream_->get_io_service().post(
                            boost::asio::detail::bind_handler(m_handler_, ec, m_bytes_transferred_));
                    }
                }

                delete this;
            }

        private:
            async_basic_filtering_ostream< char_type, device_type > *   m_filtering_ostream_;
            StreamConstBuffers const                            m_buffers_;
            StreamHandler const                                 m_handler_;
            size_t                                              m_bytes_transferred_;
            boost::system::error_code                           m_ec_;
        };

        template < typename Ch, typename DeviceType >
        class async_basic_filtering_ostream
            : public boost::iostreams::filtering_ostream
            , public Sink
        {
        public:
            typedef Ch          char_type;
            typedef void        result_type;
            typedef DeviceType  device_type;

        public:
            async_basic_filtering_ostream(
                boost::asio::io_service & ios)
                : Sink(ios)
            {
                    set_device_buffer_size(0);
                    set_filter_buffer_size(0);
            }
            virtual ~async_basic_filtering_ostream() {}

            template < typename T >
            void push(T & t,
                typename T::category * = NULL)
            {
                using namespace boost::iostreams;
                typedef typename boost::iostreams::detail::unwrap_ios<T>::type    policy_type;
                if (is_device<policy_type>::value) {
                    filtering_ostream::push(
                        basic_dummy_filter< char_type >());
                }
                filtering_ostream::push(t);
            }

            void push(
                Sink & t)
            {
                using namespace boost::iostreams;
                filtering_ostream::push(
                    basic_dummy_filter< char_type >());
                filtering_ostream::push(basic_sink_wrapper< char_type >(t));
            }

        private:
            virtual size_t private_write_some(
                StreamConstBuffers const & buffers, 
                boost::system::error_code & ec)
            {
                using namespace boost::asio;

                typedef StreamConstBuffers::const_iterator const_iterator;
                std::size_t bytes_transferred = 0;

                if (is_complete() && size() > 1) {
                    (component< basic_dummy_filter< char_type > >(size() - 2))->set_call_type(
                        basic_dummy_filter< char_type >::buffered_call);
                    for (const_iterator iter = buffers.begin(); iter != buffers.end(); ++iter) {
                        try {
                            write(
                                (const char *)boost::asio::detail::buffer_cast_helper(*iter),
                                boost::asio::detail::buffer_size_helper(*iter));
                            flush();
                        } catch ( ... ) {
                            ec = util::stream::error::filter_sink_error;
                            break;
                        }
                        bytes_transferred += boost::asio::detail::buffer_size_helper(*iter);
                    }
                } else {
                    ec = util::stream::error::chain_is_not_complete;
                }

                return bytes_transferred;
            }

            // 内部filter设置类型为buffered_call
            virtual void private_async_write_some(
                StreamConstBuffers const & buffers, 
                StreamHandler const & handler)
            {
                async_basic_filtering_ostream_handler< char_type, device_type > *process_handler =
                    new async_basic_filtering_ostream_handler< char_type, device_type >(this, buffers, handler);
                process_handler->start();
            }

        };

        typedef async_basic_filtering_ostream<char, basic_sink_wrapper< char> > async_filtering_ostream;

    } // stream
} // util

#endif // _UTIL_TREAM_ASYNC_OSTREAM_H_
