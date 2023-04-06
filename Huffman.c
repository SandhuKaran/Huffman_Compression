#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

typedef struct huff_node {
    char symbol;
    int frequency;
    struct huff_node *left;
    struct huff_node *right;
} huff_node_t;

typedef struct huff_heap {
    huff_node_t **nodes;
    int count;
} huff_heap_t;

typedef struct huff_code {
    char symbol;
    char *code;
} huff_code_t;

huff_node_t *new_huff_node(char symbol, int frequency, huff_node_t *left, huff_node_t *right) {
    huff_node_t *node = (huff_node_t *) malloc(sizeof(huff_node_t));
    node->symbol = symbol;
    node->frequency = frequency;
    node->left = left;
    node->right = right;
    return node;
}

