#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

typedef struct Node Node;

struct Node {
	Node *next;
	int value;
};

typedef struct LinkedList {
	Node *head;
} LinkedList;

int node_init(Node *self, int value) {
	self->value = value;
	self->next = NULL;
	return 0;
}

int linked_list_append(LinkedList *self, int value) {
	Node **node_ptr = &self->head;
	while(*node_ptr != NULL) {
		node_ptr = &(*node_ptr)->next;
	}
	*node_ptr = malloc(sizeof(Node));
	node_init(*node_ptr, value);

	return 0;
}


int linked_list_init(LinkedList *self) {
	self->head = NULL;
	return 0;
}

// Returns the Node with the given idx, or NULL if idx is out of bounds.
// The linked list retains ownership.
Node *linked_list_get(LinkedList *self, int idx) {
	Node *node_ptr = self->head;
	for(int i = 1; i < idx; i++) {
		node_ptr = node_ptr->next;

		if(node_ptr == NULL) {
			// Stop iteration, idx was out of bounds.
			break;
		}
	}
	return node_ptr;
}

Node *linked_list_last(LinkedList *self) {
	Node *node_ptr = self->head;

	if(node_ptr == NULL) {
		// head was null
		return NULL;
	}

	while(node_ptr->next != NULL) {
		node_ptr = node_ptr->next;
	}

	return node_ptr;
}

int linked_list_empty(LinkedList *self) {
	Node *curr_node = self->head;
	Node *next_node;

	while(curr_node != NULL) {
		next_node = curr_node->next;
		curr_node->next = NULL;
		free(curr_node);
		curr_node = next_node;
	}
	self->head = NULL;
	return 0;
}

// Parses the line, appending them into the given LinkedList.
// Assumes output is initialized.
int parse_line(char *line, LinkedList *output) {
	int curr_char = 0;
	int len = strlen(line);
	int state = 0;
	int curr_value = 0;
	char buffer[256];
	int buffer_idx = 0;
	//memset(buffer, 0, 256); Not really needed since we null-term anyway.
	
	// state 0: whitespace
	// state 1: Within number
	for(int i = 0; i < len; i++) {
		curr_char = line[i];
		
		switch(curr_char) {
			case ' ':
			case '\n':
				// Whitespace
				if(state == 1) {
					// We've reached the end of a number
					buffer[buffer_idx] = 0; // null term
					curr_value = atoi(buffer);
					linked_list_append(output, curr_value);
					buffer_idx = 0;
					state = 0;
				}
				break;
			default:
				// Any other char
				state = 1;
				buffer[buffer_idx++] = curr_char;
				break;
		}
	}

	return 0;
}

int sign(int value) {
	if(value > 0) return 1;
	if(value < 0) return -1;
	return 0;
}

int main(int argc, char *argv[]) {
	FILE *input;
	char buffer[1024];
	int prev_value;
	int diff; // <0 = decreasing, >0 = increase, 0 = unknown yet
	int prev_change_direction;
	int is_valid = 0;
	int valid_count_2;
	int valid_count = 0;
	int curr_diff = 0;
	Node *curr_node;
	LinkedList curr_line;
	linked_list_init(&curr_line);

	if(argc < 2) {
		perror("Missing argumnet. Usage: main <FILE_PATH>");
	}
	if((input = fopen(argv[1], "r")) == NULL) {
		perror("Failed to read in file");
		exit(-1);
	}

	linked_list_init(&curr_line);
	// We do not need to init curr_line.

	while(fgets(buffer, sizeof(buffer), input) != NULL) {
		is_valid = 1;
		parse_line(buffer, &curr_line);


		curr_node = curr_line.head;

		// Need to initialize this here .
		prev_change_direction = sign(curr_node->value - curr_node->next->value);
		do {

			diff = curr_node->value - curr_node->next->value;
			is_valid = ((abs(diff) <= 3) && diff != 0) && (sign(diff) == prev_change_direction);

			prev_change_direction = sign(diff);
			curr_node = curr_node->next;

		} while((curr_node->next != NULL) && is_valid);

		if(is_valid) {
			valid_count++;
		}
		linked_list_empty(&curr_line);
	}
	fclose(input);
	printf("%d\n", valid_count);
	printf("%d\n", valid_count_2);
	return 0;
}

