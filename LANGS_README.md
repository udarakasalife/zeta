# Adding a New Language to Zeta

Zeta can be written using keywords from any language. The default is
English (`print`, `let`, `if`), but contributors can add packs for other
languages, e.g. Sundanese (`tembongkeun`, `simpen`, `lamun`).

## How to create a new language pack

1. Copy `langs/en.lang`, name it after your language code. For example,
   Javanese: `langs/jv.lang`
2. Translate the right-hand side of each `=`. Do NOT change the left
   side (the canonical name) — that's the key the interpreter reads.

   Format per line: `canonical_name=local_word`

   ```
   print=tulis
   let=simpen
   if=yen
   ...
   ```

3. Math symbols (`+ - * / % > < == != ( )`) are NOT translated, they stay
   the same across every language.

4. Test it by creating a `.ze` file with `@lang <language_code>` as the
   first line, e.g. `@lang jv`

5. Open a pull request adding your new `.lang` file to the `langs/`
   folder.

## Keywords required in every pack

print, let, if, then, else, end, while, do, true, false, and, or, not,
import, store, fetch, node
