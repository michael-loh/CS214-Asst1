/*
 * mymalloc.h
 *
 *  Created on: Sep 30, 2019
 *      Author: micha
 */

#ifndef MYMALLOC_H_
#define MYMALLOC_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define malloc(x) mymalloc(x, __LINE__, __FILE__)
#define free(x) myfree(x, __LINE__, __FILE__)

/*
 * This array will represent the heap.
 * All dynamic memory will be allocated on this array.
 * The elements are initialized to '\0'
 */
static char myblock[4096] = {'\0'};

/*
 * A node of metadata.
 * Each allocated entity will be associated with a node.
 */
typedef struct node_t {
	unsigned short size : 12;	// Represents space set aside for the user data. Also used for traversal.
	bool active : 1;	// True if the node is actively storing data and false otherwise.
} node_t;

/*
 * A type that represents an action to be taken on a node.
 * ACTION_FILL dictates that a node should be activated and its size kept the same.
 * ACTION_SPLIT dictates that a node is large enough to be split into two, in the interest of leaving more space for future data.
 * ACTION_SKIP indicates that a node is too small to fit the requested data, and mymalloc() should search for a larger node to fill or split.
 */
enum _action_type {ACTION_FILL, ACTION_SPLIT, ACTION_SKIP};
typedef enum _action_type action_type;

/*
 * Creates a new, inactive node.
 * @param curr_mem_index Current index in memory array
 * @param size Amount of space the node will contain
 */
void create_node(short curr_mem_index, unsigned short size);

/*
 * Sets up memory if mymalloc() has not been called previously.
 * Creates a single, inactive node encompassing all of memory.
 * If mymalloc() has been called in the past, nothing will be done.
 */
void initialize_malloc();

/*
 * Determines whether or not the space requested by the user is a valid request.
 * The user can request a minimum of 1 byte.
 * The maximal request is determined by subtracting the size of one metadata node from the size of the memory array.
 * @param request_size Amount of bytes requested by the user
 * @return True if the request is valid, and false otherwise
 */
bool validate_request(size_t request_size, int line, char* filename);

/*
 * Calculates and returns the index of the next metadata node.
 * @param curr_mem_index Index in the memory array from which to begin counting
 * @return Index of the next metadata node
 */
short get_next_index(short curr_mem_index);
	
/*
 * Compares the current inactive node with the amount of requested space and determines an action.
 * @param curr_mem_index Index of the current node
 * @param request_size Amount of space requested
 * @return The first action that should be taken on the current node
 */
action_type compare(short curr_mem_index, short request_size);

/*
 * Splits a large node into two.
 * The first node is created with a specified size.
 * The second node is created out of the remaining space from the original node.
 * @param curr_mem_index Index of the large node that will be split
 * @param request_size Desired size of first node
 */
void split_node(short curr_mem_index, short request_size);

/*
 * Returns a void pointer to the beginning of the user data associated with a metadata node.
 * @param curr_mem_index Index of the metadata node
 * @return Void pointer to user data
 */
void *get_data_ptr (short curr_mem_index);

/*
 * Allocates space of requested size in "dynamic" memory.
 * @param request_size Amount of memory requested by user
 * @return Pointer to block of memory
 */
void *mymalloc(size_t request_size, int line, char* filename);

/*
 * Determines whether or not a pointer lies within the memory array
 * @param *ptr Prospective pointer
 * @return True if pointer is within the array, false otherwise
 */
bool validate_ptr(void *ptr);

/*
 * Merges two nodes.
 * The first node absorbs the second node, creating one large node.
 * @param first_index Index of the first node
 * @param second_index Index of the second node
 */
void merge_two_nodes(short first_index, short second_index);

/*
 * Attempts to combine the current node with the node before and after it.
 * Nodes can only be combined if they are both inactive.
 * @param prev_mem_index Index of the previous node
 * @param curr_mem_index Index of the current node
 * @param next_mem_index Index of the next node
 */
void combine_nodes(short prev_mem_index, short curr_mem_index, short next_mem_index);

/*
 * Frees a block of dynamically allocated memory for future use.
 * Freed memory cannot be accessed.
 * @param *ptr Pointer to a previous allocation performed by mymalloc()
 */
void myfree(void *ptr, int LINE, char *FILE);

/*
 * Prints out the nodes of our memory array.
 */
void print_memory();

#endif /* MYMALLOC_H_ */




