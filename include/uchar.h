/* TCC <uchar.h> — C11 char16_t/char32_t support.
   TCC has no builtin 16/32-bit character types; these are plain
   typedefs, matching the u'x'/U'x' literal support in the compiler
   (literals yield unsigned integer constants). */

#ifndef _TCC_UCHAR_H
#define _TCC_UCHAR_H

#if defined __STDC_VERSION__ && __STDC_VERSION__ >= 201112L

#define __STDC_UTF_16__ 1
#define __STDC_UTF_32__ 1

typedef unsigned short char16_t;
typedef unsigned int char32_t;

#endif /* __STDC_VERSION__ >= 201112L */

#endif /* _TCC_UCHAR_H */
