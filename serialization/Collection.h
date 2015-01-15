// Collection.h

#ifndef _UTIL_SERIALIZATION_COLLECTION_H_
#define _UTIL_SERIALIZATION_COLLECTION_H_

#include "util/serialization/NVPair.h"
#include "util/serialization/SplitMember.h"

namespace util
{
    namespace serialization
    {

        template<
            class Size,
            class Archive, 
            class Container
        >
        inline void save_collection(
            Archive & ar, 
            Container const & t, 
            Size n)
        {
            typename Container::const_iterator it = t.begin();
            Size count = n;
            while(ar && count-- > 0) {
                ar << util::serialization::make_nvp("item", *it++);
            }
        }

        template<
            class Size,
            class Archive, 
            class Container
        >
        inline void load_collection(
            Archive & ar, 
            Container & t, 
            Size n)
        {
            t.clear();
            Size count = n;
            while(count-- > 0) {
                typename Container::value_type v;
                ar >> util::serialization::make_nvp("item", v);
                if (ar)
                    t.insert(t.end(), v);
                else
                    break;
            }
        }

        template<
            class Size,
            class Archive, 
            class Container
        >
        inline void save_collection(
            Archive & ar, 
            Container const & t)
        {
            Size count(t.size());
            ar <<  SERIALIZATION_NVP(count);
            save_collection(ar, t, count);
        }

        template<
            class Size,
            class Archive, 
            class Container
        >
        inline void load_collection(
            Archive & ar, 
            Container & t)
        {
            t.clear();
            Size count = 0;
            ar >>  SERIALIZATION_NVP(count);
            load_collection(ar, t, count);
        }

        struct free_collection_saver
        {
            template <
                class Size,
                class Archive, 
                class Container
            >
            static void invoke(
                Archive & ar, 
                Container const & t)
            {
                save_collection<Size, Archive, Container>(ar, t);
            }

            template <
                class Size,
                class Archive, 
                class Container
            >
            static void invoke(
                Archive & ar, 
                Container const & t, 
                Size n)
            {
                save_collection<Size, Archive, Container>(ar, t, n);
            }
        };

        /// 调用类中拆分出来的反序列化方法
        struct free_collection_loader
        {
            template<
                class Size,
                class Archive, 
                class Container
            >
            static void invoke(
                Archive & ar, 
                Container & t)
            {
                load_collection<Size>(ar, t);
            }

            template<
                class Size,
                class Archive, 
                class Container
            >
            static void invoke(
                Archive & ar, 
                Container & t, 
                Size n)
            {
                load_collection(ar, t, n);
            }
        };
        
        template<
            class Size,
            class Archive, 
            class Container
        >
        inline void serialize_collection(
            Archive & ar, 
            Container & t)
        {
            typedef BOOST_DEDUCED_TYPENAME boost::mpl::if_<
                BOOST_DEDUCED_TYPENAME Archive::is_saving, 
                free_collection_saver, 
                free_collection_loader
            >::type typex;
            typex::template invoke<Size>(ar, t);
        }

        template<
            class Archive, 
            class Container
        >
        inline void serialize_collection(
            Archive & ar, 
            Container & t)
        {
            serialize_collection<typename Container::size_type>(ar, t);
        }

        template<
            class Size,
            class Archive, 
            class Container
        >
        inline void serialize_collection(
            Archive & ar, 
            Container & t, 
            Size n)
        {
            typedef BOOST_DEDUCED_TYPENAME boost::mpl::if_<
                BOOST_DEDUCED_TYPENAME Archive::is_saving, 
                free_collection_saver, 
                free_collection_loader
            >::type typex;
            typex::invoke(ar, t, n);
        }

        template<
            class Size,
            class Container
        >
        class Sized
        {
        public:
            Sized(
                Container & c)
                : c_(c)
            {
            }

            SERIALIZATION_SPLIT_MEMBER();

            template <typename Archive>
            void save(Archive & ar) const
            {
                save_collection<Size>(ar, c_);
            }

            template <typename Archive>
            void load(Archive & ar)
            {
                load_collection<Size>(ar, c_);
            }

        private:
            Container & c_;
        };

        template<
            class Size,
            class Container
        >
        Sized<Size, Container> const make_sized(
            Container & c)
        {
            return Sized<Size, Container>(c);
        }

        template<
            class Size,
            class Container
        >
        class Sized2
        {
        public:
            Sized2(
                Container & c, 
                Size size)
                : c_(c)
                , size_(size)
            {
            }

            SERIALIZATION_SPLIT_MEMBER();

            template <typename Archive>
            void save(Archive & ar) const
            {
                save_collection(ar, c_, size_);
            }

            template <typename Archive>
            void load(Archive & ar)
            {
                load_collection(ar, c_, size_);
            }

        private:
            Container & c_;
            Size size_;
        };

        template<
            class Size,
            class Container
        >
        Sized2<Size, Container> const make_sized(
            Container & c, 
            Size size)
        {
            return Sized2<Size, Container>(c, size);
        }

    } // namespace serialization
} // namespace util

#endif // _UTIL_SERIALIZATION_COLLECTION_H_
