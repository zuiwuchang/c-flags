#include "tests.h"
#include <stdio.h>

int main(int argc, char **argv)
{
    CuString *output = CuStringNew();
    CuSuite *suite = ejs_unit_tests();

    CuSuiteRun(suite);

    CuSuiteSummary(suite, output);
    CuSuiteDetails(suite, output);
    printf("%s\n", output->buffer);
    int ret = suite->failCount;
    CuSuiteDelete(suite);
    CuStringDelete(output);
    return ret;
}