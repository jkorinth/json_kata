from pathlib import Path
import os
import unittest
import time
from hypothesis import given, settings
from .jsongen import JsonGen
from functools import wraps
from subprocess import Popen, PIPE
import json
from statistics import fmean
import sys


class Recorder:
    def __init__(self, exe):
        self.records = []
        self.log = Path(exe).absolute().name + ".json"
        print(f"writing logfile to: {self.log}", file=sys.stderr)
    
    def _compute_stats(self):
        stats = {
            "average runtime": fmean((r["duration"] for r in self.records)),
            "average normalized runtime": fmean((r["duration"] / len(r["json"]) for r in self.records)),
            "average input size": fmean((len(r["json"]) for r in self.records)),
        }
        return stats

    def __del__(self):
        self.write(self)
    
    def write(self):
        with open(self.log, "w+") as f:
            f.write(json.dumps({
                #"runs": [{k:v for k, v in r.items() if not k == "args" or k == "kwargs"} for r in self.records],
                "runs": self.records,
                "stats": self._compute_stats(),
            }))

    def __call__(self, f):
        @wraps(f)
        def wrapper(*args, **kwargs):
            start = time.time()
            ret = f(*args, **kwargs)
            end = time.time()
            #self.records.append({"args": list([*args]), "kwargs":{**kwargs}, "duration": end - start})
            self.records.append({"json": kwargs.get("json"), "duration": end - start})
            return ret
        return wrapper


class Runner(unittest.TestCase):
    exe = None

    @classmethod
    def process_json(cls, json):
        assert cls.exe.exists()
        assert cls.exe.is_file()
        assert os.access(cls.exe, os.X_OK)
        proc = Popen([cls.exe], stdin=PIPE, stdout=PIPE)
        stdout, stderr = proc.communicate(json.encode("utf-8"))
        assert proc.returncode == 0


if __name__ == "__main__":
    unittest.main()
