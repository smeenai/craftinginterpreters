use std::env;
use std::fs;
use std::io;
use std::io::Write;
use std::process::ExitCode;

mod ast_printer;
mod error;
mod expr;
mod interpreter;
mod parser;
mod scanner;
mod token;
mod value;

use interpreter::Interpreter;

use crate::parser::Parser;
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
    } else if error::had_runtime_error() {
        ExitCode::from(70)
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
    let mut parser = Parser::new(&tokens);
    let expr = parser.parse();

    // Stop if there was a syntax error.
    if error::had_error() {
        return;
    }

    let interpreter = Interpreter::new();
    interpreter.interpret(&expr.expect("Should have expression if there was no syntax error"))
}
