// StlBuffer.h

#ifndef _UTIL_BUFFERS_STL_BUFFER_H_
#define _UTIL_BUFFERS_STL_BUFFER_H_

#include <boost/asio/buffer.hpp>

#include <streambuf>

namespace util
{
    namespace buffers
    {

        template <
            class _Witch
        >
        struct WitchVoidPointer;

        template <
            class _Witch
        >
        struct WitchBuffer;

        template <
            class _Witch
        >
        struct WitchBuffers;

        template <
            class _Witch
        >
        struct SwitchWitch;

        namespace detail
        {
            struct _write{};
            struct _read{};
        }

        template <
        >
        struct WitchVoidPointer<detail::_read>
        {
            typedef void const * type;
        };

        template <
        >
        struct WitchVoidPointer<detail::_write>
        {
            typedef void * type;
        };

        template <
        >
        struct WitchBuffer<detail::_read>
        {
            typedef boost::asio::const_buffer type;
        };

        template <
        >
        struct WitchBuffer<detail::_write>
        {
            typedef boost::asio::mutable_buffer type;
        };

        template <
        >
        struct WitchBuffers<detail::_read>
        {
            typedef boost::asio::const_buffers_1 type;
        };

        template <
        >
        struct WitchBuffers<detail::_write>
        {
            typedef boost::asio::mutable_buffers_1 type;
        };

        template <
        >
        struct SwitchWitch<detail::_write>
        {
            typedef detail::_read type;
        };

        template <
        >
        struct SwitchWitch<detail::_read>
        {
            typedef detail::_write type;
        };

        class StlBufferAccees
        {
            template <
                class _Buffer, 
                class _Witch
            >
            friend class StlBuffer;

            template <
                class _Buffer, 
                class _Witch
            >
            static void set_buffer(
               _Buffer & buf, 
               typename _Buffer::char_type * beg, 
               typename _Buffer::char_type * cur, 
               typename _Buffer::char_type * end, 
               _Witch const & witch)
            {
                buf.set_buffer(beg, cur, end, witch);
            }

            template <
                class _Buffer, 
                class _Witch
            >
            static void get_buffer(
                _Buffer & buf, 
                typename _Buffer::char_type *& beg, 
                typename _Buffer::char_type *& cur, 
                typename _Buffer::char_type *& end, 
                _Witch const & witch)
            {
                buf.get_buffer(beg, cur, end, witch);
            }
        };

        template <
            typename Elem = char, 
            typename Traits = std::char_traits<Elem>
        >
        class StlStream
            : public std::basic_streambuf<Elem, Traits>
        {
        public:
            typedef typename std::basic_streambuf<Elem, Traits>::char_type char_type;

        private:
            friend class StlBufferAccees;

            void set_buffer(
                char_type * beg, 
                char_type * cur, 
                char_type * end, 
                detail::_write const &)
            {
                this->setp(beg, end);
                this->pbump(static_cast<int>(cur - beg));
            }

            void set_buffer(
                char_type * beg, 
                char_type * cur, 
                char_type * end, 
                detail::_read const &)
            {
                this->setg(beg, cur, end);
            }

            void get_buffer(
                char_type *& beg, 
                char_type *& cur, 
                char_type *& end, 
                detail::_write const &)
            {
                beg = this->pbase();
                cur = this->pptr();
                end = this->epptr();
            }

            void get_buffer(
                char_type *& beg, 
                char_type *& cur, 
                char_type *& end, 
                detail::_read const &)
            {
                beg = this->eback();
                cur = this->gptr();
                end = this->egptr();
            }

        };

