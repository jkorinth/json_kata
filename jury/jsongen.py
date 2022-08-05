from hypothesis import given, settings
import hypothesis.strategies as st
import json
import unittest

# almost verbatim copy of JSON grammar on https://json.org

json_gen = lambda: ElementGen

value_gen = lambda: st.one_of(
    ObjectGen,
    ArrayGen,
    StringGen,
    NumberGen,
    st.just("true"),
    st.just("false"),
    st.just("null"),
)


@st.composite
def object_gen(draw):
    mem = draw(st.lists(MemberGen))
    return "{" + ",".join(mem) + "}"


ObjectGen = object_gen()


@st.composite
def member_gen(draw):
    ws1 = draw(st.lists(WsGen))
    key = draw(StringGen)
    ws2 = draw(st.lists(WsGen))
    ele = draw(ElementGen)
    return "".join(ws1) + key + "".join(ws2) + ":" + ele


MemberGen = member_gen()


@st.composite
def array_gen(draw):
    els = draw(st.lists(ElementGen))
    return "[" + ",".join(els) + "]"


ArrayGen = array_gen()


@st.composite
def element_gen(draw):
    ws1 = draw(st.lists(WsGen))
    val = draw(ValueGen)
    ws2 = draw(st.lists(WsGen))
    return "".join(ws1) + val + "".join(ws2)


ElementGen = element_gen()


@st.composite
def string_gen(draw):
    chs = draw(st.lists(CharGen))
    return '"' + "".join(chs) + '"'


StringGen = string_gen()

char_gen = lambda: st.one_of(
    st.sampled_from(
        [chr(x) for x in range(0x20, 0x10FFFF) if chr(x) != '"' and chr(x) != "\\"]
    ),
    # escape_gen()
)

escape_gen = lambda: st.one_of(
    st.just('"'),
    st.just("\\"),
    st.just("/"),
    st.just("b"),
    st.just("f"),
    st.just("n"),
    st.just("r"),
    st.just("t"),
    unicode_escape_gen(),
)


@st.composite
def unicode_escape_gen(draw):
    he1 = draw(HexGen)
    he2 = draw(HexGen)
    he3 = draw(HexGen)
    he4 = draw(HexGen)
    return f"u{he1}{he2}{he3}{he4}"


hex_gen = lambda: st.one_of(
    DigitGen,
    st.sampled_from([chr(x) for x in range(ord("A"), ord("G"))]),
    st.sampled_from([chr(x) for x in range(ord("a"), ord("g"))]),
)


@st.composite
def number_gen(draw):
    itg = draw(IntegerGen)
    frc = draw(st.one_of(st.just(""), FractionGen))
    exp = draw(st.one_of(st.just(""), ExponentGen))
    return f"{itg}{frc}{exp}"


NumberGen = number_gen()


@st.composite
def single_digit_integer_gen(draw):
    sig = draw(st.one_of(st.just(""), st.just("-")))
    dig = draw(DigitGen)
    return f"{sig}{dig}"


@st.composite
def multi_digit_integer_gen(draw):
    sig = draw(st.one_of(st.just(""), st.just("-")))
    otn = draw(OneNineGen)
    dig = draw(st.lists(DigitGen, min_size=1))
    return f"{sig}{otn}" + "".join(dig)


IntegerGen = st.one_of(single_digit_integer_gen(), multi_digit_integer_gen())

DigitGen = st.sampled_from([str(i) for i in range(10)])
OneNineGen = st.sampled_from([str(i) for i in range(1, 10)])


@st.composite
def fraction_gen(draw):
    dig = draw(st.lists(DigitGen, min_size=1))
    return "." + "".join(dig)


FractionGen = fraction_gen()


@st.composite
def exponent_gen(draw):
    exp = draw(st.one_of(st.just("E"), st.just("e")))
    sig = draw(SignGen)
    dig = draw(st.lists(DigitGen, min_size=1))
    return f"{exp}{sig}" + "".join(dig)


SignGen = st.one_of(st.just(""), st.just("-"), st.just("+"))

WsGen = st.one_of(
    st.just(""), st.just(" "), st.just("\n"), st.just("\r"), st.just("\t")
)

# using lambdas for these productions allows to keep the same
# order as json.org
ExponentGen = exponent_gen()
HexGen = hex_gen()
CharGen = char_gen()
ValueGen = value_gen()
ElementGen = element_gen()
JsonGen = json_gen()


class TestJsonGen(unittest.TestCase):
    """Sanity check for the generated JSON based on Python's parser."""

    @settings(max_examples=10000)
    @given(JsonGen)
    def test_json_gen(self, j):
        """Generate JSON, parse and dump with Python to check for basic
        parsing errors."""
        try:
            json.dumps(json.loads(j))
        except:
            print(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>")
            print(j)
            print("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<")
            raise


if __name__ == "__main__":
    unittest.main()
