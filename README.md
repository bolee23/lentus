# Lentus
Lentus is a C-style dynamically typed programming language inspired by Lua.

See the `examples` folder for some examples on Lentus' syntax and grammar.

## Features
* If statements
* Loops: For, While, Do While
* Functions and closures
* Lists
* Tables

## Building
```sh
cmake -S . -B build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug
```
Change `DCMAKE_BUILD_TYPE` to `Release` for a release build