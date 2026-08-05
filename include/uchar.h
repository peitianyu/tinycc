/* TCC <uchar.h> — C11 char16_t/char32_t support.
   TCC has no builtin 16/32-bit character types; these are plain
   typedefs, matching the u'x'/U'x' literal support in the compiler
   (literals yield unsigned integer constants). */

#ifndef _TCC_UCHAR_H
#define _TCC_UCHAR_H

#include <wchar.h> /* mbstate_t */

#if defined __STDC_VERSION__ && __STDC_VERSION__ >= 201112L

#define __STDC_UTF_16__ 1
#define __STDC_UTF_32__ 1

typedef unsigned short char16_t;
typedef unsigned int char32_t;

size_t mbrtoc16(char16_t *restrict pc16, const char *restrict s, size_t n,
                mbstate_t *restrict ps);
size_t c16rtomb(char *restrict s, char16_t c16, mbstate_t *restrict ps);
size_t mbrtoc32(char32_t *restrict pc32, const char *restrict s, size_t n,
                mbstate_t *restrict ps);
size_t c32rtomb(char *restrict s, char32_t c32, mbstate_t *restrict ps);

#endif /* __STDC_VERSION__ >= 201112L */

#endif /* _TCC_UCHAR_H */
