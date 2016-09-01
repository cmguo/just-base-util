// BasicOArchive.h

#ifndef _UTIL_ARCHIVE_BASIC_O_ARCHIVE_H_
#define _UTIL_ARCHIVE_BASIC_O_ARCHIVE_H_

#include "util/archive/BasicArchiveT.h"

namespace util
{
    namespace archive
    {

        struct SaveAccess
        {
            template <typename Archive, typename T>
            static void save_wrapper(Archive & ar, T const & t)
            {
                ar.save_wrapper(t);
            }

            template <typename Archive, typename T>
            static void save(Archive & ar, T const & t)
            {
                ar.save(t);
            }

            template <typename Archive, typename T, typename Catalog>
            static void save_catalog(Archive & ar, T const & t, Catalog * cat)
            {
                ar.save_catalog(t, cat);
            }

            template <typename Archive>
            static void save_binary(Archive & ar, char const * p, std::size_t n)
            {
                ar.save_binary(p, n);
            }

            template <typename Archive>
            static void save_start(Archive & ar, std::string const & name)
            {
                ar.save_start(name);
            }

            template <typename Archive>
            static void save_end(Archive & ar, std::string const & name)
            {
                ar.save_end(name);
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
            输出文档的基类
            
            提供“<<”、“&”操作符重载
            区别对待基本类型与标准类型的序列化
            @param Archive 派生自basic_oarchive的输出文档类，实现对基本类型的输出
         */
        template <typename Archive>
        class BasicOArchive
            : public BasicArchiveT<Archive>
        {
        public:
            typedef boost::mpl::true_ is_saving; ///< 表明该归档是用于保存数据的
            typedef boost::mpl::false_ is_loading; ///< 表明该归档“不是”用于加载数据的
            friend struct archive_access;
            friend struct SaveAccess;

        public:
            /// 重载操作符“<<”
            template<class T>
            Archive & operator <<(
                T const & t)
            {
                return BasicArchiveT<Archive>::operator &(t);
            }

            /*
            template<class T>
            Archive & operator <<(
                T const t)
            {
                return BasicArchiveT<Archive>::operator &(t);
            }
            */

            /// 重载操作符“&”
            template<class T>
            Archive & operator &(
                T const & t)
            {
                return This()->operator <<(t);
            }

        protected:
            using BasicArchiveT<Archive>::This;

            template<class T, class C>
            void serialize_catalog(
                T & t, C * c)
            {
                SaveAccess::save_catalog(*This(), t, c);
            }

            void serialize_binary(
                char * p, 
                std::size_t n)
            {
                SaveAccess::save_binary(*This(), p, n);
            }

        protected:
            template<class T, class C>
            void save_catalog(
                T const & t, C * c)
            {
                BasicArchiveT<Archive>::serialize_catalog(const_cast<T &>(t), c);
            }

            template<class T>
            void save_catalog(
                T const & t, catalog_wrapper *)
            {
                SaveAccess::save_wrapper(*This(), t);
            }

            template <class T>
            void save_catalog(
                T const & t, catalog_null_ptr *)
            {
                SaveAccess::save(*This(), null_ptr());
            }

            template <class T>
            void save_catalog(
                T const & t, catalog_primitive *)
            {
                SaveAccess::save(*This(), t);
            }

            template <class T>
            void save_catalog(
                T const & t, catalog_stringlized *)
            {
                std::string str = t.to_string();
                serialize_catalog(str, (catalog_primitive *)NULL);
            }

        protected:
            void start_name(
                std::string const & name)
            {
                SaveAccess::save_start(*This(), name);
            }

            void finish_name(
                std::string const & name)
            {
                SaveAccess::save_end(*This(), name);
            }

            template <class Catalog>
            void start_child(
                Catalog *)
            {
                SaveAccess::sub_start(*This());
            }

            template <class Catalog>
            void finish_child(
                Catalog *)
            {
                SaveAccess::sub_end(*This());
            }

        protected:
            template <class T>
            void save(
                T const & t)
            {
                this->state(3);
            }

            /// 向流中写入标准库字符串
            template<class _Elem, class _Traits, class _Ax>
            void save(
                std::basic_string<_Elem, _Traits, _Ax> const & t)
            {
                typename std::basic_string<_Elem, _Traits, _Ax>::size_type size = t.size();
                SaveAccess::save(*This(), size);
                SaveAccess::save_binary(*This(), (char const *)&t[0], t.size() * sizeof(_Elem));
            }

            template<class T>
            void save_wrapper(
                T const & t)
            {
                BasicArchiveT<Archive>::serialize_catalog(
                    const_cast<T &>(t), (catalog_wrapper *)NULL);
            }

            void save_binary(
                char const * p, 
                std::size_t n)
            {
                this->state(3);
            }

        protected:
            void save_start(
                std::string const & name)
            {
            }

            void save_end(
                std::string const & name)
            {
            }

            void sub_start()
            {
            }

            void sub_end()
            {
            }
        }; // class BasicOArchive

    } // namespace archive
} // namespace util

namespace util
{
    namespace serialization
    {

        BOOST_TTI_HAS_MEMBER_FUNCTION(to_string);

        template<
            class _T
        >
        struct has_to_string
            : has_member_function_to_string<_T const, std::string>
        {
        };

    } // namespace serialization
} // namespace util

#endif // _UTIL_ARCHIVE_BASIC_O_ARCHIVE_H_
