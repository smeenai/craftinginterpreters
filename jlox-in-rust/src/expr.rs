use crate::token::{BinaryToken, UnaryToken};

pub enum Literal<'a> {
    Nil,
    Boolean(bool),
    Number(f64),
    String(&'a str),
}

pub enum Expr<'a> {
    Binary(Box<Expr<'a>>, BinaryToken, Box<Expr<'a>>),
    Grouping(Box<Expr<'a>>),
    Literal(Literal<'a>),
    Unary(UnaryToken, Box<Expr<'a>>),
}
