use crate::token::Token;

pub enum Literal<'a> {
    Nil,
    Boolean(bool),
    Number(f64),
    String(&'a str),
}

pub enum Expr<'a> {
    Binary(Box<Expr<'a>>, &'a Token<'a>, Box<Expr<'a>>),
    Grouping(Box<Expr<'a>>),
    Literal(Literal<'a>),
    Unary(&'a Token<'a>, Box<Expr<'a>>),
}
