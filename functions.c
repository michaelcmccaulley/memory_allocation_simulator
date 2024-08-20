/*
Date: 08/08/2024
Class: CS4541 Computer Systems
Assignment: Memory Allocation Simulator
File: Implementation file (functions.c)
Author(s): Michael McCaulley
*/

// Include necessarry libaries
#include "functions.h"
#include <stdlib.h>
#include <string.h>


// Define and initialize global variables
char allocation_method;
char allocation_strategy;
unsigned int alignment = 8;
unsigned int heap[MAX_HEAP_SIZE];
unsigned int heap_size = MIN_HEAP_SIZE * sizeof(WORD_SIZE);
void *allocation_tracking_heap[MIN_HEAP_SIZE];

// Define two structs to hold the head of the lists
MEMORY_BLOCK *implicit_list_head;
EXPLICIT_LIST *explicit_list_head;


// Function implements two memory allocation strategies: First Fit and Best Fit for explicit free list
EXPLICIT_LIST* find_free_block_explicit(int size, char strategy)
{
    // Start at the head of the explicit free list
    EXPLICIT_LIST *explicit_free_node = explicit_list_head;
    
    // Variable to keep track of the best fit block (only used for Best Fit strategy)
    EXPLICIT_LIST *best_node = 0;

    // If the free list is empty, return NULL
    if (explicit_list_head == 0)
    {
        return 0;
    }

    // Traverse the explicit free list
    while (explicit_free_node != 0)
    {
        // Check if the current block is large enough
        if (explicit_free_node->block->size >= size)
        {
            if (strategy == 'F') // First Fit strategy
            {
                // Return the first block that's large enough
                return explicit_free_node;
            }
            else if (strategy == 'B') // Best Fit strategy
            {
                // Update best_node if this is the first fit or a better fit than the previous best
                if (best_node == 0 || (explicit_free_node->block->size - size) < (best_node->block->size - size))
                {
                    best_node = explicit_free_node;
                }
            }
        }
        // Move to the next node in the list
        explicit_free_node = explicit_free_node->next;
    }

    // Return the best fit block (or NULL if no suitable block was found)
    return best_node;
}


// Function that reallocates memory by allocating a new block and freeing the old block
int* myrealloc(int size, void *address_pointer)
{ 
    // Allocate a new block of memory with the specified size
    void* new_ptr = myalloc(size);
    
    // Cast pointers to MEMORY_BLOCK for easier access to block metadata
    MEMORY_BLOCK *new_block = new_ptr;
    MEMORY_BLOCK *old_block = address_pointer;

    // Calculate the size of the old block in WORDs
    int old_block_size_in_words = (old_block->size + sizeof(WORD_SIZE) - 1) / sizeof(WORD_SIZE);

    // Ensure the block's size is aligned to 8 bytes
    old_block_size_in_words = (old_block_size_in_words + (alignment / sizeof(WORD_SIZE) - 1)) & ~(alignment / sizeof(WORD_SIZE) - 1);

    // Calculate the number of WORDs to copy (minimum of old and new block sizes)
    int copy_size_in_words = (size < old_block->size) ? size / sizeof(WORD_SIZE) : old_block_size_in_words;

    // Copy old payload into the new block
    for (int i = 0; i < copy_size_in_words; i++) {
        heap[new_block->payload_index + i] = heap[old_block->payload_index + i];
    }

    // Free the old block of memory
    myfree(address_pointer);

    // Return a pointer to the new block of memory
    return new_ptr;
}


