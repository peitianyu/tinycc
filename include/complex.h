/* TCC <complex.h> — C11 complex arithmetic (partial support).
   _Complex double/float types, assignment and + - * / are implemented;
   creal/cimag are macros over the two-part memory layout. */

#ifndef _TCC_COMPLEX_H
#define _TCC_COMPLEX_H

/* The imaginary unit.  A compound literal keeps it addressable, which
   complex values require in TCC (they are two-part objects). */
#define _Complex_I ((float _Complex){0.0f, 1.0f})
#define I _Complex_I

/* real/imag extraction via the (re, im) memory layout */
#define creal(z)  (((double *)&(z))[0])
#define cimag(z)  (((double *)&(z))[1])
#define crealf(z) (((float *)&(z))[0])
#define cimagf(z) (((float *)&(z))[1])

#endif /* _TCC_COMPLEX_H */
