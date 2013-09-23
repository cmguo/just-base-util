// ClassRegister.h

#ifndef UTIL_DELAY_REGISTER_CLASS

#  ifndef UTIL_REGISTER_CLASS_NAME

#    ifdef UTIL_REGISTER_CLASS
#      undef UTIL_REGISTER_CLASS
#      undef UTIL_REGISTER_CLASS_FUNC
#      undef UTIL_REGISTER_CLASS_FACTORY
#      undef UTIL_REGISTER_CLASS_FACTORY_FUNC
#    endif

#    include "util/tools/Create.h"
#    include "util/tools/Call.h"

#    define UTIL_REGISTER_CLASS_NAME(cls) BOOST_PP_CAT(reg_class_, BOOST_PP_CAT(cls, __LINE__))
#    define UTIL_REGISTER_CLASS(key, cls) \
         static util::tools::Call UTIL_REGISTER_CLASS_NAME(cls)(cls::register_creator, key, util::tools::Creator<cls>())
#    define UTIL_REGISTER_CLASS_FUNC(key, cls, func) \
         static util::tools::Call UTIL_REGISTER_CLASS_NAME(cls)(cls::register_creator, key, func)
#    define UTIL_REGISTER_CLASS_FACTORY(key, fact, cls) \
         static util::tools::Call UTIL_REGISTER_CLASS_NAME(cls)(fact::register_creator, key, util::tools::Creator<cls>())
#    define UTIL_REGISTER_CLASS_FACTORY_FUNC(key, fact, func) \
         static util::tools::Call UTIL_REGISTER_CLASS_NAME(cls)(fact::register_creator, key, func)

#  endif // UTIL_REGISTER_CLASS_NAME

#else // UTIL_DELAY_REGISTER_CLASS

#  ifndef UTIL_REGISTER_CLASS_NAME
#    define UTIL_REGISTER_CLASS(key, cls)
#    define UTIL_REGISTER_CLASS_FUNC(key, cls, func)
#    define UTIL_REGISTER_CLASS_FACTORY(key, fact, cls)
#    define UTIL_REGISTER_CLASS_FACTORY_FUNC(key, fact, func)
#  endif

#endif // UTIL_DELAY_REGISTER_CLASS
