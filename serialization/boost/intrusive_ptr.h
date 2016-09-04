// intrusive_ptr.h

#ifndef _UTIL_SERIALIZATION_BOOST_INTRUSIVE_PTR_H_
#define _UTIL_SERIALIZATION_BOOST_INTRUSIVE_PTR_H_

#include <boost/intrusive_ptr.hpp>

namespace boost
{

    template<
        class _Ty
    >
    struct is_pointer<intrusive_ptr<_Ty> >
        : boost::true_type
    {
    };

} // namespace boost

#endif // _UTIL_SERIALIZATION_BOOST_INTRUSIVE_PTR_H_
