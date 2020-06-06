
#ifndef MINI_C_H
#define MINI_C_H
#include <stdio.h>

#ifndef MINI_CHUNK_SIZE
#define MINI_CHUNK_SIZE 1024
#endif

enum mini_result {
    MINI_OK,
    MINI_INVALID_ARG,
    MINI_INVALID_PATH,
    MINI_FILE_NOT_FOUND,
    MINI_ACCESS_DENIED,
    MINI_INVALID_TYPE,
    MINI_READ_ERROR,
    MINI_UNKNOWN
};

typedef struct mini_value_s {
    char *id;                   /* The id of this item                  */
    char *val;                  /* The value for this item              */
    struct mini_value_s *next;  /* The next value in this group         */
} mini_value_t;

typedef struct mini_group_s {
    char *id;                   /* The id of this group                 */
    struct mini_group_s *child; /* The first child group                */
    struct mini_group_s *parent; 
    struct mini_group_s *next;  /* The next group on the same level     */
    mini_value_t *value;        /* The first value for this group       */
} mini_group_t;

typedef struct mini_s {
    char *path;
    mini_group_t *root;
} mini_t;


mini_t *mini_load(const char *path);
mini_t *mini_loadf(FILE *f);

mini_t *mini_load_ex(const char *path, int *err);
mini_t *mini_loadf_ex(FILE *f, int *err);

int mini_save(const minit_t *mini);
int mini_savef(const mini_t *mini, FILE *f);

void mini_free(mini_t *mini);
#endif
