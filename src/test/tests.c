#include "tests.h"
#include "strconv.h"
CuSuite *ejs_unit_tests(void)
{
    CuSuite *suite = CuSuiteNew();

    TESTS_ADD_GROUP(suite, strconv);

    return suite;
}