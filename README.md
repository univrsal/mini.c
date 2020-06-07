# mini.c
Minimal ini file read/write library.
- Plain C
- Split into one header and one implementaion file
- < 600 LOC
- Supports strings, integers, floats
- Simple interface
- Permissive license

### Usage
See [demo.c](./example/demo.c). Basically:
```C
// Load or create if no file was found
mini_t *ini = mini_try_load("./test.ini");

// Set values
mini_set_string(ini, NULL, "string", "string_value"); // NULL for root group
mini_set_int(ini, "group1", "int1", 1337);
mini_set_int(ini, "group1", "int2", -1337);
mini_set_double(ini, "group2", "double", 3.141);
mini_set_bool(ini, "group3", "bool", 0);

// Retrieving values, with error checking and fallback values
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
    
// Deleting values
if (mini_delete_value(ini, "test_group", "id") != MINI_OK) {
    printf("Value %s in group %s not found\n", "id", "test_group");
} else {
    printf("Value %s in group %s deleted\n", "id", "test_group");
}

if (mini_delete_value(ini, "missin_group", "id") != MINI_OK) {
    printf("Value %s in group %s not found\n", "id", "missing_group");
}

mini_save(ini); // Write to disk
mini_free(ini); // Free memory
```
