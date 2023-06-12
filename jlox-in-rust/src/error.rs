// We're single-threaded, so accessing this is safe even though it requires unsafe blocks.
static mut HAD_ERROR: bool = false;

pub fn error(line: u32, message: &str) {
    report(line, "", message);
}

pub fn had_error() -> bool {
    unsafe { HAD_ERROR }
}

pub fn clear_error() {
    unsafe {
        HAD_ERROR = false;
    }
}

fn report(line: u32, r#where: &str, message: &str) {
    eprintln!("[line {}] Error{}: {}", line, r#where, message);
    // We're single-threaded so this is fine
    unsafe {
        HAD_ERROR = true;
    }
}
