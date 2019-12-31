
#include "mymalloc.h"

/*
 * Creates a new, inactive node.
 * @param curr_mem_index Current index in memory array
 * @param size Amount of space the node will contain
 */
void create_node(short curr_mem_index, unsigned short size)
{
	node_t *new_node = (node_t*) &myblock[curr_mem_index];

	new_node->size = size;
	new_node->active = 0;
}

/*
 * Sets up memory if mymalloc() has not been called previously.
 * Creates a single, inactive node encompassing all of memory.
 * If mymalloc() has been called in the past, nothing will be done.
 */
void initialize_malloc()
{
	if (myblock[0] == '\0')	// memory is initialized to \0 prior to the creation of the first node
	{
		create_node(0, sizeof(myblock) - sizeof(node_t));
	}
}

/*
 * Determines whether or not the space requested by the user is a valid request.
 * The user can request a minimum of 1 byte.
 * The maximal request is determined by subtracting the size of one metadata node from the size of the memory array.
 * @param request_size Amount of bytes requested by the user
 * @return True if the request is valid, and false otherwise
 */
bool validate_request(size_t request_size, int line, char* filename)
{
	if (request_size < 1)	// checking if request is too small
	{
		printf("Error at line %d in file %s: Request is too small! Minimum request: %d; your request: %d\n", line, filename, 1, request_size);
		return false;
	}
	else if (request_size > sizeof(myblock) - sizeof(node_t)) 	// check if the request is too big
	{

		printf("Error at line %d in file %s: Request is too large! Maximum request: %d; your request: %d\n", line, filename, sizeof(myblock) - sizeof(node_t), request_size);
		return false;
	}

	return true;
}

/*
 * Calculates and returns the index of the next metadata node.
 * @param curr_mem_index Index in the memory array from which to begin counting
 * @return Index of the next metadata node
 */
short get_next_index(short curr_mem_index)
{
	node_t *curr_node = (node_t*) &myblock[curr_mem_index];

	short next_mem_index = curr_mem_index + curr_node->size + sizeof(node_t);	// calculate location of next node

	if (next_mem_index >= sizeof(myblock) - sizeof(node_t)) // out of bounds!
	{
		return -1;
	}

	return next_mem_index;
}

/*
 * Compares the current inactive node with the amount of requested space and determines an action.
 * @param curr_mem_index Index of the current node
 * @param request_size Amount of space requested
 * @return The first action that should be taken on the current node
 */
action_type compare(short curr_mem_index, short request_size)
{
	node_t *curr_node = (node_t*) &myblock[curr_mem_index];
	action_type result = ACTION_SKIP;

	if (curr_node->size < request_size)	// node is too small
	{
		result = ACTION_SKIP;
	}
	else if (curr_node->size <= request_size && request_size <= curr_node->size + sizeof(node_t)	// node can accomodate requested data, but is too small to split
			|| curr_mem_index + sizeof(node_t) + request_size >= sizeof(myblock) - sizeof(node_t))	// node can accomodate data, but there is not enough space in memory left to split
	{
		result = ACTION_FILL;
	}
	else	// Node can accomodate data and has enough leftover space to birth a new, unused node
	{
		result = ACTION_SPLIT;
	}

	return result;
}

/*
 * Splits a large node into two.
 * The first node is created with a specified size.
 * The second node is created out of the remaining space from the original node.
 * @param curr_mem_index Index of the large node that will be split
 * @param request_size Desired size of first node
 */
void split_node(short curr_mem_index, short request_size)
{
	node_t *first_node = (node_t*) &myblock[curr_mem_index];

	short second_size = first_node->size - sizeof(node_t) - request_size;	// calculates size of second node from what will be left over after forming first node

	first_node->size = request_size;	// assigns first node its requested size
	short second_node_index = get_next_index(curr_mem_index);	// sets the second node to begin at what is now unused memory
	create_node(second_node_index, second_size);	// creates the second node, allowing you to navigate past it in the future

}

/*
 * Returns a void pointer to the beginning of the user data associated with a metadata node.
 * @param curr_mem_index Index of the metadata node
 * @return Void pointer to user data
 */
void *get_data_ptr (short curr_mem_index)
{
	void *data_ptr = (void*) &myblock[curr_mem_index + sizeof(node_t)];
	return data_ptr;
}


/*
 * Allocates space of requested size in "dynamic" memory.
 * @param request_size Amount of memory requested by user
 * @return Pointer to block of memory
 */
