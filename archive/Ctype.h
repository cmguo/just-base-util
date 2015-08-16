// Ctype.h

namespace std {

template <>
class __attribute__ ((__type_visibility__("default"))) ctype<unsigned char>
    : public ctype<char>
{
public:
    typedef unsigned char char_type;

    explicit ctype(const mask* __tab = 0, bool __del = false, size_t __refs = 0)
        : ctype<char>(__tab, __del, __refs) {}

    __attribute__ ((__visibility__("hidden"), __always_inline__))
    bool is(mask __m, char_type __c) const
    {
        return ctype<char>::is(__m, (char)__c);
    }

    __attribute__ ((__visibility__("hidden"), __always_inline__))
    const char_type* is(const char_type* __low, const char_type* __high, mask* __vec) const
    {
        return (const char_type*)ctype<char>::is((const char *)__low, (const char *)__high, __vec);
    }

    __attribute__ ((__visibility__("hidden"), __always_inline__))
    const char_type* scan_is (mask __m, const char_type* __low, const char_type* __high) const
    {
        return (const char_type*)ctype<char>::scan_is(__m, (const char *)__low, (const char *)__high);
    }

    __attribute__ ((__visibility__("hidden"), __always_inline__))
    const char_type* scan_not(mask __m, const char_type* __low, const char_type* __high) const
    {
        return (const char_type*)ctype<char>::scan_not(__m, (const char *)__low, (const char *)__high);
    }

    __attribute__ ((__visibility__("hidden"), __always_inline__))
    char_type toupper(char_type __c) const
    {
        return ctype<char>::toupper((char)__c);
    }

    __attribute__ ((__visibility__("hidden"), __always_inline__))
    const char_type* toupper(char_type* __low, const char_type* __high) const
    {
        return (const char_type *)ctype<char>::toupper((char *)__low, (const char *)__high);
    }

    __attribute__ ((__visibility__("hidden"), __always_inline__))
    char_type tolower(char_type __c) const
    {
        return ctype<char>::tolower((char)__c);
    }

    __attribute__ ((__visibility__("hidden"), __always_inline__))
    const char_type* tolower(char_type* __low, const char_type* __high) const
    {
        return (const char_type*)ctype<char>::tolower((char *)__low, (const char *)__high);
    }

    __attribute__ ((__visibility__("hidden"), __always_inline__))
    char_type widen(char __c) const
    {
        return ctype<char>::widen(__c);
    }

    __attribute__ ((__visibility__("hidden"), __always_inline__))
    const char* widen(const char* __low, const char* __high, char_type* __to) const
    {
        return ctype<char>::widen(__low, __high, (char *)__to);
    }

    __attribute__ ((__visibility__("hidden"), __always_inline__))
    char narrow(char_type __c, char __dfault) const
    {
        return ctype<char>::narrow((char)__c, __dfault);
    }

    __attribute__ ((__visibility__("hidden"), __always_inline__))
    const char* narrow(const char_type* __low, const char_type* __high, char __dfault, char* __to) const
    {
        return ctype<char>::narrow((const char *)__low, (const char *)__high, __dfault, __to);
    }

    static locale::id id;

protected:
    ~ctype();
};

} // namespace std
