// DummyFilter.h

#ifndef _UTIL_STREAM_DUMMY_FILTER_H_
#define _UTIL_STREAM_DUMMY_FILTER_H_

#include "boost/iostreams/categories.hpp"
#include "boost/iostreams/filtering_streambuf.hpp"
#include "boost/iostreams/filtering_stream.hpp"

#include "boost/shared_ptr.hpp"

namespace util
{
    namespace stream
    {

        template < typename Ch >
        class basic_dummy_filter {
        public:
            typedef Ch char_type;
            typedef size_t size_type;
            typedef boost::asio::streambuf buffer_type;
            struct category
                : boost::iostreams::dual_use,
                boost::iostreams::filter_tag,
                boost::iostreams::multichar_tag
            {};

        public:
            typedef enum {
                buffered_call,
                unbuffered_call
            } call_type;

            enum flag_type {
                f_read   = 1,
                f_write  = f_read << 1
            };

        public:
            basic_dummy_filter(call_type type = buffered_call)
                : m_call_type_(type)
                , m_data_(new buffer_type)
                , m_cur_(0) {}

            ~basic_dummy_filter() {
                close_impl();
            }

            void reset() {
            }

            template< typename Source >
            std::streamsize read(Source & src, char_type * s, std::streamsize n) {
                BOOST_ASSERT(!(m_state_ & f_write));
                m_state_ |= f_read;
                if (m_call_type_ == unbuffered_call && m_data_->size() == 0) {
                    return boost::iostreams::read(src, s, n);
                } else {
                    std::streamsize amt = m_data_->sgetn(s, n);
                    m_data_->consume(amt);
                    return amt;
                }
            }

            template<typename Sink>
            std::streamsize write(Sink & snk, const char_type * s, std::streamsize n) {
                BOOST_ASSERT(!(m_state_ & f_read));
                m_state_ |= f_write;
                if (m_call_type_ == unbuffered_call) {
                    if (m_data_->size() > 0) {
                        n = boost::iostreams::write(
                            snk, boost::asio::buffer_cast<char_type const *>(m_data_->data()), m_data_->size());
                        m_data_->consume(n);
                    }
                    if (m_data_->size() > 0) {
                        n = 0;
                    } else {
                        n = boost::iostreams::write(snk, s, n);
                    }
                } else {
                    n = m_data_->sputn(s, n);
                    m_data_->commit(n);
                }
                return n;
            }

            template<typename Sink>
            void close(Sink & sink, BOOST_IOS::openmode which) {
                close_impl();
            }

            void set_calltype(call_type type) {
                m_call_type_ = type;
            }

            const buffer_type & get_buffer() const {
                return *m_data_;
            }

            buffer_type & use_buffer() {
                return *m_data_;
            }

            call_type get_call_type() const {
                return m_call_type_;
            }

            void set_call_type(call_type calltype) {
                m_call_type_ = calltype;
            }

        private:
            void close_impl() {
                m_state_ = 0;
            }

        private:
            call_type                       m_call_type_;
            boost::shared_ptr<buffer_type>  m_data_;
            size_type                       m_cur_; // current read or write position
            int                             m_state_;
        };

        typedef basic_dummy_filter< char >    MDummyFilter;
        typedef basic_dummy_filter< wchar_t > WDummyFilter;

    } // namespace stream
} // namespace util

#endif // _UTIL_STREAM_DUMMY_FILTER_H_
