#include "defines.h"
#include <stdio.h>
void CuAssertIntUint64Equals_LineMsg(CuTest *tc, const char *file, int line, const char *message,
                                     uint64_t expected, uint64_t actual)
{
    char buf[STRING_MAX];
    if (expected == actual)
        return;
    sprintf(buf, "expected <%lu> but was <%lu>", expected, actual);
    CuFail_Line(tc, file, line, message, buf);
}
