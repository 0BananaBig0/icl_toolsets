#!/usr/bin/env python3
"""Fault-inject a small traversal budget into the production runtime."""
from pathlib import Path
import shlex
import subprocess
import sys
import tempfile


ROOT = Path(__file__).resolve().parents[3]
DRIVER = r'''
#include "pattern/pattern_runtime.h"
#include "pattern/pattern_netlist.h"

int main() {
  pattern::Design design;
  design.AddIcl("budget.icl", R"(
Module Top {
 DataInPort p[7:0];
 DataRegister value {WriteCallBack Top store <D>;}
})");
  const auto netlist = pattern::Elaborate(design, "Top");
  pattern::Runtime runtime(design, netlist);
  runtime.Execute("budget.pdl", R"(
iPDLLevel 1
iProcsForModule Top
for {set attempt 0} {$attempt < 3} {incr attempt} {
 if {![catch {iWrite p 0} message]} {error {exhausted budget replenished}}
 if {![string match {*callback dependency resource limit exceeded*} $message]} {
  error $message
 }
}
iReset
)");
  runtime.Finish();
}
'''


def main():
    target, *make_args = sys.argv[1:]
    dry_run = subprocess.run(
        ["make", "-n", "-B", *make_args, target], cwd=ROOT,
        capture_output=True, text=True, check=True)
    commands = [shlex.split(line) for line in dry_run.stdout.splitlines()
                if "tests/src/pattern_runtime_test.cc" in line]
    assert len(commands) == 1, dry_run.stdout
    source = (ROOT / "src/pattern/pattern_runtime.cc").read_text()
    initializer = "callback_dependency_budget_ = 4 * 1024 * 1024"
    assert source.count(initializer) == 1
    with tempfile.TemporaryDirectory(prefix="icl-runtime-budget-") as folder:
        folder = Path(folder)
        runtime, driver, binary = [folder / name for name in
                                   ("runtime.cc", "driver.cc", "check")]
        runtime.write_text(source.replace(initializer, "callback_dependency_budget_ = 2"))
        driver.write_text(DRIVER)
        command = commands[0]
        for index, argument in enumerate(command):
            if argument == "tests/src/pattern_runtime_test.cc":
                command[index] = str(driver)
            elif argument.endswith("/pattern/pattern_runtime.o"):
                command[index] = str(runtime)
        command[command.index("-o") + 1] = str(binary)
        subprocess.run(command, cwd=ROOT, check=True)
        subprocess.run([str(binary)], cwd=folder, check=True, timeout=30)
    print("Production runtime budget remains exhausted across three caught errors.")


if __name__ == "__main__":
    main()
