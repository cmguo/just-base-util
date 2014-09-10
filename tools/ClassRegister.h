// ClassRegister.h

#ifndef UTIL_DELAY_REGISTER_CLASS

#  ifndef UTIL_REGISTER_CLASS_NAME

#    ifdef UTIL_REGISTER_CLASS
#      undef UTIL_REGISTER_CLASS
#      undef UTIL_REGISTER_CLASS_FUNC
#    endif

#    include "util/tools/Call.h"

#    define UTIL_REGISTER_CLASS_NAME(cls) BOOST_PP_CAT(reg_class_, BOOST_PP_CAT(cls, __LINE__))
#    define UTIL_REGISTER_CLASS(fact, key, cls) \
         static util::tools::Call UTIL_REGISTER_CLASS_NAME(cls)(fact::register_class<cls>, key)
#    define UTIL_REGISTER_CLASS_FUNC(fact, key, func) \
         static util::tools::Call UTIL_REGISTER_CLASS_NAME(func)(fact::register_creator, key, func)

#  endif // UTIL_REGISTER_CLASS_NAME

#else // UTIL_DELAY_REGISTER_CLASS

#  ifndef UTIL_REGISTER_CLASS_NAME
#    define UTIL_REGISTER_CLASS(fact, key, cls)
#    define UTIL_REGISTER_CLASS_FUNC(fact, key, func)
#  endif

#endif // UTIL_DELAY_REGISTER_CLASS
