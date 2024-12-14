#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*typedef struct StateTracker {
	int state;
	int x_direction;
	int y_direction;
} Statetracker;*/


typedef struct Cell {
	// Values:
	// UNSET = 0;
	// X = 1, -1
	// M = 2, -2
	// A = 3, -3
	// S = 4, -4
	//
	// Negative values denote the word going in reverse.
	int value;
	
	// The index of the state determines 
	// the direction, whereas the value
	// determines what the cell needs to be
	// in order to continue building.
	//
	// State value of 0 means there's no existing word being bulit
	// from that direction.
	// The value in the 0th index is unused.
	int states[5]; 
} Cell;

void cell_init(Cell *self, int value) {
	self->value = value;
	memset(&(self->states), 0, sizeof(self->states));
}

typedef struct Row {
	Cell *cells;
	int alloc_size;
	int len;
} Row;

void row_init(Row *self, int initial_size) {
	self->alloc_size = initial_size;
	self->cells = malloc(sizeof(Cell) * initial_size);
	memset(self->cells, 0, sizeof(Cell) * initial_size);
	self->len = 0;
}

void row_destroy(Row *self) {
	free(self->cells);
	self->alloc_size = 0;
	self->len = 0; // TODO: Do we really need to track len here?
}

// Doubles the allocated size
void row_expand(Row *self) {
	if(self->alloc_size == 0) {
		printf("Invalid allocation size found for Row object, exiting.\n");
		exit(-1);
	}
	self->alloc_size *= 2;
	self->cells = realloc(self->cells, sizeof(Cell) * self->alloc_size);
	memset(self->cells + (self->alloc_size / 2), 0, (self->alloc_size / 2) * sizeof(Cell));
}

// Gets a ptr to the memory location where the Cell would
// be stored if it exists.
// This will expand the row size if necessary.
// *return_value will be NULL if the value doesn't exist.
Cell *row_get_ref(Row *self, int idx) {
	while(idx >= self->alloc_size) row_expand(self);
	self->len = (idx > self->len) ? idx : self->len; 
	return &(self->cells[idx]);
}

// -1 are essentially invalid directions since 
// we only ever need to go down or to the right.
const int DIRECTION_STATE_INDEX_MAP[2][3] = {
	{ 0, 0, 1 },
	{ 4, 3, 2 },
};

const int STATE_INDEX_DIRECTION_MAP[5][2] = {
	{-0, 0}, // Unused.
	{ 1, 0 },
	{ 1, 1 },
	{ 0, 1 },
	{ -1, 1 },
};

int CHAR_TO_VALUE_MAP[23];

int sign_num(int value) {
	if(value < 0) return -1;
	else if(value > 0) return 1;
	return 0;
}

int main(int argc, char *argv[]) {
	int row_count = 0; // Current row index
	int row_alloc = 4; // Current allocation size of rows
	int curr_value = 0; // Current value determined by curr_char
	int curr_state = 0; // Current state 
	Cell *curr_cell_ptr = NULL; // Current cell
	Cell *next_cell_ptr =  NULL; // Neighboring cell targeted for a modification 
	int curr_char = 0; // Current character read in from the input
	int word_count = 0; // The total number of instances of "xmas" found
	int cross_mas_count = 0; // The total number of occurences of mas crossing
	char curr_line[1024]; // The buffer for the current line read in from the input.
	int curr_line_len = 0; // The length of the line read in from the input 
	FILE *f; // The input file to read from
	int curr_direction[2] = { 0, 0 }; // The current direction used to target a neighboring cell.
	int state_assignment = 0; // Value to assign to a cell neighboring an X or S.
	int state_idx = 0; // Current state index, corresponding to a direction (see mapping).
	Row *row_ptr; // Points to a target row for modification, may or may not be current row.
	Row *rows = malloc(sizeof(Row) * row_alloc); // Collection of all rows.

	for(int i = 0; i < row_alloc; i++) {
		row_init(&rows[i], 128);
	}
	
	// - 65 so the buffer only needs to size 23 instead of 88.
	CHAR_TO_VALUE_MAP[(int) 'X' - 65] = 1;
	CHAR_TO_VALUE_MAP[(int) 'M' - 65] = 2;
	CHAR_TO_VALUE_MAP[(int) 'A' - 65] = 3;
	CHAR_TO_VALUE_MAP[(int) 'S' - 65] = 4;

	if(argc < 2) {
		return -2;
	}

	f = fopen(argv[1], "r");
	
	while(fgets(curr_line, sizeof(curr_line), f) != NULL) {
		curr_line_len = strlen(curr_line);
		while((row_count + 1) >= row_alloc) {
			// realloc
			// + 1 to cover the following row which will be 
			// accessed in the current iteration.
			row_alloc *= 2;
			rows = realloc(rows, sizeof(Row) * row_alloc);
			for(int i = row_alloc / 2; i < row_alloc; i++) {
				row_init(&rows[i], 128);
			}
		}
		
		for(int i = 0; i < curr_line_len; i++) {
			curr_char = curr_line[i];
			curr_value = CHAR_TO_VALUE_MAP[(int) curr_char - 65];
			curr_cell_ptr = row_get_ref(&rows[row_count], i);
			if(curr_cell_ptr->value == 0) {
				curr_cell_ptr->value = curr_value;
			}
			
			// Handle the caes for X and S since they affect all nearby cells
			// regardless of state.
			if(curr_char == 'X') {
				state_assignment = 2;
			}
			else if(curr_char == 'S') {
				state_assignment = -3;
			}
			if(state_assignment) {
				for(state_idx = 1; state_idx < 5; state_idx++) {
					memcpy(&curr_direction, STATE_INDEX_DIRECTION_MAP + state_idx, sizeof(int) * 2);
					if((curr_direction[0] + i) < 0) {
						// Trying to reach beyond the left edge.
						continue;
					}
					next_cell_ptr = row_get_ref(&rows[row_count + curr_direction[1]], curr_direction[0] + i);
					next_cell_ptr->states[state_idx] = state_assignment;
				}
			}
			state_assignment = 0;
			for(state_idx = 1; state_idx < 5; state_idx++) {
				curr_state = curr_cell_ptr->states[state_idx];
				switch(abs(curr_state)) {
					case 1:
						if(curr_char == 'X') {
							// We've finished a word
							word_count++;
						}
						break;
					case 4: 
						if(curr_char == 'S') {
							// We've finished a word
							word_count++;
						}
						break;
					default:
						if(curr_value == abs(curr_state)) {
							// We are following a potential word
							memcpy(&curr_direction, STATE_INDEX_DIRECTION_MAP + state_idx, sizeof(int) * 2);
							if((i + curr_direction[0]) < 0) {
								// Goes off left edge, do nothing
								break;
							}
							row_ptr = &rows[row_count + curr_direction[1]];
							next_cell_ptr = row_get_ref(row_ptr, i + curr_direction[0]);
							// +1 to the second value to offset into the correct spot.
							int d = DIRECTION_STATE_INDEX_MAP[curr_direction[1]][curr_direction[0] + 1];
							next_cell_ptr->states[DIRECTION_STATE_INDEX_MAP[curr_direction[1]][curr_direction[0] + 1]] = curr_state + 1;


						}
						break;
				}
			}
		}
		row_count++;
	}


	printf("%d\n", word_count);	
	for(int i = 0; i < row_alloc; i++) {
		row_destroy(&rows[i]);
	}
	free(rows);
	fclose(f);	
	return 0;
}
