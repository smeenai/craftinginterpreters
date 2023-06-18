use crate::expr::{Expr, Literal};

pub fn println(expr: &Expr) {
    print(expr);
    println!();
}

fn print(expr: &Expr) {
    match expr {
        Expr::Binary(left, op, right) => parenthesize(op.lexeme, &[left, right]),
        Expr::Grouping(expr) => parenthesize("group", &[expr]),
        Expr::Literal(value) => match value {
            Literal::Bool(boolean) => print!("{boolean}"),
            Literal::Nil => print!("nil"),
            Literal::Number(number) => print!("{number}"),
            Literal::String(string) => print!("{string}"),
        },
        Expr::Unary(op, right) => parenthesize(op.lexeme, &[right]),
    }
}

fn parenthesize(name: &str, exprs: &[&Expr]) {
    print!("({name}");
    for expr in exprs {
        print!(" ");
        print(expr);
    }
    print!(")");
}