// Function to combine two free blocks (block and block->prev), called by consolidate_free_memory function.
MEMORY_BLOCK* combine_memory_blocks(char* link, MEMORY_BLOCK *block, EXPLICIT_LIST *explicit_free_node)
{
    // Initialize the result to the previous block
    MEMORY_BLOCK *result = block->prev;

    // Define the word size
    const size_t word_size = sizeof(WORD_SIZE);

    // Initialize total_size and aligned_size to zero
    size_t total_size = 0;
    size_t aligned_size = 0;

    // Calculate total size and align it
    if (block->next != 0) 
    {
        // Calculate aligned size of the previous block
        aligned_size = ((result->size + word_size - 1) / word_size) * word_size;
        aligned_size = (aligned_size + 7) & ~7;  // Align to nearest multiple of 8

        // Calculate total size including header, footer, and current block size
        total_size = aligned_size + 2 * word_size + ((block->size + word_size - 1) / word_size) * word_size;
        total_size = (total_size + 7) & ~7;  // Align to nearest multiple of 8
    } 
    
    else 
    {
        // Calculate aligned size of the previous block
        aligned_size = ((result->size + word_size - 1) / word_size) * word_size;
        aligned_size = (aligned_size + 7) & ~7;  // Align to nearest multiple of 8

        // Calculate total size including header and footer
        total_size = aligned_size + 2 * word_size + block->size;
    }

    // Update size of the combined blocks
    result->size = total_size;

    // Update linked list pointers
    result->next = block->next;
    if (block->next != 0) 
    {
        block->next->prev = result;
    }

    // Handle explicit allocation method
    if (allocation_method == 'E') 
    {
        // Search for the explicit free node in the explicit list
        EXPLICIT_LIST *current = explicit_list_head;

        while (current != 0 && current->block != result && current->block != block) 
        {
            current = current->next;
        }

        explicit_free_node = current;

        if (explicit_free_node != 0) 
        {
            // Update block pointer in the explicit free node
            explicit_free_node->block = result;

            // Remove node from its current position
            if (explicit_free_node->prev != 0) 
            {
                explicit_free_node->prev->next = explicit_free_node->next;
            }
            if (explicit_free_node->next != 0) 
            {
                explicit_free_node->next->prev = explicit_free_node->prev;
            }

            // Handle right combination
            if (strcmp(link, "right") == 0 && explicit_free_node->next != 0) 
            {
                EXPLICIT_LIST* temp = explicit_free_node->next;
                explicit_free_node->next = temp->next;

                if (temp->next != 0) 
                {
                    temp->next->prev = explicit_free_node;
                }
                free(temp);
            }

            // Move node to the head of the explicit list
            explicit_free_node->next = explicit_list_head;
            explicit_free_node->prev = 0;

            if (explicit_list_head != 0) 
            {
                explicit_list_head->prev = explicit_free_node;
            }
            explicit_list_head = explicit_free_node;
        } 
        
        else 
        {
            // Create a new explicit free node and add to the head of the list
            explicit_free_node = malloc(sizeof(EXPLICIT_LIST));
            explicit_free_node->block = result;
            explicit_free_node->next = explicit_list_head;
            explicit_free_node->prev = 0;
            
            if (explicit_list_head != 0) 
            {
                explicit_list_head->prev = explicit_free_node;
            }
            explicit_list_head = explicit_free_node;
        }
    }

    // Free the current block
    free(block);

    // Return the combined block
    return result;
}

