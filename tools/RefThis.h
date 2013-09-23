// RefThis.h

#ifndef _UTIL_TOOLS_REF_THIS_H_
#define _UTIL_TOOLS_REF_THIS_H_

#include <boost/intrusive_ptr.hpp>

namespace util
{
    namespace tools
    {

        template<
            typename _Ty
        >
        class RefThis
        {
        public:
            typedef boost::intrusive_ptr<_Ty> pointer;

            typedef boost::intrusive_ptr<_Ty const> const_pointer;

        protected:
            RefThis()
                : nref_(0)
            {
                _internal_weak_this = static_cast<_Ty *>(this);
            }

            ~RefThis()
            {
            }

        protected:
            pointer ref_this()
            {
                boost::intrusive_ptr<_Ty> p(_internal_weak_this);
                BOOST_ASSERT(p.get() == this);
                return p;
            }

            const_pointer ref_this() const
            {
                const_pointer p(_internal_weak_this);
                BOOST_ASSERT(p.get() == this);
                return p;
            }

            template<
                typename _Ty1
            >
            boost::intrusive_ptr<_Ty1> ref(
                _Ty1 * p)
            {
                assert(p == this);
                return p;
            }

            template<
                typename _Ty1
            >
            boost::intrusive_ptr<_Ty1 const> ref(
                _Ty1 const * p) const
            {
                assert(p == this);
                return p;
            }

        private:
            // not copyable
            RefThis(
                RefThis const &);

            RefThis & operator=(
                RefThis const &);

        private:
            friend void intrusive_ptr_add_ref(
                RefThis const * p)
            {
                ++p->nref_;
            }

            friend void intrusive_ptr_release(
                RefThis const * p)
            {
                if (--p->nref_ == 0) {
                    delete static_cast<_Ty const *>(p);
                }
            }

        private:
            mutable size_t nref_;
            mutable _Ty * _internal_weak_this;
        };
        
    } // namespace tools
} // namespace util

#endif // _UTIL_TOOLS_REF_THIS_H_
