/* TCC <stdbit.h> — C23 bit manipulation utilities.
   Implemented over __builtin_clz/ctz/popcount with per-width helpers
   dispatched via _Generic. */

#ifndef _STDBIT_H
#define _STDBIT_H

/* width helpers */
#define STDBIT_UC 8
#define STDBIT_US 16
#define STDBIT_UI 32
#define STDBIT_UL (sizeof(unsigned long) * 8)
#define STDBIT_ULL 64

/* leading zeros (count of zero bits before the first one, within the
   type's width) */
static inline unsigned int stdc_leading_zeros_uc(unsigned char x)
{ return x ? (unsigned)__builtin_clz((unsigned)x) - (32 - STDBIT_UC) : STDBIT_UC; }
static inline unsigned int stdc_leading_zeros_us(unsigned short x)
{ return x ? (unsigned)__builtin_clz((unsigned)x) - (32 - STDBIT_US) : STDBIT_US; }
static inline unsigned int stdc_leading_zeros_ui(unsigned int x)
{ return x ? (unsigned)__builtin_clz(x) : STDBIT_UI; }
static inline unsigned int stdc_leading_zeros_ul(unsigned long x)
{ return x ? (unsigned)__builtin_clzl(x) : STDBIT_UL; }
static inline unsigned int stdc_leading_zeros_ull(unsigned long long x)
{ return x ? (unsigned)__builtin_clzll(x) : STDBIT_ULL; }

#define stdc_leading_zeros(x) \
    _Generic((x), \
        unsigned char: stdc_leading_zeros_uc((unsigned char)(x)), \
        unsigned short: stdc_leading_zeros_us((unsigned short)(x)), \
        unsigned int: stdc_leading_zeros_ui((unsigned int)(x)), \
        unsigned long: stdc_leading_zeros_ul((unsigned long)(x)), \
        unsigned long long: stdc_leading_zeros_ull((unsigned long long)(x)), \
        default: stdc_leading_zeros_ull((unsigned long long)(x)))

/* trailing zeros */
static inline unsigned int stdc_trailing_zeros_uc(unsigned char x)
{ return x ? (unsigned)__builtin_ctz((unsigned)x) : STDBIT_UC; }
static inline unsigned int stdc_trailing_zeros_us(unsigned short x)
{ return x ? (unsigned)__builtin_ctz((unsigned)x) : STDBIT_US; }
static inline unsigned int stdc_trailing_zeros_ui(unsigned int x)
{ return x ? (unsigned)__builtin_ctz(x) : STDBIT_UI; }
static inline unsigned int stdc_trailing_zeros_ul(unsigned long x)
{ return x ? (unsigned)__builtin_ctzl(x) : STDBIT_UL; }
static inline unsigned int stdc_trailing_zeros_ull(unsigned long long x)
{ return x ? (unsigned)__builtin_ctzll(x) : STDBIT_ULL; }

#define stdc_trailing_zeros(x) \
    _Generic((x), \
        unsigned char: stdc_trailing_zeros_uc((unsigned char)(x)), \
        unsigned short: stdc_trailing_zeros_us((unsigned short)(x)), \
        unsigned int: stdc_trailing_zeros_ui((unsigned int)(x)), \
        unsigned long: stdc_trailing_zeros_ul((unsigned long)(x)), \
        unsigned long long: stdc_trailing_zeros_ull((unsigned long long)(x)), \
        default: stdc_trailing_zeros_ull((unsigned long long)(x)))

/* count of one bits */
static inline unsigned int stdc_count_ones_uc(unsigned char x)
{ return (unsigned)__builtin_popcount((unsigned)x); }
static inline unsigned int stdc_count_ones_us(unsigned short x)
{ return (unsigned)__builtin_popcount((unsigned)x); }
static inline unsigned int stdc_count_ones_ui(unsigned int x)
{ return (unsigned)__builtin_popcount(x); }
static inline unsigned int stdc_count_ones_ul(unsigned long x)
{ return (unsigned)__builtin_popcountl(x); }
static inline unsigned int stdc_count_ones_ull(unsigned long long x)
{ return (unsigned)__builtin_popcountll(x); }

#define stdc_count_ones(x) \
    _Generic((x), \
        unsigned char: stdc_count_ones_uc((unsigned char)(x)), \
        unsigned short: stdc_count_ones_us((unsigned short)(x)), \
        unsigned int: stdc_count_ones_ui((unsigned int)(x)), \
        unsigned long: stdc_count_ones_ul((unsigned long)(x)), \
        unsigned long long: stdc_count_ones_ull((unsigned long long)(x)), \
        default: stdc_count_ones_ull((unsigned long long)(x)))

/* power of two? */
#define stdc_has_single_bit(x) (stdc_count_ones(x) == 1)

/* bit width (position of highest set bit + 1, 0 for 0) */
#define stdc_bit_width(x) \
    ((unsigned int)(sizeof(x) * 8) - stdc_leading_zeros(x))

/* largest power of two <= x */
#define stdc_bit_floor(x) \
    ((x) == 0 ? 0 : ((x) & ~((__typeof__(x))-1 >> 1) ? \
     ((__typeof__(x))1 << (sizeof(x) * 8 - 1)) : \
     ((__typeof__(x))1 << (stdc_bit_width(x) - 1))))

/* smallest power of two >= x */
#define stdc_bit_ceil(x) \
    ((x) <= 1 ? 1 : \
     (stdc_has_single_bit(x) ? (x) : \
      ((__typeof__(x))1 << stdc_bit_width(x))))

#endif /* _STDBIT_H */
