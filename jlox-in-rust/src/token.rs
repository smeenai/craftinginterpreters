use std::fmt;

use crate::token_type::TokenType;

pub enum Literal<'a> {
    None,
    Number(f64),
    String(&'a str),
}

pub struct Token<'a> {
    pub token_type: TokenType,
    pub lexeme: &'a str,
    pub literal: Literal<'a>,
    pub line: u32,
}

impl<'a> fmt::Display for Token<'a> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let number_str;
        let literal_str = match self.literal {
            Literal::None => "None",
            Literal::Number(number) => {
                number_str = number.to_string();
                &number_str
            }
            Literal::String(string) => string,
        };
        write!(f, "{:?} {} {literal_str}", self.token_type, self.lexeme)
    }
}