// Function attempts to merge a given memory block with its adjacent free blocks, ensuring proper memory alignment and updating the heap accordingly
MEMORY_BLOCK* consolidate_free_memory(MEMORY_BLOCK *block)
{
    // If it is the last block, consolidate with the remaining free space
    if (block->next == 0) 
    {
        // Ensure the block size is aligned to the nearest multiple of WORD_SIZE
        int aligned_size = (block->size + sizeof(WORD_SIZE) - 1) & ~(sizeof(WORD_SIZE) - 1);
        
        // Ensure the block size is aligned to the nearest multiple of 8 bytes
        aligned_size = (aligned_size + 7) & ~7;

        // Calculate the remaining free space and update the block size if needed
        if (block->payload_index + aligned_size < (heap_size / sizeof(WORD_SIZE))) 
        {
            int remaining_free_space = heap_size / sizeof(WORD_SIZE) - block->payload_index;
            block->size = remaining_free_space * sizeof(WORD_SIZE);
        }
    }

    // Initialize a pointer for an explicit free list node
    EXPLICIT_LIST *explicit_free_node = 0;

    // If the block is free, attempt to consolidate with neighboring free blocks
    if (block->allocated == 0) 
    {
        int memory_combined = 0;

        // Check and combine with the previous block if it is free
        if (block->prev != 0 && block->prev->allocated == 0) 
        {
            block = combine_memory_blocks("left", block, explicit_free_node);
            memory_combined = 1;
        }

        // Check and combine with the next block if it is free
        if (block->next != 0 && block->next->allocated == 0) 
        {
            block = combine_memory_blocks("right", block->next, explicit_free_node);
            memory_combined = 1;
        }

        // If no consolidating occurred, add the block to the explicit free list
        if (memory_combined == 0 && allocation_method == 'E') 
        {
            explicit_free_node = malloc(sizeof(EXPLICIT_LIST));
            explicit_free_node->block = block;
            explicit_free_node->next = explicit_list_head;
            explicit_free_node->prev = 0;

            if (explicit_list_head != 0) 
            {
                explicit_list_head->prev = explicit_free_node;
            }

            explicit_list_head = explicit_free_node;
        }
    }

    // Update heap for the block and its neighbors
    if (block->next != 0)
    {
        // Calculate aligned size of the block
        int aligned_size = block->size / sizeof(WORD_SIZE);

        if (block->size % sizeof(WORD_SIZE) > 0)
        {
            aligned_size++;
        }
        aligned_size = (aligned_size + (alignment / sizeof(WORD_SIZE) - 1)) & ~(alignment / sizeof(WORD_SIZE) - 1);

        // Calculate and set header and footer values for the block
        int header_footer_value = (aligned_size + 2) * 4;

        // Update the header in the heap
        heap[block->payload_index - 1] = header_footer_value; 

        // Update the footer in the heap
        heap[block->payload_index + aligned_size] = header_footer_value; 
    } 

    else if (block->next == 0)
    {
        // Calculate aligned size of the block
        int aligned_size = block->size / sizeof(WORD_SIZE);

        // Calculate and set header and footer values for the block
        int header_footer_value = (aligned_size - 1) * 4;
        heap[block->payload_index - 1] = header_footer_value; // header
        heap[block->payload_index + aligned_size - 3] = header_footer_value; // footer
    }

    return block;
}


// Function implements two memory allocation strategies: First Fit and Best Fit for implicit free list
MEMORY_BLOCK* find_free_block_implicit(int size, char strategy)
{
    // Start at the head of the implicit free list
    MEMORY_BLOCK *block = implicit_list_head;
    
    // Variable to keep track of the best fit block (only used for Best Fit strategy)
    MEMORY_BLOCK *best_block = 0;

    // If the free list is empty, return NULL
    if (implicit_list_head == 0)
    {
        return 0;
    }

    // Traverse the implicit free list
    while (block != 0)
    {
        // Check if the current block is unallocated and large enough
        if (!block->allocated && block->size >= size)
        {
            if (strategy == 'F') // First Fit strategy
            {
                // Return the first unallocated block that's large enough
                return block;
            }
            else if (strategy == 'B') // Best Fit strategy
            {
                // Update best_block if this is the first fit or a better fit than the previous best
                if (best_block == 0 || (block->size - size) < (best_block->size - size))
                {
                    best_block = block;
                }
            }
        }
        // Move to the next block in the list
        block = block->next;
    }

    // Return the best fit block (or NULL if no suitable block was found)
    return best_block;
}


