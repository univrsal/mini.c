#include "mini.h"
#include <malloc.h>
#include <sys/stat.h>
#include <string.h>

mini_t *mini_load(const char *path)
{
    return mini_load_ex(path, NULL);
}

mini_t *mini_loadf(FILE *f)
{
    return mini_loadf_ex(f, NULL);
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
    mini_t *result = malloc(sizeof(mini_t));
    mini_group_t *current;
    char buffer[MINI_CHUNK_SIZE]
    size_t nread = 0;
     
    result->path = NULL;
    result->root = make_group(NULL);
    
    current = result->root;

    while (fgets(buffer, sizeof(buffer), f)) {
        buffer[MINI_CHUNK_SIZE - 1] = '\0';

        if (strlen(buffer) < 1) {
            continue;
        } else if (buffer[0] == '[') {
            /* Group header */
            buffer[strlen(buffer) - 1] = '\0'; /* Remove ']' */
            current = get_group(result, buffer + 1); /* Skip '[' */
        } else {
            add_value(current, buffer);
        }
    }
}

int mini_save(const minit_t *mini)
{
    int result = NULL;
    struct stat buf;
    
    if (!mini->path || strlen(mini->path < 1)) {
        result = MINI_INVALID_PATH;
    } else if (stat(mini->path, &buf) != 0) {
        result = MINI_FILE_NOT_FOUND;
    } else {
        FILE *fp = fopen(mini->path, "r");
        if (fp) {
            result = mini_savef(mini, fp);
            fclose(fp);
        } else {
            result = MINI_ACCESS_DENIED;
        }
    }

    return result;
}

void write_group(const mini_group_t *g, FILE *f, char *parent_groups)
{
    char buf[MINI_CHUNK_SIZE];
    char *old_header = NULL;
    mini_value_t *cval = g->value;
    mini_group_t *cgrp = g->next; 

    /* Write header for this group, if this group is on the top level,
     * we only write the groups name since it doesn't have a parent */
    if (g->parent) { /* Root group doesn't have a header so it'll skip this */
        if (strlen(parent_groups) > 0) { 
            old_header = strdup(parent_groups);
            fprintf(f, "[%s::%s]\n", parent_groups, g->id);
            snprintf(parent_groups, MINI_CHUNK_SIZE - 1, "%s::%s", parent_groups, g->id);
        } else {
            fprintf(f, "[%s]\n",g->id);
            snprintf(parent_groups, MINI_CHUNK_SIZE - 1, "%s", g->id);
        }
    }
    
    /* Write all values of this group */ 
    while (cval) {
        fprintf(f, "%s=%s\n", cval->id, cval->val);
        cval = cval->next;
    }
    
    /* Write children */ 
    if (g->child) 
        write_group(g->child, f, parent_groups);
    
    strncpy(parent_groups, old_header, MINI_CHUNK_SIZE - 1);
    free(old_header);

    /* Write all groups on this level */
    while (cgrp) {
        write_group(cgrp, f, parent_group);
        cgrp = cgrp->next;
    }
}

int mini_savef(const mini_t *mini, FILE *f)
{
    if (!f)
        return MINI_INVALID_ARG;
    char buf[MINI_CHUNK_SIZE];
    buf[0] = '\0';

    write_group(mini->root, f, buf);
    return MINI_OK;
}

mini_group_t make_group(const char *name)
{
    mini_group_t *g = malloc(sizeof(mini_group_t));
    if (name)
        g->id = strdup(name);
    else
        g->id = NULL;
    
    g->child = NULL;
    g->next = NULL;
    g->value = NULL;
    return g;
}

mini_value_t *make_value()
{
    mini_value_t *val = malloc(sizeof(mini_value_t));
    val->id = NULL;
    val->val = NULL;
    val->next = NULL;
}

void free_value(mini_value_t *v)
{
    if (v) {
        free(v->id);
        free(v->val);
        free(v);

        v->id = NULL;
        v->val = NULL;
    }
}

void free_group(mini_group *g)
{
    if (g) {
        free(g->id);
        free(g);
        g->id = NULL;
    }
}

void add_value(mini_group_t *group, char *line)
{
    mini_value_t *n = make_value();
    char *id = strtok(line, "=");
    
    if (strlen(id) < 1) {
        free_value(n);
        return;
    }
    
    n->id = strdup(id);
    n->val = strdup(strtok(NULL, ""));
    
    n->next = group->value;
    group->value = n;
}

void add_group(mini_group_t *parent, const mini_group_t *child)
{
    child->next = parent->next;
    parent->next = child;
}

/* Create a group tree from a string of the form parent::child::baby
 * and add it to the parent group
 * returns the 'baby' group
 */
mini_group_t *create_group(mini_group_t *parent, char *name)
{
    mini_group_t *current = parent;
    mini_group_t *n = NULL;

    while (name) {
        n = make_group(name);
        add_group(current, n);

        current = n;
        name = strtok(NULL, "::");
    }
    return current;
}

mini_group_t *get_group(mini_t *mini, char *header, int create)
{
    char *split = strtok(header, "::");
    mini_group_t *c = mini->root;
    mini_group_t *result = NULL;
    mini_group_t *last = c;

    while (split) {

        /* Go over all groups on this level */
        while (c) {
            if (strcmp(c->id, split) == 0)
                break;
            c = c->next;
        }

        /* Didn't find any group at this level with a matching id */
        if (!c && create) {
            /* Create this group and any sub groups and return it*/
            return create_group(last, split);
        } else {
            return NULL;
        }

        last = c;

        split = strtok(NULL, "::");
        if (split)
            c = c->child;
    }

    return c;
}

