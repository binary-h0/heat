#include "circular_linked_list.h"

cll_t *cll_init() {
    cll_t *cll = (cll_t *)malloc(sizeof(cll_t));
    cll->head = NULL;
    cll->count = 0;
    return cll;
}

void cll_append(cll_t *cll, int data) {
    cll_node_t *node = (cll_node_t *)malloc(sizeof(cll_node_t));
    node->data = data;
    node->next = NULL;
    node->prev = NULL;

    if (cll->count == 0) {
        cll->head = node;
        node->next = node;
        node->prev = node;
    } else {
        node->next = cll->head;
        node->prev = cll->head->prev;
        cll->head->prev->next = node;
        cll->head->prev = node;
    }
    cll->count++;
}

int cll_get(cll_t *cll) {
    if (cll->head == NULL) {
        return -1;
    } else {
        return cll->head->data;
    }
}

void cll_next(cll_t *cll) {
    if (cll->head == NULL) {
        return;
    } else {
        cll->head = cll->head->next;
    }
}

void cll_delete(cll_t *cll) {
    if (cll->head == NULL) {
        return;
    } else {
        cll_node_t *temp = cll->head;
        cll->head->prev->next = cll->head->next;
        cll->head->next->prev = cll->head->prev;
        cll->head = cll->head->next;
        free(temp);
        cll->count--;
    }
}

void cll_print(cll_t *cll) {
    cll_node_t *temp = cll->head;
    for (int i = 0; i < cll->count; i++) {
        printf("%d ", temp->data);
        temp = temp->next;
    }
    printf("\n");
}

void cll_free(cll_t *cll) {
    cll_node_t *temp = cll->head;
    for (int i = 0; i < cll->count; i++) {
        cll_node_t *next = temp->next;
        free(temp);
        temp = next;
    }
    free(cll);
}