// ClassFactory.h

#ifndef _UTIL_TOOLS_CLASS_FACTORY_H_
#define _UTIL_TOOLS_CLASS_FACTORY_H_

#include <boost/function.hpp>

namespace util
{
    namespace tools
    {

        template <
            typename ClassType, 
            typename KeyType, 
            typename CreateProto, 
            typename FactoryType = void
        >
        class ClassFactory
        {
        protected:
            typedef KeyType key_type;
            typedef ClassType * result_type;
            typedef boost::function<CreateProto> creator_type;
            typedef std::map<KeyType, creator_type> creator_map_type;

        protected:
            typedef ClassFactory factory_type;

        public:
            // override by devide classes
            static boost::system::error_code error_not_found()
            {
                return framework::system::logic_error::item_not_exist;
            }

        public:
            static void register_creator(
                key_type const & key, 
                creator_type creator)
            {
                creator_map().insert(std::make_pair(key, creator));
            }

            static result_type create(
                key_type const & key, 
                boost::system::error_code & ec)
            {
                typename creator_map_type::const_iterator iter = 
                    creator_map().find(key);
                if (iter == creator_map().end()) {
                    ec = ClassType::error_not_found();
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
                    ec = ClassType::error_not_found();
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
                    ec = ClassType::error_not_found();
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
                    ec = ClassType::error_not_found();
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