// Function to initialize the heap with the first memory block
void create_heap()
{
    // Allocate memory for the initial block structure
    MEMORY_BLOCK *initial_block = malloc(sizeof(MEMORY_BLOCK));

    // Initialize block properties
    initial_block->allocated = 0;           // Mark the block as free
    initial_block->size = 3992;             // Set the size of the initial block (in bytes)
    initial_block->next = 0;                // No next block initially
    initial_block->prev = 0;                // No previous block initially
    initial_block->payload_index = 2;       // Payload starts at index 2 (after header)

    // Set the head of the implicit list to this initial block
    implicit_list_head = initial_block;

    // If using explicit allocation method, set up the explicit free list
    if (allocation_method == 'E')
    {
        // Allocate memory for the explicit list node
        EXPLICIT_LIST *explicit_free_node = malloc(sizeof(EXPLICIT_LIST));

        // Initialize explicit list node properties
        explicit_free_node->block = initial_block;   // Point to the newly created block
        explicit_free_node->next = 0;        // No next node initially
        explicit_free_node->prev = 0;        // No previous node initially

        // Set the head of the explicit list to this node
        explicit_list_head = explicit_free_node;

        // Store block pointers in the heap array for quick access
        // Store at the header location (payload_index - 1)
        heap[explicit_free_node->block->payload_index - 1] = (uintptr_t) explicit_free_node->block;

        // Store at the footer location (assuming 1000-WORD_SIZE heap, so 998 is the last footer index)
        heap[998] = (uintptr_t) explicit_free_node->block;
    }
}



 // Function that is responsible for allocating memory blocks
void* myalloc(int size)
{
    // Pointer to the memory block to allocate
    MEMORY_BLOCK *block = NULL;  

    // Pointer to an explicit free list node, if applicable               
    EXPLICIT_LIST *explicit_free_node = NULL;   

    // Step 1: Find a suitable free block or expand the heap if necessary
    do {
        // Try to find a free block based on the allocation method
        if (allocation_method == 'I') 
        {
            block = find_free_block_implicit(size, allocation_strategy);
        } 

        else if (allocation_method == 'E') 
        {
            explicit_free_node = find_free_block_explicit(size, allocation_strategy);
            block = explicit_free_node ? explicit_free_node->block : NULL;
        }

        // If no suitable block is found, expand the heap
        if (!block && implicit_list_head) 
        {
            MEMORY_BLOCK *last_block = implicit_list_head;
            while (last_block->next) last_block = last_block->next;

            // Expand the heap by requesting more memory
            mysbrk(size);

            // Create a new block in the expanded space
            block = malloc(sizeof(MEMORY_BLOCK));
            block->size = size;
            block->allocated = 0;
            block->payload_index = heap_size / sizeof(WORD_SIZE) - size / sizeof(WORD_SIZE) - 2;
            block->next = NULL;
            block->prev = last_block;
            last_block->next = block;

            // If using the explicit free list, add the new block to the list
            if (allocation_method == 'E') 
            {
                explicit_free_node = malloc(sizeof(EXPLICIT_LIST));
                explicit_free_node->block = block;
                explicit_free_node->next = explicit_list_head;
                explicit_free_node->prev = NULL;
                explicit_list_head = explicit_free_node;
            }
            break;  // Break out of the loop once a block is found or created
        }
    } while (!block && implicit_list_head);

    // Step 2: Handle the case where the heap is empty (initial allocation)
    if (!block) 
    {
        block = malloc(sizeof(MEMORY_BLOCK));
        block->size = size;
        block->allocated = 1;
        block->next = NULL;
        block->prev = NULL;
        block->payload_index = 2;  // Start the payload index at the beginning of the heap
        implicit_list_head = block;
    }

    // Step 3: Process the found or newly created block
    if (block->size - size > 2) 
    {
        // If the block is larger than needed, split it into two blocks
        separate_memory_block(size, block, explicit_free_node);
    } 
    
    else 
    {
        // Use the entire block
        block->size = size;
        block->allocated = 1;

        // Remove the block from the explicit free list if applicable
        if (allocation_method == 'E' && explicit_free_node) 
        {
            if (explicit_free_node->prev) 
                explicit_free_node->prev->next = explicit_free_node->next;
            if (explicit_free_node->next) 
                explicit_free_node->next->prev = explicit_free_node->prev;
            if (explicit_free_node == explicit_list_head) 
                explicit_list_head = explicit_free_node->next;
            free(explicit_free_node);
        }
    }

    // Step 4: Update the heap and set the block's header and footer
    int aligned_word_count = (block->size + sizeof(WORD_SIZE) - 1) / sizeof(WORD_SIZE);

    // Ensure the block's size is aligned to 8 bytes
    aligned_word_count = (aligned_word_count + (alignment / sizeof(WORD_SIZE) - 1)) & ~(alignment / sizeof(WORD_SIZE) - 1);

    // Calculate the header/footer value (aligned size + 2) * 4 + 1
    int header_footer_value = (aligned_word_count + 2) * 4 + 1;

    // Set the header just before the payload
    heap[block->payload_index - 1] = header_footer_value;

    // Set the footer just after the payload
    heap[block->payload_index + aligned_word_count] = header_footer_value;

    // Return a pointer to the payload of the allocated block
    return &block->payload_index;
}


