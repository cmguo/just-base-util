// ClassFactory.h

#ifndef _UTIL_TOOLS_CLASS_FACTORY_H_
#define _UTIL_TOOLS_CLASS_FACTORY_H_

#include "util/tools/Creator.h"
#include "util/tools/StaticInitor.h"

#include <boost/function.hpp>
#include <boost/type_traits/is_same.hpp>

namespace util
{
    namespace tools
    {

        struct ClassFactoryTraits
        {
            typedef void (sinit_proto)();
            typedef void sinitor_type;
            typedef void sinitor_map_type;

            typedef void creator_type;
            typedef void creator_map_type;

            // override by devide classes
            static boost::system::error_code error_not_found()
            {
                return framework::system::logic_error::item_not_exist;
            }
        };

        template <
            typename FactoryTraits
        >
        class ClassFactory
        {
        protected:
            typedef ClassFactory factory_type;
            typedef FactoryTraits factory_traits;
            typedef typename FactoryTraits::key_type key_type;
            typedef typename FactoryTraits::sinit_proto sinit_proto;
            typedef typename boost::mpl::if_<
                boost::is_same<void, typename FactoryTraits::sinitor_type>, 
                boost::function<sinit_proto>, 
                typename FactoryTraits::sinitor_type
            >::type sinitor_type;
            typedef typename boost::mpl::if_<
                boost::is_same<void, typename FactoryTraits::sinitor_map_type>, 
                std::map<key_type, sinitor_type>, 
                typename FactoryTraits::sinitor_map_type
            >::type sinitor_map_type;
            typedef typename FactoryTraits::create_proto create_proto;
            typedef typename boost::mpl::if_<
                boost::is_same<void, typename FactoryTraits::creator_type>, 
                boost::function<create_proto>, 
                typename FactoryTraits::creator_type
            >::type creator_type;
            typedef typename creator_type::result_type result_type;
            typedef typename boost::mpl::if_<
                boost::is_same<void, typename FactoryTraits::creator_map_type>, 
                std::map<key_type, creator_type>, 
                typename FactoryTraits::creator_map_type
            >::type creator_map_type;

        public:
            template <typename C>
            static void register_class(
                key_type const & key)
            {
                register_sinitor(key, StaticInitor<C, sinit_proto>());
                register_creator(key, Creator<C>());
            }

            static void register_sinitor(
                key_type const & key, 
                sinitor_type sinitor)
            {
                sinitor_map().insert(std::make_pair(key, sinitor));
            }

            static void register_creator(
                key_type const & key, 
                creator_type creator)
            {
                creator_map().insert(std::make_pair(key, creator));
            }

        public:
            static void static_init()
            {
                typename sinitor_map_type::const_iterator iter =
                    sinitor_map().begin();
                for (; iter != sinitor_map().end(); ++iter) {
                    iter->second();
                }
            }

            template <
                typename Arg1
            >
            static void static_init(
                Arg1 & arg1)
            {
                typename sinitor_map_type::const_iterator iter =
                    sinitor_map().begin();
                for (; iter != sinitor_map().end(); ++iter) {
                    iter->second(arg1);
                }
            }

            template <
                typename Arg1, 
                typename Arg2
            >
            static void static_init(
                Arg1 & arg1, 
                Arg2 & arg2)
            {
                typename sinitor_map_type::const_iterator iter =
                    sinitor_map().begin();
                for (; iter != sinitor_map().end(); ++iter) {
                    iter->second(arg1, arg2);
                }
            }

        public:
            static result_type create(
                key_type const & key, 
                boost::system::error_code & ec)
            {
                typename creator_map_type::const_iterator iter = 
                    creator_map().find(key);
                if (iter == creator_map().end()) {
                    ec = factory_traits::error_not_found();
                    return result_type();
                }
                ec.clear();
                return iter->second();
            }

            template <
                typename Arg1
            >
            static result_type create(
                key_type const & key, 
                Arg1 & arg1, 
                boost::system::error_code & ec)
            {
                typename creator_map_type::const_iterator iter = 
                    creator_map().find(key);
                if (iter == creator_map().end()) {
                    ec = factory_traits::error_not_found();
                    return result_type();
                }
                ec.clear();
                return iter->second(arg1);
            }

            template <
                typename Arg1, 
                typename Arg2
            >
            static result_type create(
                key_type const & key, 
                Arg1 & arg1, 
                Arg2 & arg2, 
                boost::system::error_code & ec)
            {
                typename creator_map_type::const_iterator iter = 
                    creator_map().find(key);
                if (iter == creator_map().end()) {
                    ec = factory_traits::error_not_found();
                    return result_type();
                }
                ec.clear();
                return iter->second(arg1, arg2);
            }

            template <
                typename Arg1, 
                typename Arg2, 
                typename Arg3
            >
            static result_type create(
                key_type const & key, 
                Arg1 & arg1, 
                Arg2 & arg2, 
                Arg3 & arg3, 
                boost::system::error_code & ec)
            {
                typename creator_map_type::const_iterator iter = 
                    creator_map().find(key);
                if (iter == creator_map().end()) {
                    ec = factory_traits::error_not_found();
                    return result_type();
                }
                ec.clear();
                return iter->second(arg1, arg2, arg3);
            }

            static void destroy(
                result_type & obj)
            {
                delete obj;
                obj = result_type();
            }

        protected:
            static sinitor_map_type & sinitor_map()
            {
                static sinitor_map_type smap;
                return smap;
            }

            static creator_map_type & creator_map()
            {
                static creator_map_type smap;
                return smap;
            }
        };

    } // namespace tools
} // namespace util

#define UTIL_DELAY_REGISTER_CLASS
#include "util/tools/ClassRegister.h"
#undef UTIL_DELAY_REGISTER_CLASS

#endif // _UTIL_TOOLS_CLASS_FACTORY_H_
