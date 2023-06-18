mod expr;
mod token;
mod token_type;

use expr::Expr;
use token::Token;
use token_type::TokenType;

fn print(expr: &Expr) {
    match expr {
        Expr::Binary(left, op, right) => parenthesize(op.lexeme, &[left, right]),
        Expr::Grouping(expr) => parenthesize("group", &[expr]),
        Expr::Literal(value) => match value {
            expr::Literal::Bool(boolean) => print!("{boolean}"),
            expr::Literal::Nil => print!("nil"),
            expr::Literal::Number(number) => print!("{number}"),
            expr::Literal::String(string) => print!("{string}"),
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

fn main() {
    let expr = Box::new(Expr::Binary(
        Box::new(Expr::Unary(
            &Token {
                token_type: TokenType::Minus,
                lexeme: "-",
                literal: token::Literal::None,
                line: 1,
            },
            Box::new(Expr::Literal(expr::Literal::Number(123.))),
        )),
        &Token {
            token_type: TokenType::Star,
            lexeme: "*",
            literal: token::Literal::None,
            line: 1,
        },
        Box::new(Expr::Grouping(Box::new(Expr::Literal(
            expr::Literal::Number(45.67),
        )))),
    ));
    print(&expr);
    println!("");
}
