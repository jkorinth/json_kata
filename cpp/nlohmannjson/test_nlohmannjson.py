from hypothesis import given, settings
from jury.jsongen import JsonGen
from pathlib import Path
from subprocess import Popen, PIPE
import subprocess
import unittest
from jury.runner import Runner, Recorder


class TestNLohmannJson(Runner):
    exe = Path(__file__).parent / "nlohmannjson"
    rec = Recorder(exe)

    @classmethod
    def tearDownClass(cls):
        cls.rec.write()

    @given(JsonGen)
    @settings(max_examples=10000)
    @rec
    def test_nlohmannjson(self, json):
        TestNLohmannJson.process_json(json)

