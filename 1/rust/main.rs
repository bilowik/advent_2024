use std::fs::File;

static INPUT: &str = include_str!("../input.txt");


pub fn parse_input(input: &str) -> (Vec<u32>, Vec<u32>) {
    // States:
    //  0: In Number first column
    //  1: In Whitespace
    //  2: In Number 2nd column
    //
    //  0:
    //      Number: Append number to current buffer
    //      Whitespace: Move to state 1, ignore char
    //      Newline: (
    //
    let mut state = 0;
    let mut result = (Vec::new(), Vec::new());
    



    result
}


pub fn main() {
    
}
