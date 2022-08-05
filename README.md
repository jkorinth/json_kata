My JSON kata
============

To grow as programmers, it is helpful to look at how people train in other
areas, e.g., sports. Most martial arts have a similar notion as Karate's "kata":
A simulated fight against imagined enemies, using a set of well known movements
to practice and internalize techniques.

This is my programming kata: To write a [JSON](json.org) parser in multiple
programming languages, testing against a wide variety of generated test cases.

## The Challenge

 1. Write a program that takes a valid JSON string as input, parses it into an
    AST, then prints it as JSON.
 2. All content must be preserved on this roundtrip, but not its form (e.g., the
    order of keys may vary).

## The Jury

A python unit test will be run against each executable. It generates valid JSON
data to pass to the executable and measures its runtime performance. All
programs will be presented with the same inputs for comparability; generation is
random, but based on a fixed seed.

The output of each program will be parsed by the test program to

 1. check for presence and identical contents of every key,
 2. check validity of the generated JSON

