# Simplify BMP

Siplifies a BMP file to make it usable for specific programs.

* Elimiantes alpha channel from BMP, converting it into a 24 color depth BMP.
* Rewrites BMP using only positive values of width and height.

## Doesn't support

* Indexed BMP.
* Legacy BMP formats.

## Build

```
cmake .
cmake --build .
```

## Usage

```
simplify_bmp [-v] <img1.bmp> [img2.bmp ...]
```

* **-v** Enable verbose.