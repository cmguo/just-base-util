// DummyFilter.h

#ifndef _UTIL_STREAM_DUMMY_FILTER_H_
#define _UTIL_STREAM_DUMMY_FILTER_H_

#include <boost/iostreams/categories.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/asio/streambuf.hpp>

namespace util
{
    namespace stream
    {

        template < typename Ch >
        class basic_dummy_filter
        {
        public:
            typedef Ch                      char_type;
            typedef size_t                  size_type;
            typedef boost::asio::streambuf  buffer_type;
            struct category
                : boost::iostreams::dual_use,
                boost::iostreams::filter_tag,
                boost::iostreams::multichar_tag
            {};

        public:
            basic_dummy_filter()
                : is_async_(false)
                , eof_(false)
                , m_data_(new buffer_type)
            {
            }

            ~basic_dummy_filter()
            {
            }

        public:
            template< typename Source >
            std::streamsize read(
                Source & src, 
                char_type * s, 
                std::streamsize n)
            {
                if (m_data_->size() > 0) {
                    std::streamsize amt = m_data_->sgetn(s, n);
                    return amt;
                } else if (eof_) {
                    return std::char_traits<char_type>::eof();
                } else if (!is_async_) {
                    return boost::iostreams::read(src, s, n);
                } else {
                    return 0;
                }
            }

            template<typename Sink>
            std::streamsize write(
                Sink & snk, 
                const char_type * s, 
                std::streamsize n)
            {
                if (is_async_) {
                    n = m_data_->sputn(s, n);
                } else {
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
                }
                return n;
            }

            template<typename Sink>
            void close(
                Sink & sink, 
                BOOST_IOS::openmode which)
            {
            }

        public:
            const buffer_type & get_buffer() const
            {
                return *m_data_;
            }

            buffer_type & use_buffer()
            {
                return *m_data_;
            }

            void begin_async()
            {
                is_async_ = true;
            }

            void end_async()
            {
                is_async_ = false;
            }

            void set_eof()
            {
                eof_ = true;
            }

            bool is_eof() const
            {
                return eof_;
            }

        private:
            bool is_async_;
            bool eof_;
            boost::shared_ptr<buffer_type> m_data_;
        };

        typedef basic_dummy_filter< char >    MDummyFilter;
        typedef basic_dummy_filter< wchar_t > WDummyFilter;

    } // namespace stream
} // namespace util

#endif // _UTIL_STREAM_DUMMY_FILTER_H_
