/**
 ** This file is part of the minic project.
 ** Copyright 2020 univrsal <uni@vrsal.cf>.
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

#ifndef MINI_C_H
#define MINI_C_H

#ifdef __cplusplus
#extern "C" {
#endif

#include <stdio.h>

#ifndef MINI_CHUNK_SIZE
#define MINI_CHUNK_SIZE 1024
#endif

enum mini_result {
    MINI_OK,
    MINI_INVALID_ARG,
    MINI_INVALID_PATH,
    MINI_INVALID_TYPE,
    MINI_INVALID_ID,
    MINI_DUPLICATE_ID,
    MINI_FILE_NOT_FOUND,
    MINI_GROUP_NOT_FOUND,
    MINI_VALUE_NOT_FOUND,
    MINI_ACCESS_DENIED,
    MINI_READ_ERROR,
    MINI_CONVERSION_ERROR,
    /* Flag errors, will occur independently of the above errors */
    MINI_INVALID_GROUP = 1 << 4,
    MINI_UNKNOWN
};

typedef struct mini_value_s {
    char *id;                   /* The id of this item                  */
    char *val;                  /* The value for this item              */
    struct mini_value_s *next;  /* The next value in this group         */
    struct mini_value_s *prev;
} mini_value_t;

typedef struct mini_group_s {
    char *id;                   /* The id of this group                 */
    struct mini_group_s *next;  /* The next group on the same level     */
    struct mini_group_s *prev;
    mini_value_t *value;        /* The first value for this group       */
} mini_group_t;

typedef struct mini_s {
    char *path;
    mini_group_t *root;
} mini_t;


/* Load from FILE instance, you will have to set path in the returned struct
 * manually otherwise mini_save will not work */
/* Loading with optional error code, can be NULL,
 * will contain either MINI_OK, or any MINI_* error*/
mini_t *mini_load_ex(const char *path, int *err);
mini_t *mini_loadf_ex(FILE *f, int *err);
mini_t *mini_try_load_ex(const char *path, int *err);

/* Load or return an empty file if it doesn't exist */
static inline mini_t *mini_try_load(const char *path)
{
    return mini_try_load_ex(path, NULL);
}

/* Load from path */
static inline mini_t *mini_load(const char *path)
{
    return mini_load_ex(path, NULL);
}

static inline mini_t *mini_loadf(FILE *f)
{
    return mini_loadf_ex(f, NULL);
}

int mini_save(const mini_t *mini);
int mini_savef(const mini_t *mini, FILE *f);

void mini_free(mini_t *mini);

int mini_value_exists(mini_t *mini, const char *group, const char *id);

/* Data creation/retrival/deleting
 * For all set/get methods group can be NULL
 * which will then use the root group.
 */

int mini_delete_value(mini_t *mini, const char *group, const char *id);
int mini_delete_group(mini_t *mini, const char *group);

int mini_set_string(mini_t *mini, const char *group, const char *id, const char *val);
int mini_set_int(mini_t *mini, const char *group, const char *id, long long val);
int mini_set_double(mini_t *mini, const char *group, const char *id, double val);

#define mini_set_bool(m, g, i, v) mini_set_int(m, g, i, v)

const char *mini_get_string_ex(mini_t *mini, const char *group, const char *id, const char *fallback, int *err);
long long mini_get_int_ex(mini_t *mini, const char *group, const char *id, long long fallback, int *err);
double mini_get_double_ex(mini_t *mini, const char *group, const char *id, double fallback, int *err);

#define mini_get_bool(m, g, i, v) mini_get_int(m, g, i, v)
#define mini_get_bool_ex(m, g, i, v, e) mini_get_int_ex(m, g, i, v, e)

static inline const char *mini_get_string(mini_t *mini, const char *group, const char *id, const char *fallback)
{
    return mini_get_string_ex(mini, group, id, fallback, NULL);
}

static inline long long mini_get_int(mini_t *mini, const char *group, const char *id, long long fallback)
{
    return mini_get_int_ex(mini, group, id, fallback, NULL);
}

static inline double mini_get_double(mini_t *mini, const char *group, const char *id, double fallback)
{
    return mini_get_double_ex(mini, group, id, fallback, NULL);
}

#ifdef __cplusplus
}
#endif

#endif
