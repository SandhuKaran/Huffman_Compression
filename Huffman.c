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

void free_huff_node(huff_node_t *node) {
    if (node == NULL) return;
    free_huff_node(node->left);
    free_huff_node(node->right);
    free(node);
}

huff_heap_t *new_huff_heap() {
    huff_heap_t *heap = (huff_heap_t *) malloc(sizeof(huff_heap_t));
    heap->nodes = (huff_node_t **) malloc(SYMBOL_COUNT * sizeof(huff_node_t *));
    heap->count = 0;
    return heap;
}

void free_huff_heap(huff_heap_t *heap) {
    for (int i = 0; i < heap->count; i++) {
        free_huff_node(heap->nodes[i]);
    }
    free(heap->nodes);
    free(heap);
}

void huff_heap_push(huff_heap_t *heap, huff_node_t *node) {
    heap->nodes[heap->count++] = node;
    int i = heap->count - 1;
    while (i > 0) {
        int parent = (i - 1) / 2;
        if (heap->nodes[i]->frequency >= heap->nodes[parent]->frequency) break;
        huff_node_t *tmp = heap->nodes[i];
        heap->nodes[i] = heap->nodes[parent];
        heap->nodes[parent] = tmp;
        i = parent;
    }
}

