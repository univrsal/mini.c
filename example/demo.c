#include <mini.h>

int main()
{
    mini_t *ini = mini_try_load("./test.ini");

    mini_set_string(ini, NULL, "test_string", "test_value");
    mini_set_int(ini, "parent::child", "test_int", 1337);
    mini_set_int(ini, "parent::child", "test_int2", -1337);
    mini_set_double(ini, "parent", "double", 3.141);
    mini_set_bool(ini, "parent::child::baby", "bool", 0);

    mini_delete_value(ini, "test_group", "id");

    const char *test = mini_get_string(ini, NULL, "test_string", "error");
    const char *test2 = mini_get_string(ini, "test_group", "test_string", "error");
    const char *test3 = mini_get_string(ini, "test_group", "id", "error");
    double d = mini_get_double(ini, "parent", "double", 333);

    printf("test_string=%s\n", test);
    printf("test_string=%s\n", test2);
    printf("test_string=%s\n", test3);
    printf("test_string=%lf\n", d);

    mini_save(ini);

    mini_free(ini);
    return 0;
}
