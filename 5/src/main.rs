use std::collections::{HashMap, HashSet};
use std::io::{BufReader, BufRead};
use std::fs::File;
use std::env;

enum State {
    RULES,
    UPDATE,
}

#[derive(Clone, Default)]
struct PageDeps {
    pub dependents: HashSet<u32>, // What needs to come before this.
}

impl PageDeps {
    pub fn new() -> Self {
        Self {
            dependents: HashSet::new(),
        }
    }
}

fn main() {
    let mut dep_tree: HashMap<u32, PageDeps> = HashMap::new();
    let path = env::args().skip(1).next().expect("Missing <PATH> argument");

    let f = File::open(&path).expect("Failed to open given file");
    let reader = BufReader::new(f);

    let mut state = State::RULES;

    let mut curr_page_order= Vec::<u32>::with_capacity(16);
    let mut middle_sum = 0;
    let mut middle_sum_only_valid = 0;
    let mut was_invalid;

    let empty_deps = PageDeps::default();
    

    for line in reader.lines() {
        let line = line.expect("IO Error occured while reading input file.");
        if line.is_empty() {
            // Update state and continue, we are now doing the updates.
            state = State::UPDATE;
            continue;
        }

        match state {
            State::RULES => {
                let page_numbers = line
                    .split('|')
                    .filter_map(|s| s.parse::<u32>().ok())
                    .collect::<Vec<_>>();
                if page_numbers.len() != 2 {
                    return;
                }
                let page_deps = if let Some(p) = dep_tree.get_mut(&page_numbers[0]) {
                    p
                }
                else {
                    let new_deps= PageDeps::new();
                    dep_tree.insert(page_numbers[0], new_deps);
                    dep_tree.get_mut(&page_numbers[0]).unwrap()

                };
                page_deps.dependents.insert(page_numbers[1]);
            },
            State::UPDATE => {
                let mut pending_pages = line.split(",").filter_map(|s| s.parse::<u32>().ok()).collect::<Vec<_>>();
                let pending_pages_orig= pending_pages.clone();
                let mut indices_to_pop= Vec::new();
                while !pending_pages.is_empty() {
                    for (idx, page_number) in pending_pages.iter().copied().enumerate() {
                        let page_deps = dep_tree.get(&page_number).unwrap_or(&empty_deps);

                        if page_deps.dependents.iter().copied().all(|page| !pending_pages.contains(&page)) {
                            // We can print!
                            curr_page_order.push(page_number);
                            indices_to_pop.push(idx);
                        }
                        else {
                            
                        }
                    }
                    // Reverse it bc we want to pop from right to left
                    // to avoid shifting. 
                    // TODO: Use a VecQueue
                    indices_to_pop.reverse();
                    for idx in indices_to_pop.iter() {
                        pending_pages.remove(*idx);
                    }
                    indices_to_pop.clear();
                }
                curr_page_order.reverse();

                was_invalid = curr_page_order != pending_pages_orig;
                if !curr_page_order.is_empty() {
                    let mid_val = curr_page_order[curr_page_order.len() / 2];
                    if was_invalid {
                        middle_sum += mid_val;
                    }
                    else {
                        middle_sum_only_valid += mid_val;
                    }
                }
                curr_page_order.clear();
            }
        }
    }

    println!("{}", middle_sum_only_valid);
    println!("{}", middle_sum);
}
