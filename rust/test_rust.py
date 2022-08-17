from hypothesis import given, settings
from jury.jsongen import JsonGen
from pathlib import Path
from subprocess import Popen, PIPE
import subprocess
import unittest
from jury.runner import Runner, Recorder


class TestRust(Runner):
    exe = Path(__file__).parent / "target" / "release" / "json-rust"
    rec = Recorder(exe)

    @classmethod
    def tearDownClass(cls):
        cls.rec.write()

    @given(JsonGen)
    @settings(max_examples=10000)
    @rec
    def test_rust(self, json):
        TestRust.process_json(json)

