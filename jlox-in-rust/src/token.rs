use crate::token_type::TokenType;

pub struct Token<'a> {
    pub r#type: TokenType<'a>,
    pub lexeme: &'a str,
    pub line: u32,
}
