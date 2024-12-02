#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct List {
	int len;
	int *values;
	int alloc_size;
} List;


int list_append(List *self, int value) {
	self->values[self->len++] = value;
	if(self->len == self->alloc_size) {
		self->alloc_size *= 2;
		self->values = realloc(self->values, sizeof(int) * self->alloc_size);
	}
	return 0;
}

int list_init(List *self, int initial_size) {
	self->alloc_size = initial_size;
	self->values = malloc(sizeof(int) * self->alloc_size);
	self->len = 0;
}

// Parses lists_in and places pointers to the two parsed lists 
// - lists_out will be updated with a pointer to an array of lists. Caller owns the memory and needs to free it.
// - lists_count will be updated with the number of lists.
// If an error occurs, the output pointers will be unchanged.
int parse_lists(char *lists_in, int lists_in_len, List **lists_out, int *list_count_out) {
	char buffer[256];
	int column_counter = 0;
	int row_counter = 0;
	char current_char = 0;
	int buffer_count = 0;
	int state = 0;
	int num_lists = 1;
	int curr_int;

	List *lists = malloc(sizeof(List) * num_lists);

	for(int i = 0; i < num_lists; i++) {
		list_init(&lists[i], 32);
	}
	
	// State:
	// 	0: Last charactor was a character
	// 	1: Last character was whitespace
	// 	2: Last character was newline
	//
	// In state 1, all input is ignored until new characters are found.
	for(int i = 0; i < lists_in_len; i++) {
		current_char = lists_in[i];
		switch(current_char) {
			case ' ':
				if(state == 0) {
					// We've gone from characters to whitespace.
					// Save the current buffer.
					state = 1;
					buffer[buffer_count] = 0; // null-term for atoi
					curr_int = atoi(buffer);
					list_append(lists + column_counter, curr_int);
					buffer_count = 0;
					column_counter++;
				}
				break;
			case '\n':
			case '\r':
				// We've reached the end of the line, we 
				// finish up the final column.
				if(state < 2) { // state 0 or 1.
					// We were not in a whitespcae or newline state
					// so save what we have currently.
					state = 2;
					if(buffer_count > 0) {
						// There's actual stuff in the buffer.
						buffer[buffer_count] = 0;
						curr_int = atoi(buffer);
						list_append(lists + column_counter, curr_int);
					}
					column_counter = 0;
				}
				break;
			default:
				if((state != 0) && (column_counter >= num_lists)) {
					// We need to reallocate.
					// Have to do it here bc we only
					// want to realloc if we hit whitespace
					// followed by non-newline chars.
					lists = realloc(lists, sizeof(int *) * ++num_lists);
				}
				// This is any other character.
				state = 0; // if we aren't already
				buffer[buffer_count++] = current_char;
		}
	}

	*lists_out = lists;
	*list_count_out = num_lists;
	return 0;
}

int main(int argc, char **argv) {
	if(argc < 2) {
		printf("Missing argument. Usage: main <FILE_PATH>");
		exit(-1);
	}

	List *lists; 
	char *path = argv[0];

	FILE *f = fopen(path, "r");
	fseek(f, 0, SEEK_END);


	fclose(f);


}
