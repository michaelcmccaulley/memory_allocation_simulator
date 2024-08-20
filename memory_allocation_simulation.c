/*
Date: 08/08/2024
Class: CS4541 Computer Systems
Assignment: Memory Allocation Simulator
File Main program (memory_allocation_simulation.c)
Author(s): Michael McCaulley
*/

// Include necessary libraries
#include <stdio.h>
#include <stdlib.h>
#include "functions.h"


// Main function
int main(int argc, char *argv[])
{
    // Declare local variables
    char file_name[100];
    char continue_choice;
    int run_counter = 0;

    // Print message to screen as standard output
    printf("\nWELCOME TO MEMORY ALLOCATION SIMULATOR!\n");

    // Do-while loop to run program as long as user wants
    do 
    {
        // Increment counter
        run_counter += 1;
        
        // Print message to screen as standard output to get user input
        printf("\n\nEnter the input file name: ");
        scanf("%s", file_name);

        // Print message to screen as standard output to get user input
        printf("\n\nEnter allocation method by entering a \"I\" for implicit or \"E\" for explicit: ");
        scanf(" %c", &allocation_method); 

        // Print message to screen as standard output to get user input
        printf("\n\nEnter allocation strategy by entering a \"F\" for first-fit or \"B\" for best-fit: ");
        scanf(" %c", &allocation_strategy);

        // Call function to initialize heap
        create_heap();

        // Open the input file in read mode
        FILE *input_file = fopen(file_name, "r");

        // Call function to read and process input file
        read_and_process_input_file(input_file);

        // Close file
        fclose(input_file);

        // Call function to write output file
        write_output_file();

        // Check if this program has been ran once already
        if (run_counter > 1)
        {
            printf("\n\nBefore proceeding to the next question, please review the \"output.txt\" file ");
            printf("to see desired outcome from iteration %d of the program.\n\n", run_counter);
        }
        else
        {
            printf("\n\nPlease review the \"output.txt\" file to see desired outcome.\n\n");
        }

        // Print message to screen as standard output & get user response
        printf("\nDo you want to run the program again? (Y/N): ");
        scanf(" %c", &continue_choice);

    } while (continue_choice == 'Y' || continue_choice == 'y');

    // Print message to screen as standard output
    printf("\n\nEXITING PROGRAM!\n\n");

    // Exit program successfully
    return EXIT_SUCCESS;
}
