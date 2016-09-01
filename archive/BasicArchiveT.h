// BasicArchiveT.h

#ifndef _UTIL_ARCHIVE_BASIC_ARCHIVE_T_H_
#define _UTIL_ARCHIVE_BASIC_ARCHIVE_T_H_

#include "util/archive/BasicArchive.h"

#include <framework/string/Format.h>

#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_pointer.hpp>
#include <boost/type_traits/is_enum.hpp>
#include <boost/tti/has_member_function.hpp>

namespace util
{
    namespace archive
    {

        struct archive_access
        {
            template <typename Archive>
            static void start_name(Archive & ar, std::string const & name)
            {
                ar.start_name(name);
            }

            template <typename Archive>
            static void finish_name(Archive & ar, std::string const & name)
            {
                ar.finish_name(name);
            }

            template <typename Archive, typename Catalog>
            static void start_child(Archive & ar, Catalog * cat)
            {
                ar.start_child(cat);
            }

            template <typename Archive, typename Catalog>
            static void finish_child(Archive & ar, Catalog * cat)
            {
                ar.finish_child(cat);
            }

            template <typename Archive, typename T, typename Catalog>
            static void serialize_catalog(Archive & ar, T & t, Catalog * cat)
            {
                ar.serialize_catalog(t, cat);
            }
        };

        struct catalog_wrapper;
        struct catalog_null_ptr;
        struct catalog_primitive;
        struct catalog_enum;
        struct catalog_single_unit;
        struct catalog_stringlized;
        struct catalog_object;
        struct catalog_collection;

        struct null_ptr {};

        /** 
            �ĵ��Ļ���

            �ṩ��&������������
            ����Դ������������׼���͵Ķ�
            @param Archive ������
         */
        template <typename Archive>
        class BasicArchiveT
            : public BasicArchive
        {
        public:
            friend struct archive_access;

        public:
            /// ����ָ�����͵Ķ�
            struct serialize_pointer
            {
                template<class T>
                static void invoke(
                    Archive & ar, 
                    T & t)
                {
                    if (t == T())
                        archive_access::serialize_catalog(ar, t, (catalog_null_ptr*)NULL);
                    else
                        ar.operator &(*t);
                }
            };

            /// ����ö�����͵Ķ�
            struct serialize_enum
            {
                template<class T>
                static void invoke(
                    Archive & ar, 
                    T & t)
                {
                    int i;
                    archive_access::serialize_catalog(ar, i, (catalog_primitive*)NULL);
                    t = static_cast<T>(i);
                }
            };

            struct serialize_non_pointer
            {

                template<class T>
                static void invoke(
                    Archive &ar, 
                    T & t)
                {
                /// ����������𣨻������ͣ���׼���ͣ����ֱ����
                    typedef BOOST_DEDUCED_TYPENAME boost::mpl::if_<
                        util::serialization::is_primitive<Archive, T>, 
                        catalog_primitive, 
                        BOOST_DEDUCED_TYPENAME boost::mpl::if_<
                            util::serialization::is_wrapper<Archive, T>, 
                            catalog_wrapper, 
                            BOOST_DEDUCED_TYPENAME boost::mpl::if_<
                                util::serialization::is_single_unit<Archive, T>, 
                                catalog_single_unit, 
                                BOOST_DEDUCED_TYPENAME boost::mpl::if_<
                                    util::serialization::is_stringlized<Archive, T>, 
                                    catalog_stringlized, 
                                    BOOST_DEDUCED_TYPENAME boost::mpl::if_<
                                        util::serialization::is_collection<Archive, T>, 
                                        catalog_collection, 
                                        catalog_object
                                    >::type
                                >::type
                            >::type
                        >::type
                    >::type catalog_type;
                    archive_access::serialize_catalog(ar, t, (catalog_type*)NULL);
                }
            };

            /// ���ز����� &
            template<class T>
            Archive & operator &(
                T const & t)
            {
                if (this->state()) return *This();
                typedef BOOST_DEDUCED_TYPENAME boost::mpl::if_<
                    boost::is_pointer<T>, 
                    serialize_pointer, 
                    BOOST_DEDUCED_TYPENAME boost::mpl::if_<
                        boost::is_enum<T>, 
                        serialize_enum, 
                        serialize_non_pointer
                    >::type
                >::type invoke_type;
                invoke_type::invoke(*This(), const_cast<T &>(t));
                return *This();
            }

        protected:
            /// ��ȡ�������ָ��
            Archive * This()
            {
                return static_cast<Archive *>(this);
            }

            template<class T, class C>
            void serialize_catalog(
                T &, C *)
            {
                this->state(3);
            }

            template<class T>
            void serialize_catalog(
                T & t, catalog_wrapper*)
            {
                this->path_push(wrapper_name(t));

                archive_access::start_name(*This(), wrapper_name(t));
                This()->operator &(wrapper_data(t));
                archive_access::finish_name(*This(), wrapper_name(t));

                this->path_pop();
            }

            template<class T>
            void serialize_catalog(
                T & t, 
                catalog_single_unit *)
            {
                // ֱ�ӵ���serialize����
                using namespace util::serialization;
                serialize(*This(), t);
            }

            template<class T>
            void serialize_catalog(
                T & t, 
                catalog_collection * c)
            {
                archive_access::start_child(*This(), c);
                // ֱ�ӵ���serialize����
                using namespace util::serialization;
                serialize(*This(), t);
                archive_access::finish_child(*This(), c);
            }

            template<class T>
            void serialize_catalog(
                T & t, 
                catalog_object * c)
            {
                archive_access::start_child(*This(), c);
                // ֱ�ӵ���serialize����
                using namespace util::serialization;
                serialize(*This(), t);
                archive_access::finish_child(*This(), c);
            }

        protected:
            void start_name(
                std::string const & name)
            {
            }

            void finish_name(
                std::string const & name)
            {
            }

            void start_child()
            {
            }

            void finish_child()
            {
            }

            void start_object()
            {
                start_child();
            }

            void finish_object()
            {
                finish_child();
            }

            void start_collection()
            {
                start_child();
            }

            void finish_collection()
            {
                finish_child();
            }
        }; // class BasicArchiveT

    }  // namespace archive
} // namespace util

#endif // _UTIL_ARCHIVE_BASIC_ARCHIVE_T_H_

