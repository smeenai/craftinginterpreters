use crate::error;
use crate::token::Token;
use crate::token_type::TokenType;

pub struct Scanner<'a> {
    // All the tokens we're scanning for are ASCII, so we can iterate much more simply and
    // efficiently over bytes. UTF-8 means that we'll still handle non-ASCII characters correctly.
    source: &'a [u8],
    source_str: &'a str,
    start: usize,
    current: usize,
    line: u32,
}

impl<'a> Scanner<'a> {
    pub fn new(source: &'a str) -> Self {
        Scanner {
            source: source.as_bytes(),
            source_str: source,
            start: 0,
            current: 0,
            line: 0,
        }
    }

    pub fn scan_tokens(&mut self) -> Vec<Token<'a>> {
        let mut tokens = Vec::new();
        while !self.is_at_end() {
            // We are at the beginning of the next lexeme.
            self.start = self.current;
            if let Some(token) = self.scan_token() {
                tokens.push(token);
            }
        }

        tokens.push(Token {
            r#type: TokenType::Eof,
            lexeme: "",
            line: 0,
        });
        tokens
    }

    fn scan_token(&mut self) -> Option<Token<'a>> {
        let c = self.advance();
        match c {
            b'(' => Some(self.add_token(TokenType::LeftParen)),
            b')' => Some(self.add_token(TokenType::RightParen)),
            b'{' => Some(self.add_token(TokenType::LeftBrace)),
            b'}' => Some(self.add_token(TokenType::RightBrace)),
            b',' => Some(self.add_token(TokenType::Comma)),
            b'.' => Some(self.add_token(TokenType::Dot)),
            b'-' => Some(self.add_token(TokenType::Minus)),
            b'+' => Some(self.add_token(TokenType::Plus)),
            b';' => Some(self.add_token(TokenType::Semicolon)),
            b'*' => Some(self.add_token(TokenType::Star)),

            b'!' => {
                let token_type = if self.r#match(b'=') {
                    TokenType::BangEqual
                } else {
                    TokenType::Bang
                };
                Some(self.add_token(token_type))
            }

            b'=' => {
                let token_type = if self.r#match(b'=') {
                    TokenType::EqualEqual
                } else {
                    TokenType::Equal
                };
                Some(self.add_token(token_type))
            }

            b'<' => {
                let token_type = if self.r#match(b'=') {
                    TokenType::LessEqual
                } else {
                    TokenType::Less
                };
                Some(self.add_token(token_type))
            }

            b'>' => {
                let token_type = if self.r#match(b'=') {
                    TokenType::GreaterEqual
                } else {
                    TokenType::Greater
                };
                Some(self.add_token(token_type))
            }

            b'/' => {
                if self.r#match(b'/') {
                    // A comment goes until the end of the line.
                    while self.peek() != b'\n' && !self.is_at_end() {
                        self.advance();
                    }
                    None
                } else {
                    Some(self.add_token(TokenType::Slash))
                }
            }

            b' ' | b'\r' | b'\t' => None,

            b'\n' => {
                self.line += 1;
                None
            }

            b'"' => self.string(),

            b'0'..=b'9' => self.number(),

            b'A'..=b'Z' | b'a'..=b'z' | b'_' => self.identifier(),

            _ => {
                error::error(self.line, "Unexpected character .");
                None
            }
        }
    }

    fn identifier(&mut self) -> Option<Token<'a>> {
        while let b'0'..=b'9' | b'A'..=b'Z' | b'a'..=b'z' | b'_' = self.peek() {
            self.advance();
        }

        let text = &self.source_str[self.start..self.current];
        let token_type = match text {
            "and" => TokenType::And,
            "class" => TokenType::Class,
            "else" => TokenType::Else,
            "false" => TokenType::False,
            "for" => TokenType::For,
            "fun" => TokenType::Fun,
            "if" => TokenType::If,
            "nil" => TokenType::Nil,
            "or" => TokenType::Or,
            "print" => TokenType::Print,
            "return" => TokenType::Return,
            "super" => TokenType::Super,
            "this" => TokenType::This,
            "true" => TokenType::True,
            "var" => TokenType::Var,
            "while" => TokenType::While,
            _ => TokenType::Identifier,
        };
        Some(self.add_token(token_type))
    }

    fn number(&mut self) -> Option<Token<'a>> {
        while let b'0'..=b'9' = self.peek() {
            self.advance();
        }

        // Look for a fractional part.
        if self.peek() == b'.' && self.peek_next().is_ascii_digit() {
            // Consume the "."
            self.advance();

            while let b'0'..=b'9' = self.peek() {
                self.advance();
            }
        }

        let number: f64 = self.source_str[self.start..self.current].parse().unwrap();
        Some(self.add_token(TokenType::Number(number)))
    }

    fn string(&mut self) -> Option<Token<'a>> {
        while self.peek() != b'"' && !self.is_at_end() {
            if self.peek() == b'\n' {
                self.line += 1;
            }
            self.advance();
        }

        if self.is_at_end() {
            error::error(self.line, "Unterminated string.");
            return None;
        }

        // The closing ".
        self.advance();

        // Trim the surrounding quotes.
        let value = &self.source_str[self.start + 1..self.current - 1];
        Some(self.add_token(TokenType::String(value)))
    }

    fn r#match(&mut self, expected: u8) -> bool {
        if self.is_at_end() {
            return false;
        }

        if self.source[self.current] != expected {
            return false;
        }

        self.current += 1;
        true
    }

    fn peek(&self) -> u8 {
        self.source.get(self.current).copied().unwrap_or(0)
    }

    fn peek_next(&self) -> u8 {
        self.source.get(self.current + 1).copied().unwrap_or(0)
    }

    fn is_at_end(&self) -> bool {
        self.current >= self.source.len()
    }

    fn advance(&mut self) -> u8 {
        let c = self.source[self.current];
        self.current += 1;
        c
    }

    fn add_token(&self, token_type: TokenType<'a>) -> Token<'a> {
        Token {
            r#type: token_type,
            lexeme: &self.source_str[self.start..self.current],
            line: self.line,
        }
    }
}
