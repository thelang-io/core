# The Programming Language
This is the official repository of The Programming Language.

The Programming Language is a rapid, minimalistic, elegant, high-performance,
high-level programming language.
- **Unique:** The syntax was produced by a combination of best features of all
  known programming languages in the world
- **Fast:** Speed of programs produced by The Programming Language is so fast
  that comparable with writing applications in The C Programming Language
- **Helpful:** The Programming Language takes care of everything for you, so
  that you can focus on writing code
- **Independent:** To compile your code The Programming Language doesn't use
  any 3rd party dependencies
- **Portable:** The Programming Language works on
  [many](#supported-operating-systems) operating systems
- **Cloud-Based:** The Programming Language runs in cloud which allows to
  reduce time spent on installing and compiling

## Mission
Our mission is to support every possible scenario of development with less
effort from developer and centralize development around The Programming
Language by providing building, deployment and testing tools.

## How To Contribute
To contribute, please send us a [pull
request](https://github.com/thelang-io/the/compare) from your fork of this
repository. \
New developers may find notes in [CONTRIBUTING.md](CONTRIBUTING.md) file
helpful to start contributing.

## Using
In order to use The Programming Language please refer to
[CLI](https://github.com/thelang-io/cli).

## File Extension
The Programming Language files have no file extension.

## Supported Operating Systems
The Programming Language is built and tested on the following operating
systems:

| Operating System | Version           | Architecture      | Support          |
|:---------------- | ----------------- | ----------------- | ---------------- |
| MacOS            | 10.10 - 12.5      | Intel x86-64      | Official         |
|                  |                   | Apple Silicon     | Not tested       |
|                  |                   |                   |                  |
| Linux            | 20.04             | Intel x86-64      | Official         |
|                  |                   |                   |                  |
| Windows          | 10                | Intel x86-64      | Official         |
|                  |                   |                   |                  |

## Testing

```sh
$ cmake . -D BUILD_TESTS=ON
$ cmake --build .
$ ctest --output-on-failure
```

## Memory Checking

```sh
$ cmake . -D BUILD_TESTS=ON -D TEST_CODEGEN_MEMCHECK=ON
$ cmake --build .
$ ctest --output-on-failure
```

## Coverage

```sh
$ cmake . -D BUILD_COVERAGE=ON -D BUILD_TESTS=ON
$ cmake --build .
$ gcov src/*
```

## The Author
The Programming Language was designed and developed by
[Aaron Delasy](https://github.com/delasy).
