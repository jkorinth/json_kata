#![recursion_limit = "4096"]
#![allow(dead_code)]
#![allow(unused_must_use)]
use std::collections::HashMap;
use std::io;
use std::io::Read;
use std::iter::Peekable;
use std::result::Result;

static WHITESPACE: [char; 4] = [' ', '\n', '\r', '\t'];
static ESCAPE: [char; 8] = ['"', '\\', '/', 'b', 'f', 'n', 'r', 't'];
static DIGIT: [char; 10] = ['0', '1', '2', '3', '4', '5', '6', '7', '8', '9'];
static HEX: [char; 12] = ['a', 'b', 'c', 'd', 'e', 'f', 'A', 'B', 'C', 'D', 'E', 'F'];

#[derive(Debug)]
enum Element {
    Null(),
    Bool(bool),
    Int(i64),
    Double(f64),
    Str(String),
    Array(Box<Vec<Element>>),
    Object(Box<HashMap<String, Element>>),
    Error(String),
}

fn skip_ws<I>(iter: &mut Peekable<I>)
where
    I: Iterator<Item = char>,
{
    while iter.peek().is_some() && WHITESPACE.contains(iter.peek().unwrap()) {
        iter.next();
    }
}

fn expect<I>(iter: &mut Peekable<I>, c: char) -> Result<&mut Peekable<I>, String>
where
    I: Iterator<Item = char>,
{
    if iter.peek() != Some(&c) {
        Err(format!("expected '{c}', found {:?}", iter.peek()))
    } else {
        iter.next();
        Ok(iter)
    }
}

fn parse_value<I>(iter: &mut Peekable<I>) -> Result<Element, String>
where
    I: Iterator<Item = char>,
{
    match iter.peek() {
        Some('{') => parse_object(iter),
        Some('[') => parse_array(iter),
        Some('"') => parse_string(iter),
        Some('t') => parse_true(iter),
        Some('f') => parse_false(iter),
        Some('n') => parse_null(iter),
        Some(_) => parse_number(iter),
        None => Err("premature end of stream".to_string()),
    }
}

fn parse_key_value<I>(iter: &mut Peekable<I>) -> Result<(Element, Element), String>
where
    I: Iterator<Item = char>,
{
    let key = parse_string(iter)?;
    skip_ws(iter);
    expect(iter, ':');
    skip_ws(iter);
    let ele = parse_element(iter)?;
    Ok((key, ele))
}

fn parse_object<I>(iter: &mut Peekable<I>) -> Result<Element, String>
where
    I: Iterator<Item = char>,
{
    expect(iter, '{')?;
    skip_ws(iter);
    if iter.peek() == Some(&'}') {
        expect(iter, '}')?;
        Ok(Element::Object(Box::new(HashMap::new())))
    } else {
        let mut d = HashMap::<String, Element>::new();
        loop {
            skip_ws(iter);
            let kv = parse_key_value(iter)?;
            match kv {
                (Element::Str(key), val) => d.insert(key, val),
                _ => None,
            };
            skip_ws(iter);
            if iter.peek() != Some(&',') {
                break;
            }
            iter.next();
        }
        expect(iter, '}')?;
        Ok(Element::Object(Box::new(d)))
    }
}

fn parse_array<I>(iter: &mut Peekable<I>) -> Result<Element, String>
where
    I: Iterator<Item = char>,
{
    expect(iter, '[')?;
    skip_ws(iter);
    if iter.peek() == Some(&']') {
        expect(iter, ']')?;
        Ok(Element::Array(Box::new(vec![])))
    } else {
        let mut v = Vec::<Element>::new();
        loop {
            skip_ws(iter);
            v.push(parse_element(iter)?);
            if iter.peek() != Some(&',') {
                break;
            }
            iter.next();
        }
        expect(iter, ']')?;
        Ok(Element::Array(Box::new(v)))
    }
}

macro_rules! is_hex {
    ($c: expr) => {
        DIGIT.contains($c) || HEX.contains($c)
    };
}

fn parse_string<I>(iter: &mut Peekable<I>) -> Result<Element, String>
where
    I: Iterator<Item = char>,
{
    expect(iter, '"')?;
    let mut s: String = String::from("");
    loop {
        match iter.peek() {
            Some('\\') => {
                s.push(iter.next().unwrap());
                s.push(iter.next().unwrap());
                while is_hex!(iter.peek().unwrap_or(&' ')) {
                    s.push(iter.next().unwrap());
                }
            }
            Some('"') => {
                break;
            }
            _ => {
                s.push(iter.next().unwrap());
            }
        }
    }
    expect(iter, '"')?;
    Ok(Element::Str(s))
}

fn parse_number<I>(iter: &mut Peekable<I>) -> Result<Element, String>
where
    I: Iterator<Item = char>,
{
    let int = parse_integer(iter)?;
    let frc = parse_fraction(iter)?;
    let exp = parse_exponent(iter)?;
    if frc.is_some() || exp.is_some() {
        let s = format!(
            "{int}{}{}",
            frc.unwrap_or(String::from("")),
            exp.unwrap_or(String::from(""))
        );
        let d = s.parse::<f64>();
        if d.is_ok() {
            Ok(Element::Double(d.unwrap()))
        } else {
            Err(format!("expected double in {}", s))
        }
    } else {
        let i = int.parse::<i64>();
        if i.is_ok() {
            Ok(Element::Int(i.unwrap()))
        } else {
            Err(format!("expected int in {}", int))
        }
    }
}

