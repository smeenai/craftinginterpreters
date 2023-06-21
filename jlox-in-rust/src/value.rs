use std::fmt;
use std::ops;
use std::rc::Rc;

#[derive(PartialEq)]
pub enum Value {
    Nil,
    Boolean(bool),
    Number(f64),
    String(Rc<str>),
}

pub struct ValueError(pub &'static str);

type ValueResult = Result<Value, ValueError>;

impl Value {
    pub fn is_truthy(&self) -> bool {
        !matches!(self, Value::Nil | Value::Boolean(false))
    }

    // These are convenience wrappers to get the right return type for == and !=.
    pub fn eq(&self, other: &Value) -> ValueResult {
        Ok(Value::Boolean(self == other))
    }

    pub fn ne(&self, other: &Value) -> ValueResult {
        Ok(Value::Boolean(self != other))
    }

    // I can't figure out a way to have a non-bool return type for PartialOrd, so these can't be
    // implemented as operator overloads.
    pub fn lt(&self, other: &Value) -> ValueResult {
        self.numeric_binary_op(other, std::cmp::PartialOrd::lt, Value::Boolean)
    }

    pub fn le(&self, other: &Value) -> ValueResult {
        self.numeric_binary_op(other, std::cmp::PartialOrd::le, Value::Boolean)
    }

    pub fn gt(&self, other: &Value) -> ValueResult {
        self.numeric_binary_op(other, std::cmp::PartialOrd::gt, Value::Boolean)
    }

    pub fn ge(&self, other: &Value) -> ValueResult {
        self.numeric_binary_op(other, std::cmp::PartialOrd::ge, Value::Boolean)
    }

    fn numeric_binary_op<'a, T>(
        &'a self,
        other: &'a Value,
        op: fn(&'a f64, &'a f64) -> T,
        constructor: fn(T) -> Value,
    ) -> ValueResult {
        match (self, other) {
            (Value::Number(a), Value::Number(b)) => Ok(constructor(op(a, b))),
            _ => Err(ValueError("Operands must be numbers.")),
        }
    }
}

impl ops::Add for &Value {
    type Output = ValueResult;

    fn add(self, other: &Value) -> ValueResult {
        match (self, other) {
            (Value::Number(a), Value::Number(b)) => Ok(Value::Number(a + b)),
            (Value::String(a), Value::String(b)) => Ok(Value::String(format!("{a}{b}").into())),
            _ => Err(ValueError("Operands must be two numbers or two strings.")),
        }
    }
}

impl ops::Div for &Value {
    type Output = ValueResult;

    fn div(self, other: &Value) -> ValueResult {
        self.numeric_binary_op(other, ops::Div::div, Value::Number)
    }
}

impl ops::Mul for &Value {
    type Output = ValueResult;

    fn mul(self, other: &Value) -> ValueResult {
        self.numeric_binary_op(other, ops::Mul::mul, Value::Number)
    }
}

impl ops::Neg for &Value {
    type Output = ValueResult;

    fn neg(self) -> ValueResult {
        match self {
            Value::Number(a) => Ok(Value::Number(-a)),
            _ => Err(ValueError("Operand must be a number.")),
        }
    }
}

impl ops::Not for &Value {
    type Output = ValueResult;

    fn not(self) -> ValueResult {
        Ok(Value::Boolean(!self.is_truthy()))
    }
}

impl ops::Sub for &Value {
    type Output = ValueResult;

    fn sub(self, other: &Value) -> ValueResult {
        self.numeric_binary_op(other, ops::Sub::sub, Value::Number)
    }
}

impl fmt::Display for Value {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self {
            Value::Nil => write!(f, "nil"),
            Value::Boolean(boolean) => write!(f, "{}", boolean),
            Value::Number(number) => write!(f, "{}", number),
            Value::String(string) => write!(f, "{}", string),
        }
    }
}
