#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>


typedef struct List {
	char *data;
	int len;
	int alloc_size;
} List;

int list_init(List *self, int initial_alloc) {
	self->data = malloc(sizeof(char) * initial_alloc);
	self->len = 0;
	self->alloc_size = initial_alloc;
	return 0;
}

int list_append(List *self, char value) {
	if(self->len == self->alloc_size) {
		self->alloc_size *= 2;
		self->data = realloc(self->data, sizeof(char) * self->alloc_size);
	}
	self->data[self->len++] = value;
	return 0;
}

char list_get(List *self, int idx) {
	if(idx >= self->len) {
		return (char) -1;
	}
	return self->data[idx];
}

int list_set(List *self, int idx, char value) {
	if(idx >= self->len) {
		return -1;
	}
	self->data[idx] = value;
	return 0;
}

enum Direction {
	UP,
	RIGHT,
	DOWN,
	LEFT
};

const int DIRECTION_TO_XY_IDX[4][2] = {
	{ 0, -1 },
	{ 1, 0 },
	{ 0, 1 },
	{ -1, 0 },
};

int main(int argc, char **argv) {
	if(argc < 2) {
		printf("Missing <PATH> argument\n");
		return -1;
	}
	long map_width;
	long map_height;
	long file_size;
	long line_two_offset;
	char buffer[8192];
	char *map;
	FILE *f;
	int line_count;
	int guard_pos[2];
	int curr_direction = UP;
	char *curr_char;
	int offset[2];
	int unique_pos_count = 1; // 1 for first pos.

	// States:
	// 0: Looking for guard
	// 1: Moving guard.
	// 2: Reached edge.
	// 3: Error
	int state = 0;
	
	
	// Open the file and get the size of the map.
	f = fopen(argv[1], "r");
	fgets(buffer, sizeof(buffer), f);
	line_two_offset = ftell(f);
	map_width = strlen(buffer) - 1; // -1 for newline.

	if(fseek(f, 0L, SEEK_END)) {
		perror("An error occured when seeking file end for file size calc: ");
		return -1;
	}
	file_size = ftell(f);
	map_height = file_size / map_width;
	
	if(map_width > INT_MAX) {
		printf("Each line exceeds integer maximum, cannot continue. map_width=%d\n", map_width);
		return -1;
	}
	if(map_height > INT_MAX) {
		printf("Each line exceeds integer maximum, cannot continue. map_height=%d\n", map_height);
		return -1;
	}

	map = malloc(sizeof(char) * map_height * map_width);
	
	
	// Reset back to the 2nd line.
	fseek(f, line_two_offset, SEEK_SET);
	memcpy(map, buffer, map_width);
	printf("%s\n", map);
	line_count = 1;

	while(state < 2) {
		switch(state) {
			case 0:
				// We are looking for the guard.
				if(fgets(buffer, sizeof(buffer), f) == NULL) {
					perror("We've hit the end of the file, or an error occured, did not find guard.\n");
					return -1;
				}
				printf("%s\n", buffer);
				memcpy(map + map_width * line_count, buffer, map_width);
				for(int i = 0; i < map_width; i++) {
					curr_char = map + (map_width * line_count) + i;
					if(*curr_char == '^') {
						// We've found the guard
						state = 1;
						guard_pos[0] = i;
						guard_pos[1] = line_count;
						*curr_char = 'X';
					}
				}

				line_count++;
				break;
			case 1:
				// Moving the guard.
				memcpy(&offset[0], &DIRECTION_TO_XY_IDX[curr_direction], sizeof(int) * 2);
				guard_pos[0] += offset[0];
				guard_pos[1] += offset[1];
				if(
						(guard_pos[0] < 0) 			||
						(guard_pos[0] >= map_width)	||
						(guard_pos[1] < 0) 			||
						(guard_pos[1] >= map_height)
				) {
					// We've hit the end of the map.
					state = 2;
				}
				else {
					// We keep moving.
					if(guard_pos[0] > line_count) {
						//fgets(map + (map_width * line_count++), map_width, f);
						fgets(buffer, sizeof(buffer), f);
						memcpy(map + map_width * line_count++, buffer, map_width);
					}
					curr_char = map + guard_pos[1] * map_width + guard_pos[0];
					printf("pos=(%d,%d) curr_char=%c curr_direction=%d\n", guard_pos[0]+1, guard_pos[1]+1, *curr_char, curr_direction);
					switch(*curr_char) {
						case '#':
							// We cannot move into this position, backtrack 
							// and rotate.
							guard_pos[0] -= offset[0];
							guard_pos[1] -= offset[1];
							curr_direction = (curr_direction + 1) % 4;
							break;
						case '.':
							// New position, increment counter and 
							// mark as touched.
							unique_pos_count++;
							*curr_char = 'x';
							break;
					}
				}
				break;

		}
	}

	

	
	
	



	

	

	
	printf("%d\n", unique_pos_count);	
	fclose(f);
	free(map);

	return 0;

}