// Function implements a custom memory deallocation 
void myfree(void *address_pointer)
{
    // Cast the void pointer to a MEMORY_BLOCK pointer
    MEMORY_BLOCK *block = address_pointer;

    // Mark the block as free
    block->allocated = 0;

    // Attempt to consolidate this newly freed block with adjacent free blocks
    consolidate_free_memory(block);

    // Function does not return a value
    return;
}


// Function attempts to expand the heap by a specified size.
void mysbrk(int size)
{
    
    // Check if the proposed heap expansion would exceed the maximum allowed size
    if (heap_size + size + alignment > MAX_HEAP_SIZE)
    {
        // Open output file in write mode
        FILE *output_file = fopen("output.txt", "w");
        
        // Write message to output file
        fprintf(output_file, "\n\nError: Heap expansion exceeded the maximum limit of 100,000 WORD_SIZEs.\n");
        
        // Close the output file
        fclose(output_file);
        
        // Terminate the program with an error code of 1
        // This indicates an unsuccessful exit due to heap size exceeding the maximum limit
        EXIT_FAILURE;
    }

    // If the heap expansion is allowed, increase the heap size
    // Add 8 bytes to account for the header (4 bytes) and footer (4 bytes) of the new block
    heap_size += size + alignment;

    // Function does not return a value
    return;
}


// Function for reading and processing input file
void read_and_process_input_file(FILE *input_file)
{
    // Buffer to hold each line from the input file
    char buffer[50];

    // While-loop that reads each line in the input file
    while (fgets(buffer, sizeof(buffer), input_file))
    {
        // Declare and initialize token address_pointer to hold memory operation
        char *token = strtok(buffer, ", ");

        // The first character of the token indicates the memory operation (a, r, f)
        char memory_operation = token[0];

        // Declare and initialize variables to store arguments for the memory operations.
        int argument_one = 0;
        int argument_two = 0;
        int argument_three = 0;

        // Declare pointer to hold the address of allocated or reallocated memory
        void *address_pointer;

        // If-else statements to parse the arguments depending on the selected memory operation
        if (memory_operation == 'a') // Allocate memory
        {
            token = strtok(NULL, ", ");
            if (token) argument_one = atoi(token);
            token = strtok(NULL, ", ");
            if (token) argument_two = atoi(token);
        }
        else if (memory_operation == 'r') // Reallocate memory
        {
            token = strtok(NULL, ", ");
            if (token) argument_one = atoi(token);
            token = strtok(NULL, ", ");
            if (token) argument_two = atoi(token);
            token = strtok(NULL, ", ");
            if (token) argument_three = atoi(token);
        }
        else if (memory_operation == 'f') // Free memory
        {
            token = strtok(NULL, ", ");
            if (token) argument_one = atoi(token);
        }

        // Switch-statement to perform the memory operation based on action
        switch (memory_operation)
        {
            case 'a': // Allocate memory

                // Call function sending size of argument one and return pointer
                address_pointer = myalloc(argument_one);

                // Store the pointer in heap array at index of argument two
                allocation_tracking_heap[argument_two] = address_pointer;

                break;

            case 'r': // Reallocate memory

                // Get the pointer from allocation_tracking_heap array at index argument two
                address_pointer = allocation_tracking_heap[argument_two];

                // If statement to check if argument one is equal to 0
                if (argument_one == 0)
                {
                    // Call function to free the memory
                    myfree(address_pointer);
                }
                else
                {
                    // Call function to reallocate memory to the new size of argument one
                    address_pointer = myrealloc(argument_one, address_pointer);

                    // Store the new pointer in allocation_tracking_heap array at index argument three
                    allocation_tracking_heap[argument_three] = address_pointer;
                }

                break;

            case 'f': // Free memory

                // Get the pointer from allocation_tracking_heap array at index argument one
                address_pointer = allocation_tracking_heap[argument_one];

                // If statement to check if address_pointer is not null
                if (address_pointer)
                {
                    // Call function to free memory
                    myfree(address_pointer);
                }

                break;
        }
    }
}


