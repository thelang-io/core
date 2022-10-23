#!/usr/bin/env python3

import os
import subprocess
import sys

CORE_PATH = "build\\Debug\\the.exe" if os.name == "nt" else "build/the"


def update(directory: str, action: str):
    files = os.listdir(directory)
    code_sep = "======= code ======="
    flags_sep = "======= flags ======="
    stdin_sep = "======= stdin ======="
    stdout_sep = "======= stdout ======="
    stderr_sep = "======= stderr ======="

    for file in files:
        if file.startswith("."):
            continue

        filepath = directory + "/" + file
        is_error_file = file.startswith("throw-")

        f = open(filepath, "r")
        content = f.read()[len(stdin_sep) + 1:]
        trailing_code = ""

        if action == "codegen":
            stdin = content.split(code_sep)[0]
            trailing_code = flags_sep + content.split(flags_sep)[1]
        elif is_error_file:
            stdin = content.split(stderr_sep)[0]
        else:
            stdin = content.split(stdout_sep)[0]

        f.close()

        f = open(filepath, "w")
        f.write(stdin[0:-1])
        f.close()

        process = subprocess.Popen(
            [CORE_PATH, action, filepath],
            stderr=subprocess.PIPE,
            stdout=subprocess.PIPE,
            text=True
        )

        stdout, stderr = process.communicate()
        f = open(filepath, "w")

        if action == "codegen":
            f.write(
                stdin_sep + os.linesep + stdin + code_sep + os.linesep + stdout[148 + 7 * len(os.linesep):] +
                trailing_code
            )
        elif is_error_file:
            result = "/test" + stderr[stderr.find(":", 10):]
            f.write(stdin_sep + os.linesep + stdin + stderr_sep + os.linesep + result)
        else:
            f.write(stdin_sep + os.linesep + stdin + stdout_sep + os.linesep + stdout)

        f.close()


def main():
    if sys.argv[1] == "ast":
        update("test/ast-test", "ast")
    elif sys.argv[1] == "codegen":
        update("test/codegen-test", "codegen")
    elif sys.argv[1] == "parser":
        update("test/parser-test", "parse")
    else:
        update("test/parser-test", "parse")
        update("test/ast-test", "ast")
        update("test/codegen-test", "codegen")


if __name__ == "__main__":
    main()
