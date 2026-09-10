# Zeta

A small, fast, portable programming language. Written in C, compiles to a
single binary, runs with zero dependencies. The syntax is deliberately
word-based instead of symbol-heavy (no `{ }`, no `;`) to keep it easy to
read.

```
let x = 10
if x > 5 then
    print "big"
else
    print "small"
end
```

## Features

- Native interpreter (C), compiles to one binary — download and run
  anywhere with no extra runtime
- Minimal syntax: `let`, `if/then/else/end`, `while/do/end`
- Built-in key-value database (`import db`, `store`, `fetch`), data
  persists automatically to a local file
- Node.js interop (`node "javascript code"`)
- Keywords can be swapped for another language via language packs — see
  the Language Contribution section below

## Build & run

Requires `gcc` (or any C compiler).

```bash
gcc -O2 -static -o zeta zeta.c
./zeta your_file.ze
```

With no arguments, `./zeta` starts a REPL (type a line, see the result
immediately).

## Example programs

```
let x = 10
if x > 5 then
    print "big"
else
    print "small"
end
```

```
import db
store "name" = "Dika"
print fetch "name"
```

```
@lang su
simpen x = 10
lamun x > 5 mangka
    tembongkeun "gede"
sanes
    tembongkeun "leutik"
beres
```

## Contributing a Language Pack

Zeta can run with keywords from any language. Full guide in
`LANGS_README.md`. Short version: copy `langs/en.lang`, translate the
right-hand side of each `=`, open a pull request adding your file to
`langs/`.

Contributors whose language pack gets merged are credited in
`CONTRIBUTORS.md`.

## License

MIT License, see `LICENSE`. Contributor list in `CONTRIBUTORS.md`.
