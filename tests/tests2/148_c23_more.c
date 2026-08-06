// C23: u8'x' character constants, typeof_unqual, extended <stdbit.h>
#include <stdio.h>
#include <stdbit.h>

int main(void)
{
    /* u8'x' UTF-8 character constants */
    printf("u8: %d %d %d\n", u8'A', u8'\u20ac', u8'€');
    /* typeof_unqual: scalar / pointer / array */
    volatile int v = 3;
    typeof_unqual(v) a = 1;
    a = 2;
    const int *p = 0;
    typeof_unqual(p) q = p;
    typeof_unqual(const int[3]) arr = {1, 2, 3};
    arr[0] = 9;
    printf("tu: %d %d\n", a, arr[0]);
    /* stdbit.h extensions */
    printf("sb: %u %u %u %u %u %u %u\n",
           stdc_first_leading_zero(0x7FFFFFFFu),   /* 1 */
           stdc_first_leading_one(0x10u),          /* 28 */
           stdc_first_trailing_zero(7u),           /* 4 */
           stdc_first_trailing_one(12u),           /* 3 */
           stdc_leading_ones(0xF0000000u),         /* 4 */
           stdc_trailing_ones(0x0Fu),              /* 4 */
           stdc_count_zeros(0x0Fu));               /* 28 */
    return 0;
}
