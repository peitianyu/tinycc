// [[nodiscard]] warning on discarded return values (C23)
#include <stdio.h>

[[nodiscard]] int getval(void) { return 42; }
[[nodiscard]] double getd(void) { return 1.5; }
int getplain(void) { return 7; }

int main(void)
{
    getval();                   // warning
    getd();                     // warning
    getplain();                 // no warning
    (void)getval();             // no warning: explicit discard
    int x = getval();           // no warning: value used
    printf("%d %d\n", x, getplain());
    return 0;
}