macro_rules! is_digit {
    ($c: expr) => {
        DIGIT.contains($c)
    };
}

fn parse_integer<I>(iter: &mut Peekable<I>) -> Result<String, String>
where
    I: Iterator<Item = char>,
{
    let mut s = String::from("");
    if iter.peek().is_some() && iter.peek().unwrap() == &'-' {
        s.push(iter.next().unwrap());
    }
    if !is_digit!(iter.peek().unwrap_or(&'X')) {
        Err(format!("expected digit, found {:?}", iter.peek()))
    } else {
        while is_digit!(iter.peek().unwrap_or(&'X')) {
            s.push(iter.next().unwrap());
        }
        Ok(s)
    }
}

fn parse_fraction<I>(iter: &mut Peekable<I>) -> Result<Option<String>, String>
where
    I: Iterator<Item = char>,
{
    if iter.peek() != Some(&'.') {
        Ok(None)
    } else {
        let mut s = String::from("");
        s.push(iter.next().unwrap());
        while is_digit!(iter.peek().unwrap_or(&'X')) {
            s.push(iter.next().unwrap());
        }
        Ok(Some(s))
    }
}

fn parse_exponent<I>(iter: &mut Peekable<I>) -> Result<Option<String>, String>
where
    I: Iterator<Item = char>,
{
    let c = iter.peek().unwrap_or(&'X');
    if *c != 'e' && *c != 'E' {
        Ok(None)
    } else {
        let mut s = String::from(iter.next().unwrap());
        let c = *iter.peek().unwrap_or(&'X');
        if c == '-' || c == '+' {
            s.push(iter.next().unwrap());
        }
        while is_digit!(iter.peek().unwrap_or(&'X')) {
            s.push(iter.next().unwrap());
        }
        Ok(Some(s))
    }
}

fn parse_true<I>(iter: &mut I) -> Result<Element, String>
where
    I: Iterator<Item = char>,
{
    let b = iter.next() == Some('t')
        && iter.next() == Some('r')
        && iter.next() == Some('u')
        && iter.next() == Some('e');
    if b {
        Ok(Element::Bool(true))
    } else {
        Err("expected string \"true\"".to_string())
    }
}

fn parse_false<I>(iter: &mut I) -> Result<Element, String>
where
    I: Iterator<Item = char>,
{
    let b = iter.next() == Some('f')
        && iter.next() == Some('a')
        && iter.next() == Some('l')
        && iter.next() == Some('s')
        && iter.next() == Some('e');
    if b {
        Ok(Element::Bool(false))
    } else {
        Err("expected string \"false\"".to_string())
    }
}

fn parse_null<I>(iter: &mut I) -> Result<Element, String>
where
    I: Iterator<Item = char>,
{
    let b = iter.next() == Some('n')
        && iter.next() == Some('u')
        && iter.next() == Some('l')
        && iter.next() == Some('l');
    if b {
        Ok(Element::Null())
    } else {
        Err("expected string \"null\"".to_string())
    }
}

fn parse_element<I>(iter: &mut Peekable<I>) -> Result<Element, String>
where
    I: Iterator<Item = char>,
{
    skip_ws(iter);
    let el = parse_value(iter)?;
    skip_ws(iter);
    Ok(el)
}

fn parse(json: &String) -> Result<Element, String> {
    parse_element(&mut json.chars().peekable())
}

fn read_stdin() -> Vec<u8> {
    let mut stdin = io::stdin();
    let mut data = Vec::new();
    let _n = stdin.read_to_end(&mut data);
    data
}

fn print(elem: &Element) -> String {
    let mut s = String::from("");
    match elem {
        &Element::Bool(b) => {
            if b {
                s.push_str("true");
            } else {
                s.push_str("false")
            }
        }
        &Element::Int(i) => {
            s.push_str(&format!("{}", i));
        }
        &Element::Double(d) => {
            s.push_str(&format!("{}", d));
        }
        &Element::Null() => s.push_str("null"),
        Element::Str(txt) => s.push_str(&format!("\"{}\"", &txt)),
        Element::Array(arr) => {
            let elems = arr
                .iter()
                .map(|e| print(e))
                .collect::<Vec<String>>()
                .join(",");
            s.push_str(&format!("[{}]", elems));
        }
        Element::Object(d) => {
            let elems = d
                .iter()
                .map(|(key, val)| format!("\"{}\":{}", key, print(&val)))
                .collect::<Vec<String>>()
                .join(",");
            s.push_str(&format!("{{{}}}", elems));
        }
        _ => {}
    }
    s
}

fn main() {
    let input = String::from_utf8(read_stdin());
    match input {
        Err(e) => panic!("error reading input: {e:?}"),
        Ok(inp) => println!(
            "{}",
            print(&parse(&inp).expect(&format!("expected to parse input:\n{}", &inp)))
        ),
    }
}
