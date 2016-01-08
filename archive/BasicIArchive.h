// BasicIArchive.h

#ifndef _UTIL_ARCHIVE_BASIC_I_ARCHIVE_H_
#define _UTIL_ARCHIVE_BASIC_I_ARCHIVE_H_

#include "util/archive/BasicArchiveT.h"

#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_pointer.hpp>
#include <boost/type_traits/is_enum.hpp>

namespace util
{
    namespace archive
    {

        struct LoadAccess
        {
            template <typename Archive, typename T>
            static void load_wrapper(Archive & ar, T & t)
            {
                ar.load_wrapper(t);
            }

            template <typename Archive, typename T>
            static void load(Archive & ar, T & t)
            {
                ar.load(t);
            }

            template <typename Archive, typename T, typename Catalog>
            static void load_catalog(Archive & ar, T & t, Catalog * cat)
            {
                ar.load_catalog(t, cat);
            }

            template <typename Archive>
            static void load_binary(Archive & ar, char * p, std::size_t n)
            {
                ar.load_binary(p, n);
            }

            template <typename Archive>
            static void load_start(Archive & ar, std::string const & name)
            {
                ar.load_start(name);
            }

            template <typename Archive>
            static void load_end(Archive & ar, std::string const & name)
            {
                ar.load_end(name);
            }

            template <typename Archive>
            static void sub_start(Archive & ar)
            {
                ar.sub_start();
            }

            template <typename Archive>
            static void sub_end(Archive & ar)
            {
                ar.sub_end();
            }
        };

        /** 
            输入文档的基类

            提供“>>”、“&”操作符重载
            区别对待基本类型与标准类型的读
            @param Archive 派生自basic_iarchive的输入文档类，实现对基本类型的输入
         */
        template <typename Archive>
        class BasicIArchive
            : public BasicArchiveT<Archive>
        {
        public:
            typedef boost::mpl::false_ is_saving; ///< 表明该归档“不是”用于保存数据的
            typedef boost::mpl::true_ is_loading; ///< 表明该归档是用于加载数据的
            friend struct archive_access;
            friend struct LoadAccess;

        public:
            /// 重载操作符“>>”
            template<class T>
            Archive & operator >>(
                T & t)
            {
                return BasicArchiveT<Archive>::operator &(t);
            }

            /// 重载操作符“&”
            template<class T>
            Archive & operator &(
                T & t)
            {
                return This()->operator >>(t);
            }

        protected:
            using BasicArchiveT<Archive>::This;

            template<class T, class C>
            void serialize_catalog(
                T & t, C * c)
            {
                LoadAccess::load_catalog(*This(), t, c);
            }

            void serialize_binary(
                char * p, 
                std::size_t n)
            {
                LoadAccess::load_binary(*This(), p, n);
            }

        protected:
            template<class T, class C>
            void load_catalog(
                T & t, C * c)
            {
                BasicArchiveT<Archive>::serialize_catalog(t, c);
            }

            template<class T>
            void load_catalog(
                T & t, catalog_wrapper *)
            {
                LoadAccess::load_wrapper(*This(), t);
            }

            template <class T>
            void load_catalog(
                T & t, catalog_primitive *)
            {
                LoadAccess::load(*This(), t);
            }

            struct failed
            {
                bool operator()(
                    bool b)
                {
                    return !b;
                }

                bool operator()(
                    boost::system::error_code ec)
                {
                    return ec;
                }
            };

            template <class T>
            void load_catalog(
                T & t, catalog_stringlized *)
            {
                std::string str;
                serialize_catalog(str, (catalog_primitive *)NULL);
                if (this->state())
                    return;
                if (failed()(t.from_string(str)))
                    this->fail();
            }

        protected:
            void start_name(
                std::string const & name)
            {
                LoadAccess::load_start(*This(), name);
            }

            void finish_name(
                std::string const & name)
            {
                LoadAccess::load_end(*This(), name);
            }

            template <typename Catalog>
            void start_child(
                Catalog *)
            {
                LoadAccess::sub_start(*This());
            }

            template <typename Catalog>
            void finish_child(
                Catalog *)
            {
                LoadAccess::sub_end(*This());
            }

        protected:
            template <class T>
            void load(
                T & t)
            {
                this->state(3);
            }

            /// 从流中读出标准库字符串
            template<class _Elem, class _Traits, class _Ax>
            void load(
                std::basic_string<_Elem, _Traits, _Ax> & t)
            {
                typename std::basic_string<_Elem, _Traits, _Ax>::size_type len;
                LoadAccess::load(*This(), len);
                if (this->state()) return;
                std::size_t l = 0;
                if (len > 1024) {
                    for (; len > 1024; l += 1024, len -= 1024) {
                        t.resize(l + 1024);
                        LoadAccess::load_binary(*This(), &t[l], sizeof(_Elem) * 1024);
                        if (this->state()) return;
                    }
                }
                t.resize(l + len);
                LoadAccess::load_binary(*This(), &t[l], len * sizeof(_Elem));
            }

            template<class T>
            void load_wrapper(
                util::serialization::NVPair<T> const & t)
            {
                BasicArchiveT<Archive>::serialize_catalog(
                    const_cast<util::serialization::NVPair<T> &>(t), (catalog_wrapper *)NULL);
            }

            void load_binary(
                char * p, 
                std::size_t n)
            {
                this->state(3);
            }

        protected:
            void load_start(
                std::string const & name)
            {
            }

            void load_end(
                std::string const & name)
            {
            }

            void sub_start()
            {
            }

            void sub_end()
            {
            }

        }; // class BasicIArchive

    } // namespace archive
} // namespace util

namespace util
{
    namespace serialization
    {
        BOOST_TTI_HAS_MEMBER_FUNCTION(from_string);

        template<
            class _T
        >
        struct has_from_string
            : boost::mpl::or_<
                has_member_function_from_string<_T, bool, boost::mpl::vector<std::string const &> >, 
                has_member_function_from_string<_T, boost::system::error_code, boost::mpl::vector<std::string const &> > >
        {
        };

    }  // namespace serialization
} // namespace util

#endif // _UTIL_ARCHIVE_BASIC_I_ARCHIVE_H_

