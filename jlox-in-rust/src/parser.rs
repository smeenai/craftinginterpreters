use std::iter::Peekable;
use std::slice::Iter;

use crate::error;
use crate::expr::{Expr, Literal as ExprLiteral};
use crate::token::{Literal as TokenLiteral, Token};
use crate::token_type::TokenType;

pub struct Parser<'a> {
    current: Peekable<Iter<'a, Token<'a>>>,
}

struct ParseError;

type ExprResult<'a> = Result<Box<Expr<'a>>, ParseError>;

impl<'a> Parser<'a> {
    pub fn new(tokens: &'a [Token]) -> Self {
        Parser {
            current: tokens.iter().peekable(),
        }
    }

    pub fn parse(&mut self) -> Option<Box<Expr<'a>>> {
        self.expression().ok()
    }

    fn expression(&mut self) -> ExprResult<'a> {
        self.equality()
    }

    fn equality(&mut self) -> ExprResult<'a> {
        self.binary(
            Self::comparison,
            &[TokenType::BangEqual, TokenType::EqualEqual],
        )
    }

    fn comparison(&mut self) -> ExprResult<'a> {
        self.binary(
            Self::term,
            &[
                TokenType::Greater,
                TokenType::GreaterEqual,
                TokenType::Less,
                TokenType::LessEqual,
            ],
        )
    }

    fn term(&mut self) -> ExprResult<'a> {
        self.binary(Self::factor, &[TokenType::Minus, TokenType::Plus])
    }

    fn factor(&mut self) -> ExprResult<'a> {
        self.binary(Self::unary, &[TokenType::Slash, TokenType::Star])
    }

    fn binary(
        &mut self,
        next: fn(&mut Self) -> ExprResult<'a>,
        types: &[TokenType],
    ) -> ExprResult<'a> {
        let mut expr = next(self)?;

        while let Some(operator) = self.r#match(types) {
            let right = next(self)?;
            expr = Box::new(Expr::Binary(expr, operator, right));
        }

        Ok(expr)
    }

    fn unary(&mut self) -> ExprResult<'a> {
        if let Some(operator) = self.r#match(&[TokenType::Bang, TokenType::Minus]) {
            let right = self.unary()?;
            return Ok(Box::new(Expr::Unary(operator, right)));
        }

        self.primary()
    }

    fn primary(&mut self) -> ExprResult<'a> {
        let token = self.peek();
        let literal = match token.token_type {
            TokenType::False => Some(Box::new(Expr::Literal(ExprLiteral::Bool(false)))),
            TokenType::True => Some(Box::new(Expr::Literal(ExprLiteral::Bool(true)))),
            TokenType::Nil => Some(Box::new(Expr::Literal(ExprLiteral::Nil))),
            TokenType::Number => match token.literal {
                TokenLiteral::Number(number) => {
                    Some(Box::new(Expr::Literal(ExprLiteral::Number(number))))
                }
                _ => panic!("Number token should have number literal"),
            },
            TokenType::String => match token.literal {
                TokenLiteral::String(string) => {
                    Some(Box::new(Expr::Literal(ExprLiteral::String(string))))
                }
                _ => panic!("String token should have string literal"),
            },
            _ => None,
        };
        if let Some(literal) = literal {
            self.advance();
            return Ok(literal);
        }

        if self.r#match(&[TokenType::LeftParen]).is_some() {
            let expr = self.expression()?;
            self.consume(TokenType::RightParen, "Expect ')' after expression.")?;
            return Ok(Box::new(Expr::Grouping(expr)));
        }

        let token = self.peek();
        self.error(token, "Expect expression.");
        Err(ParseError)
    }

    fn r#match(&mut self, types: &[TokenType]) -> Option<&'a Token<'a>> {
        for &token_type in types {
            if let Some(token) = self
                .current
                .next_if(|&token| token.token_type == token_type)
            {
                return Some(token);
            }
        }

        None
    }

    fn consume(
        &mut self,
        token_type: TokenType,
        message: &str,
    ) -> Result<&'a Token<'a>, ParseError> {
        if self.check(token_type) {
            Ok(self.advance())
        } else {
            let token = self.peek();
            self.error(token, message);
            Err(ParseError)
        }
    }

    fn check(&mut self, token_type: TokenType) -> bool {
        if self.is_at_end() {
            return false;
        }

        self.peek().token_type == token_type
    }

    fn advance(&mut self) -> &'a Token<'a> {
        self.current
            .next_if(|&token| token.token_type != TokenType::Eof)
            .expect("Should not advance past EOF")
    }

    fn is_at_end(&mut self) -> bool {
        self.peek().token_type == TokenType::Eof
    }

    fn peek(&mut self) -> &'a Token<'a> {
        self.current.peek().expect("Should not advance past EOF")
    }

    fn error(&self, token: &Token, message: &str) {
        error::error_at_token(token, message);
    }

    fn synchronize(&mut self) {
        let mut previous = self.advance();

        while !self.is_at_end() {
            if previous.token_type == TokenType::Semicolon {
                return;
            }

            match self.peek().token_type {
                TokenType::Class => return,
                TokenType::Fun => return,
                TokenType::Var => return,
                TokenType::For => return,
                TokenType::If => return,
                TokenType::While => return,
                TokenType::Print => return,
                TokenType::Return => return,
                _ => (),
            }

            previous = self.advance();
        }
    }
}
