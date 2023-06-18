use std::env;
use std::fs;
use std::io;
use std::io::Write;
use std::process::ExitCode;

mod error;
mod expr;
mod scanner;
mod token;
mod token_type;

use crate::scanner::Scanner;

fn main() -> ExitCode {
    let args: Vec<String> = env::args().collect();
    if args.len() > 2 {
        eprintln!("Usage: {} [script]", args[0]);
        return ExitCode::from(64);
    }

    if args.len() == 2 {
        return run_file(&args[1]);
    }

    run_prompt();
    ExitCode::SUCCESS
}

fn run_file(path: &str) -> ExitCode {
    let file_contents =
        fs::read_to_string(path).unwrap_or_else(|_| panic!("Could not open path {path}"));
    run(&file_contents);

    if error::had_error() {
        ExitCode::from(65)
    } else {
        ExitCode::SUCCESS
    }
}

fn run_prompt() {
    let mut lines: Vec<String> = Vec::new();

    loop {
        print!("> ");
        io::stdout().flush().expect("Could not flush stdout");
        let mut line = String::new();
        match io::stdin().read_line(&mut line) {
            Ok(0) => return,
            Ok(_) => (),
            Err(e) => panic!("Could not read stdin: {e}"),
        }

        run(&line);
        error::clear_error();
        lines.push(line);
    }
}

fn run(source: &str) {
    let mut scanner = Scanner::new(source);
    let tokens = scanner.scan_tokens();
    for token in tokens {
        println!("{token}");
    }
}
