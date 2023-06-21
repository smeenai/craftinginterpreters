use crate::expr::{Expr, Literal};
use crate::token::{BinaryToken, BinaryTokenType, UnaryToken, UnaryTokenType};

pub fn println(expr: &Expr) {
    print(expr);
    println!();
}

fn print(expr: &Expr) {
    match expr {
        Expr::Binary(left, token, right) => parenthesize_binary(token, &[left, right]),
        Expr::Grouping(expr) => parenthesize("group", &[expr]),
        Expr::Literal(value) => match value {
            Literal::Boolean(boolean) => print!("{boolean}"),
            Literal::Nil => print!("nil"),
            Literal::Number(number) => print!("{number}"),
            Literal::String(string) => print!("{string}"),
        },
        Expr::Unary(token, right) => parenthesize_unary(token, &[right]),
    }
}

fn parenthesize_binary(BinaryToken { r#type, .. }: &BinaryToken, exprs: &[&Expr]) {
    match r#type {
        BinaryTokenType::Minus => parenthesize("-", exprs),
        BinaryTokenType::Plus => parenthesize("+", exprs),
        BinaryTokenType::Slash => parenthesize("/", exprs),
        BinaryTokenType::Star => parenthesize("*", exprs),
        BinaryTokenType::BangEqual => parenthesize("!=", exprs),
        BinaryTokenType::EqualEqual => parenthesize("==", exprs),
        BinaryTokenType::Greater => parenthesize(">", exprs),
        BinaryTokenType::GreaterEqual => parenthesize(">=", exprs),
        BinaryTokenType::Less => parenthesize("<", exprs),
        BinaryTokenType::LessEqual => parenthesize("<=", exprs),
    }
}

fn parenthesize_unary(UnaryToken { r#type, .. }: &UnaryToken, exprs: &[&Expr]) {
    match r#type {
        UnaryTokenType::Minus => parenthesize("-", exprs),
        UnaryTokenType::Bang => parenthesize("!", exprs),
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
