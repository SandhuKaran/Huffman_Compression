#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define SYMBOL_COUNT 256
#define FILENAME_MAX_SIZE 256
#define BUFFER_SIZE 256

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

huff_node_t *huff_heap_pop(huff_heap_t *heap) {
    huff_node_t *node = heap->nodes[0];
    heap->nodes[0] = heap->nodes[--heap->count];
    int i = 0;
    while (true) {
        int left_child = i * 2 + 1;
        int right_child = i * 2 + 2;
        if (left_child >= heap->count) break;
        int min_child = left_child;
        if (right_child < heap->count && heap->nodes[right_child]->frequency < heap->nodes[left_child]->frequency) {
            min_child = right_child;
        }
        if (heap->nodes[i]->frequency <= heap->nodes[min_child]->frequency) break;
        huff_node_t *tmp = heap->nodes[i];
        heap->nodes[i] = heap->nodes[min_child];
        heap->nodes[min_child] = tmp;
        i = min_child;
    }
    return node;
}

huff_node_t *huff_build_tree(int *frequencies) {
    huff_heap_t *heap = new_huff_heap();
    for (int i = 0; i < SYMBOL_COUNT; i++) {
        if (frequencies[i] > 0) {
            huff_node_t *node = new_huff_node((char) i, frequencies[i], NULL, NULL);
            huff_heap_push(heap, node);
        }
    }
    while (heap->count > 1) {
        huff_node_t *node1 = huff_heap_pop(heap);
        huff_node_t *node2 = huff_heap_pop(heap);
        huff_node_t *parent = new_huff_node('\0', node1->frequency + node2->frequency, node1, node2);
        huff_heap_push(heap, parent);
    }
    huff_node_t *root = huff_heap_pop(heap);
    free(heap);
    return root;
}

void huff_build_codes(huff_code_t *codes, huff_node_t *node, char *buffer, int depth) {
    if (node->left == NULL && node->right == NULL) {
        codes[node->symbol].symbol = node->symbol;
        codes[node->symbol].code = (char *) malloc((depth + 1) * sizeof(char));
        strcpy(codes[node->symbol].code, buffer);
        return;
    }
    buffer[depth] = '0';
    buffer[depth + 1] = '\0';
    huff_build_codes(codes, node->left, buffer, depth + 1);
    buffer[depth] = '1';
    buffer[depth + 1] = '\0';
    huff_build_codes(codes, node->right, buffer, depth + 1);
}

void huff_compress_file(char *input_file, char *output_file, huff_code_t *codes) {
    FILE *in_file = fopen(input_file, "rb");
    if (in_file == NULL) {
        fprintf(stderr, "Error: failed to open input file '%s'\n", input_file);
        exit(1);
    }
    FILE *out_file = fopen(output_file, "wb");
    if (out_file == NULL) {
        fprintf(stderr, "Error: failed to open output file '%s'\n", output_file);
        fclose(in_file);
        exit(1);
    }
    char buffer[BUFFER_SIZE];
    int bits_in_buffer = 0;
    char bit_buffer = 0;
    int c = fgetc(in_file);
    while (c != EOF) {
        char *code = codes[c].code;
        for (int i = 0; i < strlen(code); i++) {
            bit_buffer <<= 1;
            if (code[i] == '1') bit_buffer |= 1;
            bits_in_buffer++;
            if (bits_in_buffer == 8) {
                fputc(bit_buffer, out_file);
                bit_buffer = 0;
                bits_in_buffer = 0;
            }
        }
        c = fgetc(in_file);
    }
    if (bits_in_buffer > 0) {
        bit_buffer <<= (8 - bits_in_buffer);
        fputc(bit_buffer, out_file);
    }
    fclose(in_file);
    fclose(out_file);
}

void huff_decompress_file(char *input_file, char *output_file, huff_node_t *root) {
    FILE *in_file = fopen(input_file, "rb");
    if (in_file == NULL) {
        fprintf(stderr, "Error: failed to open input file '%s'\n", input_file);
        exit(1);
    }
    FILE *out_file = fopen(output_file, "wb");
    if (out_file == NULL) {
        fprintf(stderr, "Error: failed to open output file '%s'\n", output_file);
        fclose(in_file);
        exit(1);
    }
    huff_node_t *node = root;
    int bits_left = 0;
    char bit_buffer = 0;
    int c = fgetc(in_file);
    while (c != EOF) {
        bit_buffer = c;
        bits_left = 8;
        while (bits_left > 0) {
            if ((bit_buffer & 0x80) == 0x80) {
                node = node->right;
            } else {
                node = node->left;
            }
            if (node->left == NULL && node->right == NULL) {
                fputc(node->symbol, out_file);
                node = root;
            }
            bit_buffer <<= 1;
            bits_left--;
        }
        c = fgetc(in_file);
    }
    fclose(in_file);
    fclose(out_file);
}


int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage: %s <input file> <output file>\n", argv[0]);
        exit(1);
    }
    char *input_file = argv[1];
    char *output_file = argv[2];
    int frequencies[SYMBOL_COUNT] = {0};
    huff_count_frequencies(input_file, frequencies);
    huff_node_t *root = huff_build_tree(frequencies);
    huff_code_t codes[SYMBOL_COUNT];
    memset(codes, 0, sizeof(codes));
    char buffer[BUFFER_SIZE];
    huff_build_codes(codes, root, buffer, 0);
    huff_compress_file(input_file, output_file, codes);
    huff_decompress_file(output_file, "decompressed.bin", root);
    free_huff_tree(root);
    for (int i = 0; i < SYMBOL_COUNT; i++) {
        if (codes[i].code != NULL) {
            free(codes[i].code);
        }
    }
    return 0;
}
