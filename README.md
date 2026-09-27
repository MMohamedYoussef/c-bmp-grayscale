# 24-bit BMP grayscale converter (C11)

Convert the pixels of an uncompressed 24-bit Windows BMP to grayscale while preserving its header and row padding. `fixtures/color-grid.bmp` is a synthetic 2×2 image. `examples/grayscale-pixels.txt` shows the deterministic pixel values after conversion.

```sh
cc -std=c11 -O2 -Wall -Wextra -pedantic main.c -o bmp-grayscale
./bmp-grayscale fixtures/color-grid.bmp gray.bmp
cc -std=c11 -O2 -Wall -Wextra -pedantic -Wno-unused-function tests.c -o tests
./tests
```

The output uses integer luma `(77*R + 150*G + 29*B + 128) / 256`, rounded by integer division. The sample red, green, blue, and white pixels become 77, 149, 29, and 255. The parser accepts uncompressed 24-bit BMPs with a DIB header of at least 40 bytes and files up to 64 MiB, including top-down rows. It checks declared file size, dimensions, pixel offset, and padded pixel extent. Unsupported formats and truncation exit nonzero before an output file is created. Output paths must be new files; a partial output is removed if final copying fails.
