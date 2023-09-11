/**
 ** This file is part of the minic project.
 ** Copyright 2023 univrsal <uni@vrsal.xyz>.
 ** All rights reserved.
 **
 ** Redistribution and use in source and binary forms, with or without
 ** modification, are permitted provided that the following conditions are
 ** met:
 **
 ** 1. Redistributions of source code must retain the above copyright notice,
 **    this list of conditions and the following disclaimer.
 **
 ** 2. Redistributions in binary form must reproduce the above copyright
 **    notice, this list of conditions and the following disclaimer in the
 **    documentation and/or other materials provided with the distribution.
 **
 ** THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND ANY
 ** EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 ** DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR ANY
 ** DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 ** (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 ** SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 ** CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 ** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 ** OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 ** SUCH DAMAGE.
 **/

#include <mini.h>

int main()
{
    mini_t *ini = mini_try_load("./test.ini");

    /* Creating/settings values */
    mini_set_string(ini, NULL, "string", "string_value");
    mini_set_string(ini, "test_group", "id", "string_value");
    mini_set_int(ini, "group1", "int1", 1337);
    mini_set_int(ini, "group1", "int2", -1337);
    mini_set_double(ini, "group2", "double", 3.141);
    mini_set_bool(ini, "group3", "bool", 0);

    /* Retrieving values */
    int err = MINI_OK, err2 = MINI_OK;
    const char *test = mini_get_string(ini, NULL, "string", "error");
    const char *test2 = mini_get_string(ini, "test_group", "test_string", "fallback1");
    const char *test3 = mini_get_string_ex(ini, "test_group", "id", "fallback2", &err);
    mini_get_string_ex(ini, "missing_group", "id", "fallback3", &err2);
    int int1 = mini_get_double(ini, "group1", "int1", 333);
    int int2 = mini_get_double(ini, "group1", "int2", -333);
    double d = mini_get_double(ini, "group2", "double", 333.33);
    int b = mini_get_bool(ini, "group3", "bool", 0);

    if (err == MINI_VALUE_NOT_FOUND)
        printf("Value not found\n");
    if (err2 == MINI_GROUP_NOT_FOUND)
        printf("Group not found\n");

    printf("string=%s\n", test);
    printf("[test_group] test_string=%s\n", test2);
    printf("[test_group] id=%s\n", test3);
    printf("[missing_group] id=%lf\n", d);
    printf("[group1] int1=%i\n", int1);
    printf("[group1] int2=%i\n", int2);
    printf("[group2] double=%lf\n", d);
    printf("[group3] bool=%i\n", b);

    /* Deleting values */
    if (mini_delete_value(ini, "test_group", "id") != MINI_OK) {
        printf("Value %s in group %s not found\n", "id", "test_group");
    } else {
        printf("Value %s in group %s deleted\n", "id", "test_group");
    }

    if (mini_delete_value(ini, "missin_group", "id") != MINI_OK) {
        printf("Value %s in group %s not found\n", "id", "missing_group");
    }

    mini_save(ini); /* Write to disk */
    mini_free(ini); /* Free memory */

    return 0;
}
