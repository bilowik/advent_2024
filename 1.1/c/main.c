#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct List {
	int len;
	int *values;
	int alloc_size;
} List;


int list_append(List *self, int value) {
	if(self->len == self->alloc_size) {
		self->alloc_size *= 2;
		self->values = realloc(self->values, sizeof(int) * self->alloc_size);
	}
	self->values[self->len++] = value;
	return 0;
}

int list_clear(List *self) {
	self->len = 0;
}

int list_init(List *self, int initial_size) {
	self->alloc_size = initial_size;
	self->values = malloc(sizeof(int) * self->alloc_size);
	self->len = 0;
}

/// Destroys the given list, unallocating memory for the internal list.
/// self is still owned by the calling function and is responsible for that memory.
/// Usage after list_destroy is called is undefined.
int list_destroy(List *self) {
	free(self->values);
}

// Returns a malloc'd list owned by the caller of this function.
// The given list wlil have each entry correspond to the given column.
// line is assumed to be a null-terminated string.
// list is assumed to be empty but initialized.
List *parse_line(char *line, List *list) {
	char buffer[256];
	char curr_char = 0;
	int buffer_count = 0;
	int state = 0;
	int curr_int = 0;

	int line_len = strlen(line);

	for(int i = 0; i < line_len; i++) {
		curr_char = line[i];
		switch(curr_char) {
			case '\n':
				// The final char in the line
			case ' ':
				if(state == 0) {
					// We've gone from characters to whitespace.
					// Save the current buffer.
					state = 1;
					buffer[buffer_count] = 0; // null-term for atoi
					curr_int = atoi(buffer);
					list_append(list, curr_int);
					buffer_count = 0;
				}
				// else, we just break until we get something that isn't whitespace.
				break;
			default:
				// This is any other character.
				state = 0; // if we aren't already
				buffer[buffer_count++] = curr_char;
		}
	}
	return list;
}

// Parses the given file, updating lists_out with a pointer to an array
// of lists, and lists_count_out with the length of that array.
int parse_file(FILE *f, List **lists_out, int *list_count_out) {
	char buffer[1024];
	int num_lists = 1;
	int lists_size = 1;
	int lists_size_diff = 0;
	int lists_len_diff = 0;

	List curr_list;
	List *lists = malloc(sizeof(List) * lists_size);
	for(int i = 0; i < lists_size; i++) {
		list_init(lists + i, 64);
	}
	list_init(&curr_list, 64);
	

	while(fgets(buffer, sizeof(buffer), f) != NULL) {
		parse_line(buffer, &curr_list);

		lists_size_diff = curr_list.len - lists_size;

		if(curr_list.len > lists_size) {
			// We need to reallocate
			lists_size *= 2;
			lists = realloc(lists, lists_size * sizeof(List));
		}
		while(num_lists < curr_list.len) {
			// Increment num_lists and init additional
			// lists.
			list_init(lists + (num_lists++), 64);
		}

		for(int i = 0; i < curr_list.len; i++) {
			list_append(lists + i, curr_list.values[i]);
		}
		list_clear(&curr_list);
	}



	*lists_out = lists;
	*list_count_out = num_lists;
	list_destroy(&curr_list);	
	return 0;
}

int int_compare_func(const void *lhs, const void *rhs) {
	return *(int *) lhs - *(int *) rhs;
}

int main(int argc, char **argv) {
	if(argc < 2) {
		printf("Missing argument. Usage: main <FILE_PATH>");
		exit(-1);
	}
	FILE *f = fopen(argv[1], "r");
	if(f == NULL) {
		// Fialed to open file
		perror("Failed to open file");
		return -1;
	}

	List *lists;
	int num_lists = 0;

	int result = parse_file(f, &lists, &num_lists);
	
	fclose(f);

	for(int i = 0; i < num_lists; i++) {
		qsort(lists[i].values, lists[i].len, sizeof(int), &int_compare_func);
	}

	if(num_lists != 2) {
		printf("Got more lists than expected. Expected 2, got %d\n", num_lists);
		return -1;
	}

	if(lists[0].len != lists[1].len) {
		printf("Length of each list is not equal! List 1: %d; List 2: %d", lists[0].len, lists[1].len);
		return -2;
	}
	
	int distance = 0;
	for(int i = 0; i < lists[0].len; i++) {
		distance += abs(lists[0].values[i] - lists[1].values[i]);
	}

	printf("%d\n", distance);

	// Now calculate the similarity score.
	int similarity_score = 0;
	
	int prev_value = lists[0].values[0] - 1;
	int rhs_total = 0;
	int rhs_idx = 0;
	int curr_value = 0; 
	
	for(int i = 0; (i < lists[0].len) && (rhs_idx < lists[1].len); i++) {
		curr_value = lists[0].values[i];
		if(curr_value != prev_value) {
			// New value, get the number of times this value occurs in 
			// the next list.
			rhs_total = 0;
			while(curr_value >= lists[1].values[rhs_idx]) {
				// Iterate over values until we reach a value > 
				// the currrent lhs value. If values are equal
				// increment the rhs_total.
				if(curr_value == lists[1].values[rhs_idx]) {
					rhs_total++;
				}
				rhs_idx++;
				if(rhs_idx >= lists[1].len) {
					break;	
				}

			}

		}
		// In the case where the above condiitonal doesn't run, 
		// it means the currnet value is the same as the last, so 
		// the total we want to add will be the same.
		similarity_score += rhs_total * curr_value;
		prev_value = curr_value;
	}

	printf("%d\n", similarity_score);

	for(int i = 0; i < num_lists; i++) {
		list_destroy(lists + i);
	}
	free(lists);
}
