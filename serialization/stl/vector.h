// vector.h

#ifndef _UTIL_SERIALIZATION_STL_VECTOR_H_
#define _UTIL_SERIALIZATION_STL_VECTOR_H_

#include "util/serialization/Collection.h"
#include "util/serialization/SplitMember.h"
#include "util/serialization/Array.h"

#include <boost/mpl/if.hpp>

#include <vector>

namespace util
{
    namespace serialization
    {

        template<
            class Archive, 
            class _Ty, 
            class _Alloc
        >
        struct is_collection<Archive, std::vector<_Ty, _Alloc> >
            : boost::true_type
        {
        };

    }  // namespace serialization
} // namespace util

namespace std
{

    template<
        class Size,
        class Archive, 
        class _Ty, 
        class _Alloc
    >
    inline void save(
        Archive & ar,
        std::vector<_Ty, _Alloc> const & t,
        Size n,
        boost::mpl::false_)
    {
        util::serialization::save_collection(ar, t, n);
    }

    template<
        class Size,
        class Archive, 
        class _Ty, 
        class _Alloc
    >
    inline void load(
        Archive & ar,
        std::vector<_Ty, _Alloc> & t,
        Size n,
        boost::mpl::false_)
    {
        util::serialization::load_collection(ar, t, n);
    }

    // the optimized versions

    template<
        class Size,
        class Archive, 
        class _Ty, 
        class _Alloc
    >
    inline void save(
        Archive & ar,
        std::vector<_Ty, _Alloc> const & t,
        Size n,
        boost::mpl::true_)
    {
        if (!t.empty())
            ar.save_array(framework::container::make_array(&t[0], n));
    }

    template<
        class Size,
        class Archive, 
        class _Ty, 
        class _Alloc
    >
    inline void load(
        Archive & ar,
        std::vector<_Ty, _Alloc> &t,
        Size n,
        boost::mpl::true_)
    {
        t.clear();
        Size l(0);
        if (n > 1024) {
            for (; n > 1024; l += 1024, n -= 1024) {
                t.resize(l + 1024);
                ar.load_array(framework::container::make_array(&t[l], 1024));
                if (!ar) return;
            }
        }
        if (n) {
            t.resize(l + n);
            ar.load_array(framework::container::make_array(&t[l], n));
        }
    }

    // dispatch to either default or optimized versions

    template<
        class Size,
        class Archive, 
        class _Ty, 
        class _Alloc
    >
    inline void save(
        Archive & ar,
        std::vector<_Ty, _Alloc> const & t, 
        Size n)
    {
        typedef BOOST_DEDUCED_TYPENAME boost::mpl::if_<
            util::serialization::use_array_optimization<Archive, _Ty>, 
            boost::mpl::true_, 
            boost::mpl::false_>::type use_optimized;
        save(ar, t, n, use_optimized());
    }

    template<
        class Size,
        class Archive, 
        class _Ty, 
        class _Alloc
    >
    inline void load(
        Archive & ar,
        std::vector<_Ty, _Alloc> & t, 
        Size n)
    {
        typedef BOOST_DEDUCED_TYPENAME boost::mpl::if_<
            util::serialization::use_array_optimization<Archive, _Ty>, 
            boost::mpl::true_, 
            boost::mpl::false_>::type use_optimized;
        load(ar, t, n, use_optimized());
    }

    template<
        class Size,
        class Archive, 
        class _Ty, 
        class _Alloc
    >
    inline void save_collection(
        Archive & ar, 
        std::vector<_Ty, _Alloc> const & t, 
        Size n)
    {
        save(ar, t, n);
    }

    template<
        class Size,
        class Archive, 
        class _Ty, 
        class _Alloc
    >
    inline void load_collection(
        Archive & ar, 
        std::vector<_Ty, _Alloc> & t, 
        Size n)
    {
        load(ar, t, n);
    }

    template<
        class Archive, 
        class _Ty, 
        class _Alloc
    >
    inline void serialize(
        Archive & ar,
        std::vector<_Ty, _Alloc> & t)
    {
        util::serialization::serialize_collection(ar, t);
    }

} // namespace std

#endif // _UTIL_SERIALIZATION_STL_VECTOR_H_
