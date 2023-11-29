#ifndef CIRCULAR_LINKED_LIST_H
#define CIRCULAR_LINKED_LIST_H

#include <stdio.h>
#include <stdlib.h>

typedef struct cll_node_struct {
    int data;
    struct cll_node_struct *next;
    struct cll_node_struct *prev;
} cll_node_t;

typedef struct cll_struct {
    cll_node_t *head;
    int count;
} cll_t;

cll_t *cll_init();
void cll_append(cll_t *cll, int data);
int cll_get(cll_t *cll);
void cll_next(cll_t *cll);
void cll_delete(cll_t *cll);
void cll_print(cll_t *cll);
void cll_free(cll_t *cll);

#endif