        template <
            class _Buffer, 
            class _Witch
        >
        class StlBuffer
            : public _Witch
        {
        public:
            typedef _Buffer buffer_type;
            typedef typename _Buffer::char_type char_type;
            typedef typename _Buffer::traits_type traits_type;
            typedef typename WitchVoidPointer<_Witch>::type pointer_type;

            StlBuffer(
                _Buffer & stl_buf)
                : stl_buf_(stl_buf)
                , end_(NULL)
            {
                char_type * p = NULL;
                StlBufferAccees::set_buffer(stl_buf_, p, p, p, *this);
            }

            StlBuffer(
                _Buffer & stl_buf, 
                StlBuffer const & other)
                : stl_buf_(stl_buf)
                , end_(other.end_)
            {
                char_type * beg = 0;
                char_type * cur = 0;
                char_type * end = 0;
                StlBufferAccees::get_buffer(other.stl_buf_, beg, cur, end, other);
                StlBufferAccees::set_buffer(stl_buf_, beg, cur, end, *this);
            }

            friend class StlBuffer<_Buffer, typename SwitchWitch<_Witch>::type>;

            StlBuffer(
                _Buffer & stl_buf, 
                StlBuffer<_Buffer, typename SwitchWitch<_Witch>::type> & other)
                : stl_buf_(stl_buf)
                , end_(other.end_)
            {
                char_type * beg = 0;
                char_type * cur = 0;
                char_type * end = 0;
                StlBufferAccees::get_buffer(other.stl_buf_, beg, cur, end, other);
                end_ = other.end_;
                StlBufferAccees::set_buffer(stl_buf_, beg, cur, end, *this);
                end = cur;
                StlBufferAccees::set_buffer(other.stl_buf_, beg, cur, end, other);
            }

            StlBuffer & operator=(
                StlBuffer const & other) const
            {
                char_type * beg = 0;
                char_type * cur = 0;
                char_type * end = 0;
                StlBufferAccees::get_buffer(other.stl_buf_, beg, cur, end, other);
                StlBufferAccees::set_buffer(stl_buf_, beg, cur, end, *this);
                end_ = other.end_;
                return *this;
            }
/*
            StlBuffer & operator=(
                StlBuffer<_Buffer, typename SwitchWitch<_Witch>::type> & other)
            {
                char_type * beg = 0;
                char_type * cur = 0;
                char_type * end = 0;
                StlBufferAccees::get_buffer(other.stl_buf_, beg, cur, end, other);
                end = other.end_;
                StlBufferAccees::set_buffer(stl_buf_, beg, cur, end, *this);
                end_ = other.end_;
                end = cur;
                StlBufferAccees::set_buffer(other.stl_buf_, beg, cur, end, other);
                return *this;
            }
*/
            void operator=(
                boost::asio::mutable_buffer const & buf)
            {
                char_type * beg = boost::asio::buffer_cast<char_type *>(buf);
                char_type * cur = beg;
                char_type * end = beg + boost::asio::buffer_size(buf);
                StlBufferAccees::set_buffer(stl_buf_, beg, cur, end, *this);
                end_ = end;
            }

            void operator=(
                boost::asio::const_buffer const & buf)
            {
                char_type * beg = (char_type *)boost::asio::buffer_cast<char_type const *>(buf);
                char_type * cur = beg;
                char_type * end = beg + boost::asio::buffer_size(buf);
                StlBufferAccees::set_buffer(stl_buf_, beg, cur, end, *this);
                end_ = end;
            }

            void consume(
                std::size_t n)
            {
                char_type * beg = 0;
                char_type * cur = 0;
                char_type * end = 0;
                StlBufferAccees::get_buffer(stl_buf_, beg, cur, end, *this);
                cur += n;
                assert(cur <= end);
                StlBufferAccees::set_buffer(stl_buf_, beg, cur, end, *this);
            }

            std::size_t drop_back()
            {
                char_type * beg = 0;
                char_type * cur = 0;
                char_type * end = 0;
                StlBufferAccees::get_buffer(stl_buf_, beg, cur, end, *this);
                std::size_t n = cur - beg;
                beg = cur;
                StlBufferAccees::set_buffer(stl_buf_, beg, cur, end, *this);
                return n;
            }

            void commit(
                std::size_t n)
            {
                char_type * beg = 0;
                char_type * cur = 0;
                char_type * end = 0;
                StlBufferAccees::get_buffer(stl_buf_, beg, cur, end, *this);
                end += n;
                if (end > end_)
                    end_ = end;
                StlBufferAccees::set_buffer(stl_buf_, beg, cur, end, *this);
            }
           
            void limit_size(
                std::size_t n)
            {
                char_type * beg = 0;
                char_type * cur = 0;
                char_type * end = 0;
                StlBufferAccees::get_buffer(stl_buf_, beg, cur, end, *this);
                if (cur + n < end)
                    end = cur + n;
                StlBufferAccees::set_buffer(stl_buf_, beg, cur, end, *this);
            }

            char_type * ptr() const
            {
                char_type * beg = 0;
                char_type * cur = 0;
                char_type * end = 0;
                StlBufferAccees::get_buffer(stl_buf_, beg, cur, end, *this);
                (void)beg;
                (void)end;
                return cur;
            }

            std::size_t offset() const
            {
                char_type * beg = 0;
                char_type * cur = 0;
                char_type * end = 0;
                StlBufferAccees::get_buffer(stl_buf_, beg, cur, end, *this);
                (void)end;
                return cur - beg;
            }

            std::size_t size() const
            {
                char_type * beg = 0;
                char_type * cur = 0;
                char_type * end = 0;
                StlBufferAccees::get_buffer(stl_buf_, beg, cur, end, *this);
                (void)beg;
                return end - cur;
            }

            std::size_t total() const
            {
                char_type * beg = 0;
                char_type * cur = 0;
                char_type * end = 0;
                StlBufferAccees::get_buffer(stl_buf_, beg, cur, end, *this);
                (void)cur;
                return end - beg;
            }

            std::size_t split(
                boost::asio::mutable_buffer const & buf, 
                StlBuffer<_Buffer, typename SwitchWitch<_Witch>::type> & other)
            {
                char_type * buf_beg = boost::asio::buffer_cast<char_type *>(buf);
                char_type * buf_end = buf_beg + boost::asio::buffer_size(buf);
                char_type * beg = 0;
                char_type * cur = 0;
                char_type * end = 0;
                StlBufferAccees::get_buffer(stl_buf_, beg, cur, end, *this);
                assert(buf_beg <= beg && end <= buf_end);
                std::size_t off = beg - buf_beg;
                beg = buf_beg;
                end = cur;
                StlBufferAccees::set_buffer(stl_buf_, beg, cur, end, *this);
                end_ = end;
                beg = cur;
                end = buf_end;
                StlBufferAccees::set_buffer(other.stl_buf_, beg, cur, end, other);
                other.end_ = end;
                return off;
            }

            bool before(
                StlBuffer<_Buffer, typename SwitchWitch<_Witch>::type> const & other) const
            {
                char_type * beg1 = 0;
                char_type * cur1 = 0;
                char_type * end1 = 0;
                StlBufferAccees::get_buffer(stl_buf_, beg1, cur1, end1, *this);
                char_type * beg2 = 0;
                char_type * cur2 = 0;
                char_type * end2 = 0;
                StlBufferAccees::get_buffer(other.stl_buf_, beg2, cur2, end2, other);
                (void)cur1;
                (void)cur2;
                assert(beg1 == end2 || beg2 == end1);
                return beg1 < beg2 || end1 < end2;
            }

            bool operator==(
                StlBuffer const & r) const
            {
                StlBuffer const & l = *this;
                char_type * l_beg = 0;
                char_type * l_cur = 0;
                char_type * l_end = 0;
                StlBufferAccees::get_buffer(l.stl_buf_, l_beg, l_cur, l_end, l);
                char_type * r_beg = 0;
                char_type * r_cur = 0;
                char_type * r_end = 0;
                StlBufferAccees::get_buffer(r.stl_buf_, r_beg, r_cur, r_end, r);
                return l_cur == r_cur && l_end == r_end;
            }
            
            operator typename WitchBuffer<_Witch>::type() const
            {
                char_type * beg = 0;
                char_type * cur = 0;
                char_type * end = 0;
                StlBufferAccees::get_buffer(stl_buf_, beg, cur, end, *this);
                (void)beg;
                return typename WitchBuffer<_Witch>::type(cur, end - cur);
            }

        private:
            _Buffer & stl_buf_;
            char_type * end_;
        };

    }
}

