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

#ifdef __cplusplus
extern "C" {
#endif

#include "mini.h"
#include <malloc.h>
#include <sys/stat.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>

/* === Utilities === */

mini_value_t *make_value()
{
    mini_value_t *val = malloc(sizeof(mini_value_t));
    val->id = NULL;
    val->val = NULL;
    val->next = NULL;
    val->prev = NULL;
    return val;
}

void free_value(mini_value_t *v)
{
    if (v) {
        free(v->id);
        free(v->val);
        v->id = NULL;
        v->val = NULL;
        v->next = NULL;
        v->prev = NULL;
        free(v);
    }
}

mini_group_t *make_group(const char *name)
{
    mini_group_t *g = malloc(sizeof(mini_group_t));
    if (name)
        g->id = strdup(name);
    else
        g->id = NULL;

    g->next = NULL;
    g->prev = NULL;
    g->value = NULL;
    return g;
}

void free_group(mini_group_t *g)
{
    if (g) {
        free(g->id);
        g->next = NULL;
        g->prev = NULL;
        g->id = NULL;
        g->value = NULL;
        free(g);
    }
}

void free_group_children(mini_group_t *g)
{
    if (!g)
        return;

    mini_group_t *cgrp = g, *ngrp = NULL;
    mini_value_t *cval = NULL, *nval = NULL;

    while (cgrp) {
        cval = cgrp->value;
        while (cval) {
            nval = cval->next;
            free_value(cval);
            cval = nval;
        }

        ngrp = cgrp->next;
        free_group(cgrp);
        cgrp = ngrp;
    }
}

mini_value_t *get_group_value(mini_group_t *grp, const char *id)
{
    mini_value_t *result = grp->value;
    while (result) {
        if (strcmp(result->id, id) == 0)
            return result;
        result = result->next;
    }
    return NULL;
}

int add_value(mini_group_t *group, const char *id, const char *val)
{
    if (get_group_value(group, id))
        return MINI_DUPLICATE_ID;

    mini_value_t *n = make_value();
    n->id = strdup(id);
    n->val = strdup(val);
    n->next = group->value;
    if (group->value)
        group->value->prev = n;
    group->value = n;

    return MINI_OK;
}

int parse_value(mini_group_t *group, char *line)
{
    char *id = strtok(line, "=");

    if (strlen(id) < 1)
        return MINI_INVALID_ID;
    else if (get_group_value(group, id))
        return MINI_DUPLICATE_ID;

    return add_value(group, id, strtok(NULL, ""));
}

void add_group(mini_t *mini, mini_group_t *grp)
{
    grp->next = NULL;
    grp->prev = mini->tail;

    if (mini->tail)
        mini->tail->next = grp;
    mini->tail = grp;
}

mini_group_t *create_group(mini_t *mini, const char *name)
{
    mini_group_t *n = NULL;
    n = make_group(name);
    add_group(mini, n);
    return n;
}

mini_group_t *get_group(mini_t *mini, char *id, int create)
{
    if (!id)
        return mini->head;

    mini_group_t *c = mini->head->next;

    /* Go over all groups on this level */
    while (c) {
        if (strcmp(c->id, id) == 0)
            break;
        c = c->next;
    }

    /* Didn't find any group */
    if (!c && create)
        return create_group(mini, id);
    return c;
}

mini_value_t *get_value(mini_t *mini, const char *group, const char *id, int *err,
                        mini_group_t **group_ptr)
{
    char *tmp = NULL;
    mini_value_t *result = NULL;
    if (group)
        tmp = strdup(group);
    mini_group_t *grp = get_group(mini, tmp, 0);

    if (grp) {
        if (group_ptr)
            *group_ptr = grp;
        result = get_group_value(grp, id);
        if (!result && err)
            *err = MINI_VALUE_NOT_FOUND;
    } else if (err){
        *err = MINI_GROUP_NOT_FOUND;
    }

    free(tmp);
    return result;
}

void write_group(const mini_group_t *g, FILE *f)
{
    mini_value_t *cval = g->value;

    if (g->prev) /* Root group doesn't have a header so it'll skip this */
        fprintf(f, "[%s]\n",g->id);

    /* Write all values of this group */
    while (cval) {
        fprintf(f, "%s=%s\n", cval->id, cval->val);
        cval = cval->next;
    }
}

/* === API implementation === */

mini_t *mini_create(const char *path)
{
    mini_t *result =  malloc(sizeof(mini_t));
    if (path)
        result->path = strdup(path);
    result->head = make_group(NULL);
    result->tail = result->head;
    return result;
}

mini_t *mini_try_load_ex(const char *path, int *err)
{
    mini_t *result = mini_load_ex(path, err);

    if (!result)
        result = mini_create(path);
    return result;
}

mini_t *mini_load_ex(const char *path, int *err)
{
    mini_t *result = NULL;
    struct stat buf;

    if (stat(path, &buf) != 0) {
        if (err)
            *err = MINI_FILE_NOT_FOUND;
    } else {
        FILE *fp = fopen(path, "r");

        if (fp) {
            result =  mini_loadf(fp);
            result->path = strdup(path);
            fclose(fp);
        } else if (err) {
            *err = MINI_ACCESS_DENIED;
        }
    }

    return result;
}

mini_t *mini_loadf_ex(FILE *f, int *err)
{
    mini_t *result = mini_create(NULL);
    mini_group_t *current;
    char buffer[MINI_CHUNK_SIZE];

    current = result->head;

    while (fgets(buffer, sizeof(buffer), f)) {
        buffer[MINI_CHUNK_SIZE - 1] = '\0';

        if (strlen(buffer) < 1 || buffer[0] == '\n') {
            continue;
        } else if (buffer[0] == '[') {
            /* Group header */
            buffer[strlen(buffer) - 2] = '\0'; /* Remove ']\n' */
            mini_group_t *n = get_group(result, buffer + 1, 1); /* Skip '[' */

            if (n)
                current = n;
            else if (err)
                *err |= MINI_INVALID_GROUP;
        } else {
            parse_value(current, buffer);
        }
    }

    return result;
}

int mini_save(const mini_t *mini)
{
    int result = MINI_OK;

    if (!mini->path || strlen(mini->path) < 1) {
        result = MINI_INVALID_PATH;
    } else {
        FILE *fp = fopen(mini->path, "w");
        if (fp) {
            result = mini_savef(mini, fp);
            fclose(fp);
        } else {
            result = MINI_ACCESS_DENIED;
        }
    }

    return result;
}

int mini_savef(const mini_t *mini, FILE *f)
{
    if (!f)
        return MINI_INVALID_ARG;

    mini_group_t *grp = mini->head;

    while (grp) {
        write_group(grp, f);
        grp = grp->next;
        /* Unless this is the last group, add an empty line
         * to sparate groups */
        if (grp)
            fprintf(f, "\n");
    }
    return MINI_OK;
}

void mini_free(mini_t *mini)
{
    if (mini) {
        free(mini->path);
        free_group_children(mini->head);
        mini->path = NULL;
        mini->tail = NULL;
        free(mini);
    }
}

int mini_delete_value(mini_t *mini, const char *group, const char *id)
{
    if (!mini || !id)
        return MINI_INVALID_ARG;
    int result = MINI_OK;
    mini_group_t *grp = NULL;
    mini_value_t *v = get_value(mini, group, id, &result, &grp);

    if (v) {
        if (v->next)
            v->next->prev = v->prev;
        if (v->prev)
            v->prev->next = v->next;
        if (v == grp->value)
            grp->value = v->next;
        free_value(v);
    }
    return result;
}

int mini_delete_group(mini_t *mini, const char *group)
{
    if (!mini)
        return MINI_INVALID_ARG;
    int result = MINI_OK;
    char *tmp = strdup(group);
    mini_group_t *grp = get_group(mini, tmp, 0);
    free(tmp);

    if (grp) {
        free_group_children(grp);
        if (grp->next)
            grp->next->prev = grp->prev;
        if (grp->prev)
            grp->prev->next = grp->next;
        free_group(grp);
    } else {
        result = MINI_GROUP_NOT_FOUND;
    }
    return result;

}

int mini_value_exists(mini_t *mini, const char *group, const char *id)
{
    if (!mini || !id)
        return MINI_INVALID_ARG;
    int result = MINI_OK;
    get_value(mini, group, id, &result, NULL);
    return result;
}

int mini_set_string(mini_t *mini, const char *group, const char *id, const char *val)
{
    if (!mini || !id)
        return MINI_INVALID_ARG;
    int result = MINI_OK;
    mini_group_t *grp = NULL;
    mini_value_t *v = get_value(mini, group, id, &result, &grp);

    if (v) {
        free(v->val);
        v->val = strdup(val);
    } else{
        if (!grp)
            grp = create_group(mini, group);
        result = add_value(grp, id, val);
    }

    return result;
}

int mini_set_int(mini_t *mini, const char *group, const char *id, long long val)
{
    char buf[57];
    snprintf(buf, 56, "%lli", val);
    return mini_set_string(mini, group, id, buf);
}

int mini_set_double(mini_t *mini, const char *group, const char *id, double val)
{
    char buf[64];
    snprintf(buf, 56, "%lf", val);
    return mini_set_string(mini, group, id, buf);
}

const char *mini_get_string_ex(mini_t *mini, const char *group, const char *id, const char *fallback, int *err)
{
    if (!mini || !id)
        return fallback;
    const char *result = fallback;

    mini_value_t *v = get_value(mini, group, id, err, NULL);

    if (v)
        result = v->val;

    return result;

}

long long mini_get_int_ex(mini_t *mini, const char *group, const char *id, long long fallback, int *err)
{
    const char *val = mini_get_string_ex(mini, group, id, NULL, err);
    if (!val)
        return fallback;
    long long res = strtoimax(val, NULL, 10);
    if ((res == INTMAX_MAX || res == INTMAX_MIN) && errno == ERANGE &&
        err)
    {
        *err = MINI_CONVERSION_ERROR;
        res = fallback;
    }
    return res;
}

double mini_get_double_ex(mini_t *mini, const char *group, const char *id, double fallback, int *err)
{
    const char *val = mini_get_string_ex(mini, group, id, NULL, err);
    if (!val)
        return fallback;
    double res = 0;
    sscanf(val, "%lf", &res);
    return res;
}

#ifdef __cplusplus
}
#endif
