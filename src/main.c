#include <stdio.h>

int sum(int a, int b) {
    return a + b;
}


#ifndef SNOW_ENABLED
int main() {
    printf("4 + 5 = %d\n", sum(4, 5));
}
#else
#include "snow.h"
describe(sum) {
    it("sums") {
        asserteq(9, sum(4, 5));
    }
}
snow_main();
#endif