namespace boost
{
    namespace asio
    {
        namespace detail
        {
            template <
                class _Buffer, 
                class _Witch
            >
            inline typename util::buffers::WitchVoidPointer<_Witch>::type buffer_cast_helper(
                util::buffers::StlBuffer<_Buffer, _Witch> const & b)
            {
                return b.ptr();
            }

            template <
                class _Buffer, 
                class _Witch
            >
            inline std::size_t buffer_size_helper(
                util::buffers::StlBuffer<_Buffer, _Witch> const & b)
            {
                return b.size();
            }
        }

        template <
            typename PointerToPodType, 
            class _Buffer, 
            class _Witch
        >
        inline PointerToPodType buffer_cast(
            util::buffers::StlBuffer<_Buffer, _Witch> const & b)
        {
            return static_cast<PointerToPodType>(detail::buffer_cast_helper(b));
        }

        template <
            class _Buffer, 
            class _Witch
        >
        inline std::size_t buffer_size(
            util::buffers::StlBuffer<_Buffer, _Witch> const & b)
        {
            return detail::buffer_size_helper(b);
        }

        template <
            class _Buffer, 
            class _Witch
        >
        inline typename util::buffers::WitchBuffers<_Witch>::type buffer(
            const util::buffers::StlBuffer<_Buffer, _Witch> & b)
        {
            return typename util::buffers::WitchBuffers<_Witch>::type(
                detail::buffer_cast_helper(b), buffer_size_helper(b));
        }

        template <
            class _Buffer, 
            class _Witch
        >
        inline typename util::buffers::WitchBuffers<_Witch>::type buffer(
            const util::buffers::StlBuffer<_Buffer, _Witch> & b, 
            std::size_t max_size_in_bytes)
        {
            return typename util::buffers::WitchBuffers<_Witch>::type(
                typename util::buffers::WitchBuffer<_Witch>::type(detail::buffer_cast_helper(b), buffer_size_helper(b)), max_size_in_bytes);
        }
    }
}

#endif // _UTIL_BUFFERS_STL_BUFFER_H_
