use subenum::subenum;

#[subenum(BinaryTokenType, UnaryTokenType)]
#[derive(PartialEq)]
pub enum TokenType<'a> {
    // Single-character tokens.
    LeftParen,
    RightParen,
    LeftBrace,
    RightBrace,
    Comma,
    Dot,
    Semicolon,

    #[subenum(BinaryTokenType, UnaryTokenType)]
    Minus,
    #[subenum(BinaryTokenType)]
    Plus,
    #[subenum(BinaryTokenType)]
    Slash,
    #[subenum(BinaryTokenType)]
    Star,

    // One or two character tokens.
    Equal,
    #[subenum(UnaryTokenType)]
    Bang,
    #[subenum(BinaryTokenType)]
    BangEqual,
    #[subenum(BinaryTokenType)]
    EqualEqual,
    #[subenum(BinaryTokenType)]
    Greater,
    #[subenum(BinaryTokenType)]
    GreaterEqual,
    #[subenum(BinaryTokenType)]
    Less,
    #[subenum(BinaryTokenType)]
    LessEqual,

    // Literals.
    Identifier,
    String(&'a str),
    Number(f64),

    // Keywords.
    And,
    Class,
    Else,
    False,
    Fun,
    For,
    If,
    Nil,
    Or,
    Print,
    Return,
    Super,
    This,
    True,
    Var,
    While,

    Eof,
}

// I don't think subenum lets you derive only for a subenum :( https://github.com/paholg/subenum/issues/20
impl Copy for BinaryTokenType {}
impl Clone for BinaryTokenType {
    fn clone(&self) -> Self {
        *self
    }
}

impl Copy for UnaryTokenType {}
impl Clone for UnaryTokenType {
    fn clone(&self) -> Self {
        *self
    }
}

pub struct Token<'a> {
    pub r#type: TokenType<'a>,
    pub lexeme: &'a str,
    pub line: u32,
}

pub struct BinaryToken {
    pub r#type: BinaryTokenType,
    pub line: u32,
}

pub struct UnaryToken {
    pub r#type: UnaryTokenType,
    pub line: u32,
}
