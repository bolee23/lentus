# Lentus
Lentus is a C-style dynamically typed programming language inspired by Lua.

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