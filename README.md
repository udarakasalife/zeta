# Zeta

Bahasa pemrograman kecil, cepat, dan portable. Ditulis dalam C, compile jadi
satu file binary, jalan tanpa dependency apapun. Sintaksnya sengaja pakai
kata biasa (bukan simbol kayak `{ }` `;`) biar gampang dibaca.

```
let x = 10
if x > 5 then
    print "besar"
else
    print "kecil"
end
```

## Fitur

- Interpreter native (C), compile ke satu binary, bisa didownload dan
  langsung jalan di sistem manapun tanpa runtime tambahan
- Sintaks minimalis: `let`, `if/then/else/end`, `while/do/end`
- Database key-value bawaan (`import db`, `store`, `fetch`), datanya
  otomatis tersimpan ke file lokal
- Interop ke Node.js (`node "kode javascript"`)
- Kata kunci bisa diganti bahasa lain lewat language pack, lihat bagian
  Kontribusi Bahasa di bawah

## Install & jalanin

Butuh `gcc` (atau compiler C lain).

```bash
gcc -O2 -static -o zeta zeta.c
./zeta nama_file.ze
```

Tanpa argumen, `./zeta` masuk mode REPL (ketik baris kode langsung, lihat
hasilnya).

## Contoh program

```
let x = 10
if x > 5 then
    print "besar"
else
    print "kecil"
end
```

```
import db
store "nama" = "Dika"
print fetch "nama"
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

## Kontribusi Bahasa (Language Pack)

Zeta bisa dipakai dengan kata kunci dari bahasa apapun. Panduan lengkap ada
di `LANGS_README.md`. Singkatnya: copy `langs/en.lang`, ganti kata di
sebelah kanan tanda `=` sesuai bahasa lo, kirim pull request ke folder
`langs/`.

Kontributor yang pack bahasanya diterima bakal dicantumkan di
`CONTRIBUTORS.md`.

## Lisensi

MIT License, lihat `LICENSE`. Daftar kontributor ada di `CONTRIBUTORS.md`.
