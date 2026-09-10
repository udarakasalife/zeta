# Nambahin Bahasa Baru ke Zeta

Zeta bisa dipakai pake kata kunci dari bahasa apapun. Contoh: bahasa utama
pakai Inggris (`print`, `let`, `if`), tapi kontributor bisa bikin pack
bahasa daerah, misal Sunda (`tembongkeun`, `simpen`, `lamun`).

## Cara bikin pack bahasa baru

1. Copy `langs/en.lang`, kasih nama sesuai kode bahasa. Contoh buat Jawa:
   `langs/jv.lang`
2. Ganti bagian kanan tanda `=` sesuai bahasa lo. Bagian kiri (nama
   canonical) JANGAN diubah, itu kunci yang dibaca interpreter.

   Format per baris: `nama_canonical=kata_lokal`

   ```
   print=tulis
   let=simpen
   if=yen
   ...
   ```

3. Simbol matematika (`+ - * / % > < == != ( )`) TIDAK ikut diterjemahin,
   itu sama di semua bahasa.

4. Test dengan bikin file `.ze`, baris pertama diisi `@lang <kode_bahasa>`,
   contoh: `@lang jv`

5. Kirim pull request nambahin file `.lang` baru lo ke folder `langs/`.

## Kata kunci yang wajib ada di setiap pack

print, let, if, then, else, end, while, do, true, false, and, or, not,
import, store, fetch, node
