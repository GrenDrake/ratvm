# RatVM

RatVM is a virtual machine designed for creating and playing CYOA-style adventure games. This repository contains the standard compiler as well as a console-based interpreter. A JavaScript-based interpreter is also available under the [PlayRat repository](https://github.com/GrenDrake/playrat) and it is recommended to use that one for regular play.

## Dependencies

[utf8proc](https://github.com/JuliaStrings/utf8proc/) is used to handle normalization and manipulation of Unicode strings.

## Usage

All programs contained in this repository are intended to be executed via the command line.

### Compiler

The compiler is executed in the form of:

```
./build [options] [source files]
```

At least one source file must be specified. If the program uses multiple source files, they must all be specified. By default, the compiler will produce a file named `game.rvm`, but the `-o` option may be used to specify an alternative name.

Several options exist for the compiler, most of which produce output intended for use in debugging the compiler itself and will likely be of little use to game authors. The full list can be found in the documentation -- http://ratvm.grenslair.com/build-play.html#invoking-build.


### Interpreter

The interpreter is executed as:

```
./run [options] [game file]
```


If no game file is specified, the interpreter will attempt to load `game.bin` from the current directory and will fail if it cannot be loaded.
Like the compiler, The interpreter accepts a number of arguments that are described in the documentation -- http://ratvm.grenslair.com/build-play.html#invoking-run.


## License

The RatVM compiler and interpreter contained in this repository are made available under the terms of the [GPL 3.0 license](LICENSE).
