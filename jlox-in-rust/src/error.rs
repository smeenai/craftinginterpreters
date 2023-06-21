use crate::token::{Token, TokenType};

// We're single-threaded, so accessing these is safe even though it requires unsafe blocks.
static mut HAD_ERROR: bool = false;
static mut HAD_RUNTIME_ERROR: bool = false;

pub fn error(line: u32, message: &str) {
    report(line, "", message);
}

pub fn error_at_token(token: &Token, message: &str) {
    if matches!(token.r#type, TokenType::Eof) {
        report(token.line, " at end", message);
    } else {
        report(token.line, &format!(" at '{}'", token.lexeme), message);
    }
}

pub fn runtime_error(message: &str, line: u32) {
    eprintln!("{message}\n[line {line}]");
    // We're single-threaded so this is fine
    unsafe {
        HAD_RUNTIME_ERROR = true;
    }
}

pub fn had_error() -> bool {
    unsafe { HAD_ERROR }
}

pub fn had_runtime_error() -> bool {
    unsafe { HAD_RUNTIME_ERROR }
}

pub fn clear_error() {
    unsafe {
        HAD_ERROR = false;
        HAD_RUNTIME_ERROR = false;
    }
}

fn report(line: u32, r#where: &str, message: &str) {
    eprintln!("[line {}] Error{}: {}", line, r#where, message);
    // We're single-threaded so this is fine
    unsafe {
        HAD_ERROR = true;
    }
}