void *mymalloc(size_t request_size, int line, char* filename)
{
	initialize_malloc();
	if (!validate_request(request_size, line, filename))
	{
		return NULL;
	}

	short curr_mem_index = 0;
	node_t *curr_node;

	while (curr_mem_index > -1)
	{
		curr_node = (node_t*) &myblock[curr_mem_index];

		if (!curr_node->active)	// decide on an action for an inactive node
		{
			action_type action = compare(curr_mem_index, request_size);
			switch (action)
			{
				case ACTION_SPLIT:
					split_node(curr_mem_index, request_size);
				case ACTION_FILL:
					curr_node->active = 1;
					return get_data_ptr(curr_mem_index);
				case ACTION_SKIP:
					curr_mem_index = get_next_index(curr_mem_index);
			}
		}
		else // node is active
		{
			curr_mem_index = get_next_index(curr_mem_index);
		}
	} // end while

	printf("Error at line %d in file %s: Out of memory!\n", line, filename);
	return NULL;
} //end of mymalloc(size_t size)

/*
 * Determines whether or not a pointer lies within the memory array
 * @param *ptr Prospective pointer
 * @return True if pointer is within the array, false otherwise
 */
bool validate_ptr(void *ptr)
{
	return (void*) myblock <= ptr && ptr <= (void*) &myblock[sizeof(myblock) - 1];
}

/*
 * Merges two nodes.
 * The first node absorbs the second node, creating one large node.
 * @param first_index Index of the first node
 * @param second_index Index of the second node
 */
void merge_two_nodes(short first_index, short second_index)
{
	node_t *first_node = (node_t*) &myblock[first_index];
	node_t *second_node = (node_t*) &myblock[second_index];

	short combined_size = first_node->size + sizeof(node_t) + second_node->size;
	first_node->size = combined_size;	// overwrites second node
}

/*
 * Attempts to combine the current node with the node before and after it.
 * Nodes can only be combined if they are both inactive.
 * @param prev_mem_index Index of the previous node
 * @param curr_mem_index Index of the current node
 * @param next_mem_index Index of the next node
 */
void combine_nodes(short prev_mem_index, short curr_mem_index, short next_mem_index)
{
	node_t *adjacent_node;
	if (next_mem_index > -1)	// next node exists
	{
		adjacent_node = (node_t*) &myblock[next_mem_index];
		if (!adjacent_node->active)	// next node is also inactive, join them!
		{
			merge_two_nodes(curr_mem_index, next_mem_index);
		}
	}
	
	if (prev_mem_index > -1)	// previous node exists
	{
		adjacent_node = (node_t*) &myblock[prev_mem_index];
		if (!adjacent_node->active)	// previous node is inactive, join them!!
		{
			merge_two_nodes(prev_mem_index, curr_mem_index);
		}
	}
}

/*
 * Frees a block of dynamically allocated memory for future use.
 * Freed memory cannot be accessed.
 * @param *ptr Pointer to a previous allocation performed by mymalloc()
 */
void myfree(void *ptr, int LINE, char *FILE)
{

	if(ptr == NULL)
	{
		printf("Error at line %d in file %s: Cannot free a NULL pointer\n", LINE, FILE);
		return;
	}

	if (!validate_ptr(ptr))	// check that address is within our memory array
	{
		printf("Error at line %d in file %s: Argument is not an address within the heap\n", LINE, FILE);
		return;
	}

	node_t *target_node = (node_t*) (ptr - sizeof(node_t));	// we will attempt to free this node
	node_t *curr_node;	// use this node to search for target

	short prev_mem_index = -1;	// track preceding node 
	short curr_mem_index = 0;

	while (curr_mem_index > -1)
	{
		curr_node = (node_t*) &myblock[curr_mem_index];
		
		if (curr_node == target_node)	// Found node
		{
			if (!curr_node->active)	// node was already freed
			{
				printf("Error at line %d in file %s: Pointer was already freed!\n", LINE, FILE);
				return;
			}
			curr_node->active = 0;	// set inactive - do not need to clear out data
			combine_nodes(prev_mem_index, curr_mem_index, get_next_index(curr_mem_index)); // check adjacent nodes
			return;
		}

		prev_mem_index = curr_mem_index;
		curr_mem_index = get_next_index(curr_mem_index);
	}
	
	// not found? data was not a pointer
	printf("Error at line %d in file %s: Argument is not a pointer returned by malloc()\n", LINE, FILE);

	return;
} //end of myfree(void * freePtr)

/*
 * Prints out the nodes of our memory array.
 */
void print_memory()
{
	short curr_mem_index = 0;
	node_t *curr_node;

	while (curr_mem_index > -1)
	{
		curr_node = (node_t*) &myblock[curr_mem_index];

		printf("Size = %d, Active = %d ---> ", curr_node->size, curr_node->active);

		curr_mem_index = get_next_index(curr_mem_index);
	}

	printf("\n");
}
