# racket2tmp
This program parses Racket and converts it to horrible, cursed C++20 template metaprogram code that compiles on GCC 10 and up. Uses Flex/Bison for parsing and GNU Readline for line editing.

The target grammar of supported is a majority subset of Racket's *Intermediate Student with Lambda*, for the sake of achievability. It also supports a few extra features like explicit/implicit `begin`.

Not supported yet:
* All quasiquote features
* `let`, `let*`, `letrec`
* `define-struct`
* Templates
* A whooooole bunch of primitive/built-in operations

Will never be supported:
* *test-case*
* *library-require*
* `time`

Known bugs:
* The linear linked-list environments suck
* Quoting is not quite as flexible as it ought to be. There are some subtleties with nested quotes which I haven't been very thoughtful with.
