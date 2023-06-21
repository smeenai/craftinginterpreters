use crate::error;
use crate::expr::{Expr, Literal};
use crate::token::UnaryTokenType;
use crate::value::{Value, ValueError};

pub struct Interpreter {
    // This will gain some fields later.
}

struct RuntimeError {
    message: &'static str,
    line: u32,
}

impl Interpreter {
    pub fn new() -> Self {
        Self {}
    }

    pub fn interpret(&self, expr: &Expr) {
        let result = self.evaluate(expr);
        match result {
            Ok(value) => println!("{value}"),
            Err(RuntimeError { message, line }) => error::runtime_error(message, line),
        }
    }

    // We'll use self later
    #[allow(clippy::only_used_in_recursion)]
    fn evaluate(&self, expr: &Expr) -> Result<Value, RuntimeError> {
        match expr {
            Expr::Literal(literal) => match *literal {
                Literal::Nil => Ok(Value::Nil),
                Literal::Boolean(boolean) => Ok(Value::Boolean(boolean)),
                Literal::Number(number) => Ok(Value::Number(number)),
                Literal::String(string) => Ok(Value::String(string.into())),
            },

            Expr::Grouping(expr) => self.evaluate(expr),

            Expr::Unary(token, right) => {
                let right = self.evaluate(right)?;
                let result = match token.r#type {
                    UnaryTokenType::Bang => !&right,
                    UnaryTokenType::Minus => -&right,
                };
                result.map_err(|ValueError(message)| RuntimeError {
                    message,
                    line: token.line,
                })
            }

            Expr::Binary(left, token, right) => {
                let left = self.evaluate(left)?;
                let right = self.evaluate(right)?;
                let result = match token.r#type {
                    crate::token::BinaryTokenType::Minus => &left - &right,
                    crate::token::BinaryTokenType::Plus => &left + &right,
                    crate::token::BinaryTokenType::Slash => &left / &right,
                    crate::token::BinaryTokenType::Star => &left * &right,
                    crate::token::BinaryTokenType::BangEqual => left.ne(&right),
                    crate::token::BinaryTokenType::EqualEqual => left.eq(&right),
                    crate::token::BinaryTokenType::Greater => left.gt(&right),
                    crate::token::BinaryTokenType::GreaterEqual => left.ge(&right),
                    crate::token::BinaryTokenType::Less => left.lt(&right),
                    crate::token::BinaryTokenType::LessEqual => left.le(&right),
                };
                result.map_err(|ValueError(message)| RuntimeError {
                    message,
                    line: token.line,
                })
            }
        }
    }
}
