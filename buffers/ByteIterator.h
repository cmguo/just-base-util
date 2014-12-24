// ByteIterator.h

#ifndef _UTIL_BUFFERS_BYTE_TERATOR_H_
#define _UTIL_BUFFERS_BYTE_TERATOR_H_

#include <boost/iterator/iterator_facade.hpp>

namespace util
{
    namespace buffers
    {

        template <typename BufferIterator>
        struct is_const_buffer_impl
        {
            static boost::type_traits::no_type check(...);

            //static boost::type_traits::no_type check(
            //    boost::asio::mutable_buffers_1);

            static boost::type_traits::yes_type check(
                boost::asio::const_buffers_1);

            BOOST_STATIC_CONSTANT(unsigned, s = 
                sizeof(is_const_buffer_impl<BufferIterator>::check(boost::asio::buffer(**(BufferIterator *)0))));

            BOOST_STATIC_CONSTANT(bool, value = (s == sizeof(boost::type_traits::yes_type)));
        };

        template <typename BufferIterator>
        struct is_const_buffer
            : boost::integral_constant<bool, is_const_buffer_impl<BufferIterator>::value>
        {
            using boost::integral_constant<bool, is_const_buffer_impl<BufferIterator>::value>::value;
        };

        template<
            typename BufferIterator
        >
        struct BufferIteratorTraits
        {
            typedef typename boost::mpl::if_<
                is_const_buffer<BufferIterator>, 
                boost::asio::const_buffer, 
                boost::asio::mutable_buffer>::type buffer_type;
            typedef typename boost::mpl::if_<
                is_const_buffer<BufferIterator>, 
                char const, 
                char>::type byte_type;
        };

