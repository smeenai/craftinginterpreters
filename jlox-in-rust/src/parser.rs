use std::iter::Peekable;
use std::slice::Iter;

use crate::error;
use crate::expr::{Expr, Literal};
use crate::token::{BinaryToken, BinaryTokenType, Token, TokenType, UnaryToken, UnaryTokenType};

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
            &[BinaryTokenType::BangEqual, BinaryTokenType::EqualEqual],
        )
    }

    fn comparison(&mut self) -> ExprResult<'a> {
        self.binary(
            Self::term,
            &[
                BinaryTokenType::Greater,
                BinaryTokenType::GreaterEqual,
                BinaryTokenType::Less,
                BinaryTokenType::LessEqual,
            ],
        )
    }

    fn term(&mut self) -> ExprResult<'a> {
        self.binary(
            Self::factor,
            &[BinaryTokenType::Minus, BinaryTokenType::Plus],
        )
    }

    fn factor(&mut self) -> ExprResult<'a> {
        self.binary(
            Self::unary,
            &[BinaryTokenType::Slash, BinaryTokenType::Star],
        )
    }

    fn binary(
        &mut self,
        next: fn(&mut Self) -> ExprResult<'a>,
        types: &[BinaryTokenType],
    ) -> ExprResult<'a> {
        let mut expr = next(self)?;

        while let Some((r#type, operator)) = self.match_with_type(types) {
            let right = next(self)?;
            expr = Box::new(Expr::Binary(
                expr,
                BinaryToken {
                    r#type,
                    line: operator.line,
                },
                right,
            ));
        }

        Ok(expr)
    }

    fn unary(&mut self) -> ExprResult<'a> {
        if let Some((r#type, operator)) =
            self.match_with_type(&[UnaryTokenType::Bang, UnaryTokenType::Minus])
        {
            let right = self.unary()?;
            return Ok(Box::new(Expr::Unary(
                UnaryToken {
                    r#type,
                    line: operator.line,
                },
                right,
            )));
        }

        self.primary()
    }

    fn primary(&mut self) -> ExprResult<'a> {
        let token = self.peek();
        let literal = match token.r#type {
            TokenType::False => Some(Box::new(Expr::Literal(Literal::Boolean(false)))),
            TokenType::True => Some(Box::new(Expr::Literal(Literal::Boolean(true)))),
            TokenType::Nil => Some(Box::new(Expr::Literal(Literal::Nil))),
            TokenType::Number(number) => Some(Box::new(Expr::Literal(Literal::Number(number)))),
            TokenType::String(string) => Some(Box::new(Expr::Literal(Literal::String(string)))),
            _ => None,
        };
        if let Some(literal) = literal {
            self.advance();
            return Ok(literal);
        }

        if self.r#match(&[TokenType::LeftParen]).is_some() {
            let expr = self.expression()?;
            self.consume(&TokenType::RightParen, "Expect ')' after expression.")?;
            return Ok(Box::new(Expr::Grouping(expr)));
        }

        let token = self.peek();
        self.error(token, "Expect expression.");
        Err(ParseError)
    }

    fn r#match<T: PartialEq<TokenType<'a>>>(&mut self, types: &[T]) -> Option<&'a Token<'a>> {
        self.match_with_index(types).map(|pair| pair.1)
    }

    fn match_with_type<T: Copy + PartialEq<TokenType<'a>>>(
        &mut self,
        types: &[T],
    ) -> Option<(T, &'a Token<'a>)> {
        self.match_with_index(types)
            .map(|(index, token)| (types[index], token))
    }

    fn match_with_index<T: PartialEq<TokenType<'a>>>(
        &mut self,
        types: &[T],
    ) -> Option<(usize, &'a Token<'a>)> {
        for (index, r#type) in types.iter().enumerate() {
            if let Some(token) = self.current.next_if(|&token| r#type == &token.r#type) {
                return Some((index, token));
            }
        }

        None
    }

    fn consume(&mut self, r#type: &TokenType, message: &str) -> Result<&'a Token<'a>, ParseError> {
        if self.check(r#type) {
            return Ok(self.advance());
        }

        let token = self.peek();
        self.error(token, message);
        Err(ParseError)
    }

    fn check(&mut self, r#type: &TokenType) -> bool {
        if self.is_at_end() {
            return false;
        }

        &self.peek().r#type == r#type
    }

    fn advance(&mut self) -> &'a Token<'a> {
        self.current
            .next_if(|&token| !matches!(token.r#type, TokenType::Eof))
            .expect("Should not advance past EOF")
    }

    fn is_at_end(&mut self) -> bool {
        matches!(self.peek().r#type, TokenType::Eof)
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
            if matches!(previous.r#type, TokenType::Semicolon) {
                return;
            }

            match self.peek().r#type {
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
