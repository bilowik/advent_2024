#include <stdio.h>
#include <stdlib.h>

int is_numeral(char c) {
	return (c >= '0' && c <= '9');
}

int main(int argc, char *argv[]) {
	if(argc < 2) {
		printf("Missing argument: <PATH>\n");
		return -1;
	}
	
	FILE *f;
	char read_chunk[256]; // Holds read in data.
	int read_chunk_len = 0; // Current size of read-in data 
	int curr_char = 0; // Stores the current char from the read_chunk
	int read_chunk_idx = 0; // Tracks iteration in the read_chunk
	int curr_num = 0; // Stores the parsed number currently being read.
	int curr_product = 1; // Stores the curent output of the given mul op.
	int total_sum = 0; // The sum of only enabled multiply operations.
	int total_sum_with_disabled = 0; // The sum of ALL multiply operations
	int arg_count = 0; // Counts the number of args in the current mul op.	
	int mul_enabled = 1; // Tracks if mul operations should be ignored.

	// States: 
	//
	// 0: Last char was invalid, initial state
	// 1: Last char was 'm'
	// 2: Last char was 'u'
	// 3: Last char was 'l'
	// 4: Last char was '(' or ','
	// 5: Last char was [0-9]
	// 11: Last char was d
	// 12: Last char was o
	// 13: Last char was n
	// 14: Last char was '
	// 15: Last char was t
	// 16: Last char was ( following state 12.
	// 17: Last char was ( following state 15.
	int state = 0;

	if((f = fopen(argv[1], "r")) == NULL) {
		perror("Failed to open input file for processing");
		return -2;
	}

	while(fgets(read_chunk, sizeof(read_chunk), f) != NULL) {
		read_chunk_idx = 0;
		while((curr_char = read_chunk[read_chunk_idx++]) != '\0') {

			if(curr_char == 'm') {
				// Skip the following check switch
				state = 1;
				continue;
			}
			else if(curr_char == 'd') {
				//same as above.
				state = 11;
				continue;
			}
			switch(state) {
				case 0:
					break; // Handled above.
				case 1:
					state = (curr_char == 'u') ? 2 : 0;
					break;
				case 2:
					state = (curr_char == 'l') ? 3 : 0;
					break;
				case 3:
					state = (curr_char == '(') ? 4 : 0;
					break;
				case 4:
					if(is_numeral(curr_char)) {
						// This is a numeral. Initialize curr_num
						// and set state.
						curr_num = curr_char - 48;
						state = 5;
					}
					else {
						// This was an invalid mul operation
						state = 0;
						curr_num = 0;
						arg_count = 0;
						curr_product = 1;
					}
					break;
				case 5:
					if(is_numeral(curr_char)) {
						// This is a numeral
						curr_num = (curr_num * 10) + curr_char - 48;
					}
					else if(curr_char == ',') {
						// We've reached the end of this number
						// Add it to the current product.
						curr_product *= curr_num;
						arg_count++;
						curr_num = 0;
						state = 4;
					}
					else if((curr_char == ')') && (arg_count > 0)) {
						// We've reached the end of this mul op and
						// it is valid. Go back to state 0.
						curr_product *= curr_num;
						total_sum += mul_enabled ? curr_product : 0;
						total_sum_with_disabled += curr_product;
						state = 0;
					}
					else {
						// This was an invalid mul operation.
						// Reset state.
						state = 0;
					}
					if(state == 0) {
						// We have either finished a valid op, or
						// it was invalid, regardless we reset
						// these vars.
						curr_num = 0;
						arg_count = 0;
						curr_product = 1;
					}
					break;
				case 11:
					state = (curr_char == 'o') ? 12 : 0;
					break;
				case 12:
					switch(curr_char) {
						case 'n':
							state = 13;
							break;
						case '(':
							state = 16;
							break;
						default:
							state = 0;
							break;
					}
					break;
				case 13:
					state = (curr_char == '\'') ? 14 : 0;
					break;
				case 14:
					state = (curr_char == 't') ? 15 : 0;
					break;
				case 15:
					state = (curr_char == '(') ? 17: 0;
					break;
				case 16: 
					if(curr_char == ')') {
						// We've encountered a valid do() command.
						mul_enabled = 1;
					}
					state = 0;
					break;
				case 17:
					if(curr_char == ')') {
						// We've encountered a valid don't command.
						mul_enabled = 0;
					}
					state = 0;
					break;
			}
		}
	}

	fclose(f);

	printf("%d\n", total_sum_with_disabled);
	printf("%d\n", total_sum);
	return 0;
}