// Function to separate a memory block into an allocated block and a free block.
void separate_memory_block(size_t size, MEMORY_BLOCK *block, EXPLICIT_LIST *explicit_free_node)
{
    // Define the word size
    const size_t word_size = sizeof(WORD_SIZE);

    // Calculate the left footer position in the heap
    unsigned int left_footer = block->payload_index + (size + word_size - 1) / word_size;

    // Calculate the next header position (ensuring it's odd)
    unsigned int next_header = left_footer + 1;

    // Ensure next_header is odd by setting the least significant bit
    next_header |= 1;  

    // Calculate the next footer position in the heap
    unsigned int next_footer = block->payload_index + (block->size + word_size - 1) / word_size;

    // Ensure next_footer does not exceed heap size
    next_footer = (next_footer > heap_size / word_size) ? heap_size / word_size : next_footer;  

    // Calculate the size of the next block
    unsigned int next_size = (next_footer - next_header) * word_size;

     // If the next block size is too small, exit the function
    if (next_size < 2 * word_size) 
    {
        return; 
    }

    // Allocate memory for the new free block
    MEMORY_BLOCK *next_block = malloc(sizeof(MEMORY_BLOCK));
    
    // Set the size of the new block
    next_block->size = next_size; 

    // Mark the new block as free
    next_block->allocated = 0;  

    // Set the payload index of the new block
    next_block->payload_index = next_header + 1;  

    // Update the original block
    // Mark the original block as allocated
    block->allocated = 1;  

    // Update the size of the original block
    block->size = size;  

    // Update the linked list pointers
    // Set the previous pointer of the new block
    next_block->prev = block;  

    // Set the next pointer of the new block
    next_block->next = block->next; 

    // If statement to update the previous pointer of the next block if it exists
    if (block->next != 0) 
    {
        block->next->prev = next_block;  
    }

    // Update the next pointer of the original block
    block->next = next_block;  

    // Update the explicit free list if the allocation method is 'E'
    if (allocation_method == 'E') 
    {
        // Set the new block in the explicit free list
        explicit_free_node->block = next_block;  
    }

    // Calculate the size in words for heap update
    int size_in_words = (next_block->size + word_size - 1) / word_size;

    // Update the heap
    int block_header_value = (size_in_words - 1) * 4;

    // Update the header in the heap
    heap[next_block->payload_index - 1] = block_header_value;

    // Update the footer in the heap
    heap[next_block->payload_index + size_in_words - 3] = block_header_value;
}


// Function for writing output file
void write_output_file()
{
    // Open output file in write mode
    FILE *output_file = fopen("output.txt", "w");
    
    // For-loop to iterate through allocation_tracking_heap array 
    for (int i = 0; i < heap_size/sizeof(WORD_SIZE); i++)
    {
        // Write the index and the corresponding heap value to the output file
        fprintf(output_file, "%d, 0x%08X\n", i, heap[i]);
    }

    // Close file
    fclose(output_file);

    return;
}