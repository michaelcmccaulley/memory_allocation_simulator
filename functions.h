/*
Date: 08/08/2024
Class: CS4541 Computer Systems
Assignment: Memory Allocation Simulator
File: Header file (functions.h)
Author(s): Michael McCaulley
*/


#ifndef FUNCTIONS_H
#define FUNCTIONS_H


#include <stdint.h>
#include <stdio.h>


#define WORD_SIZE uint32_t
#define MAX_HEAP_SIZE 100000
#define MIN_HEAP_SIZE 1000


typedef struct memory_block {
    int payload_index;
    int size;
    int allocated;
    struct memory_block *next;
    struct memory_block *prev;
} MEMORY_BLOCK;


typedef struct explicit_list {
    struct memory_block *block;
    struct explicit_list *next;
    struct explicit_list *prev;
} EXPLICIT_LIST;


// Function declarations
EXPLICIT_LIST* find_free_block_explicit(int size, char strategy);
int* myrealloc(int size, void *address_pointer);
MEMORY_BLOCK* combine_memory_blocks(char* link, MEMORY_BLOCK *block, EXPLICIT_LIST *explicit_free_node);
MEMORY_BLOCK* consolidate_free_memory(MEMORY_BLOCK *block);
MEMORY_BLOCK* find_free_block_implicit(int size, char strategy);
void create_heap();
void* myalloc(int size);
void myfree(void *address_pointer);
void mysbrk(int size);
void read_and_process_input_file(FILE *input_file);
void separate_memory_block(size_t size, MEMORY_BLOCK* block, EXPLICIT_LIST *explicit_free_node);
void write_output_file();


// Global variables (extern declarations)
extern unsigned int heap[MAX_HEAP_SIZE];
extern unsigned int heap_size;
extern void* allocation_tracking_heap[MIN_HEAP_SIZE];
extern MEMORY_BLOCK *implicit_list_head;
extern EXPLICIT_LIST *explicit_list_head;
extern char allocation_strategy;
extern char allocation_method;


#endif // FUNCTIONS_H