        template <
            typename Derived,
            typename BufferIterator
        >
        class ByteIteratorT
            : public boost::iterator_facade<
                Derived,
                typename BufferIteratorTraits<BufferIterator>::byte_type, 
                boost::random_access_traversal_tag
            >
        {
        public:
            typedef typename BufferIteratorTraits<BufferIterator>::buffer_type buffer_t;
            typedef typename BufferIteratorTraits<BufferIterator>::byte_type byte_t;

        public:
            ByteIteratorT()
            {
            }

            ByteIteratorT(
                BufferIterator const & iter)
                : iter_(iter)
            {
            }

            template <typename Derived2>
            ByteIteratorT(
                ByteIteratorT<Derived2, BufferIterator> const & r)
                : iter_(r.buffer_iter())
            {
                dervied().set_offset(r.buffer_offset());
            }

        public:
            template <typename Derived2>
            ByteIteratorT & operator=(
                ByteIteratorT<Derived2, BufferIterator> const & r)
            {
                iter_ = r.buffer_iter();
                dervied().set_offset(r.buffer_offset());
                return *this;
            }

            void reset(
                BufferIterator iter)
            {
                iter_ = iter;
                dervied().set_offset(0);
            }

        public:
            template <typename Derived2>
            friend bool operator==(
                ByteIteratorT const & l, 
                ByteIteratorT<Derived2, BufferIterator> const & r)
            {
                return l.buffer_iter() == r.buffer_iter()
                    && l.buffer_offset() == r.buffer_offset();
            }

            template <typename Derived2>
            friend bool operator!=(
                ByteIteratorT const & l, 
                ByteIteratorT<Derived2, BufferIterator> const & r)
            {
                return !(l == r);
            }

        public:
            byte_t * buffer_ptr() const
            {
                return boost::asio::buffer_cast<byte_t *>(*iter_);
            }

            size_t buffer_size() const
            {
                return boost::asio::buffer_size(*iter_);
            }

            buffer_t buffer() const
            {
                return boost::asio::buffer(*iter_);
            }

            BufferIterator buffer_iter() const
            {
                return iter_;
            }

            size_t buffer_offset() const
            {
                return dervied().get_offset();
            }

            void increment_buffer()
            {
                ++iter_;
                dervied().set_offset(0);
            }

        protected:
            template <typename Derived2>
            void increment(
                ByteIteratorT<Derived2, BufferIterator> const & limit)
            {
                size_t off = dervied().get_offset();
                BufferIterator iter2 = limit.buffer_iter();
                size_t off2 = limit.buffer_offset();
                if (iter_ == iter2 && off == off2) {
                    assert(false);
                    return;
                }
                ++off;
                if (iter_ == iter2) {
                    dervied().set_offset(off);
                    return;
                }
                while (boost::asio::buffer_size(*iter_) == off) {
                    ++iter_;
                    off = 0;
                    if (iter_ == iter2) {
                        break;
                    }
                }
            }

            template <typename Derived2>
            size_t advance(
                ByteIteratorT<Derived2, BufferIterator> const & limit, 
                size_t n)
            {
                size_t off = dervied().get_offset();
                BufferIterator iter2 = limit.buffer_iter();
                size_t off2 = limit.buffer_offset();
                if (iter_ == iter2 && off + n > off2) {
                    assert(false);
                    return 0;
                }
                off += n;
                if (iter_ == iter2) {
                    dervied().set_offset(off);
                    return n;
                }
                size_t this_size = boost::asio::buffer_size(*iter_);
                while (off >= this_size) {
                    ++iter_;
                    off -= this_size;
                    if (iter_ == iter2) {
                        if (off > off2) {
                            assert(false);
                            return 0;
                        }
                        break;
                    }
                    this_size = boost::asio::buffer_size(*iter_);
                }
                dervied().set_offset(off);
                return n;
            }

            template <typename Derived2>
            bool equal(
                ByteIteratorT<Derived2, BufferIterator> const & r) const
            {
                return iter_ == r.iter_ 
                    && dervied().get_offset() == r.buffer_offset();
            }

        private:
            size_t get_offset() const
            {
                return 0;
            }

            void set_offset(size_t)
            {
            }

            Derived const & dervied() const
            {
                return static_cast<Derived const &>(*this);
            }

            Derived & dervied()
            {
                return static_cast<Derived &>(*this);
            }

        private:
            friend class boost::iterator_core_access;

            void increment()
            {
                size_t off = dervied().get_offset() + 1;
                if (boost::asio::buffer_size(*iter_) == off) {
                    ++iter_;
                    dervied().set_offset(0);
                }
            }

            void advance(
                size_t n)
            {
                size_t off = dervied().get_offset() + n;
                size_t this_size = boost::asio::buffer_size(*iter_);
                while (off > this_size) {
                    ++iter_;
                    off -= this_size;
                    this_size = boost::asio::buffer_size(*iter_);
                }
                dervied().set_offset(off);
            }

            bool equal(
                ByteIteratorT const & r) const
            {
                return iter_ == r.iter_ && dervied().get_offset() == r.dervied().get_offset();
            }

            byte_t & dereference() const
            {
                return boost::asio::buffer_cast<byte_t *>(*iter_)[dervied().get_offset()];
            }

        private:
            BufferIterator iter_;
        };

        template <
            typename BufferIterator
        >
        class ByteIterator
            : public ByteIteratorT<ByteIterator<BufferIterator>, BufferIterator>
        {
        public:
            typedef ByteIteratorT<ByteIterator<BufferIterator>, BufferIterator> super;

        public:
            ByteIterator(
                BufferIterator const & iter, 
                size_t off = 0)
                : super(iter)
                , off_(off)
            {
            }

            template <typename Derived2>
            ByteIterator(
                ByteIteratorT<Derived2, BufferIterator> const & r)
                : super(r)
            {
            }

        private:
            friend class ByteIteratorT<ByteIterator<BufferIterator>, BufferIterator>;

            size_t get_offset() const
            {
                return off_;
            }

            void set_offset(
                size_t o)
            {
                off_ = o;
            }

        private:
            size_t off_;
        };

    } // namespace mux
} // namespace ppbox

#endif // _UTIL_BUFFERS_BYTE_TERATOR_H_
