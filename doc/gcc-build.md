# Building Rinto as a GCC Frontend

This document describes how to build the Rinto programming language compiler
as a GCC frontend, producing the `grin` and `rin1` executables.

## Prerequisites

- GCC source tree (tested with GCC 12+)
- Standard GCC build dependencies (flex, bison, texinfo, etc.)
- `libmpfr-dev` and `libgmp-dev`
- C++14-capable host compiler

## Directory Setup

Rinto integrates into GCC's build system as a new language frontend. The
source files must be placed inside the GCC source tree under a `rinto/`
directory at the top level (alongside `gcc/`, `libstdc++-v3/`, etc.).

```
gcc-source/
  gcc/
  rinto/
    frontend/       <- contents of src/frontend/
    gcc-backend.hpp
    gcc-backend.cc
    gcc-diagnostics.cc
    rin-system.hpp
    rin1.cc
    lang-specs.h
    Make-lang.in
    config-lang.in
```

### Copying Files

From the Rinto repository root:

```bash
# Assuming $GCC_SRC points to the GCC source root
mkdir -p $GCC_SRC/rinto/frontend

# Frontend sources
cp src/frontend/*.hpp src/frontend/*.cc $GCC_SRC/rinto/frontend/

# GCC backend sources
cp src/gcc/gcc-backend.hpp src/gcc/gcc-backend.cc $GCC_SRC/rinto/
cp src/gcc/gcc-diagnostics.cc $GCC_SRC/rinto/
cp src/gcc/rin-system.hpp $GCC_SRC/rinto/
cp src/gcc/rin1.cc $GCC_SRC/rinto/
cp src/gcc/lang-specs.h $GCC_SRC/rinto/
cp src/gcc/Make-lang.in $GCC_SRC/rinto/
cp src/gcc/config-lang.in $GCC_SRC/rinto/
```

## Configuration

### config-lang.in

This file registers Rinto as a GCC language. Key settings:

| Field | Value | Description |
|-------|-------|-------------|
| `language` | `"rinto"` | Language name for `--enable-languages` |
| `compilers` | `"grin$(exeext)"` | The driver executable name |
| `build_by_default` | `"no"` | Must be explicitly enabled |
| `lang_requires_boot_languages` | `c++` | Requires C++ bootstrap |

### Make-lang.in

This file defines the build rules for the Rinto frontend within GCC's
Makefile infrastructure. It compiles the following object files:

- `rinto/diagnostic.o` - Frontend diagnostic formatting
- `rinto/expressions.o` - Expression AST nodes
- `rinto/file.o` - File I/O and location tracking
- `rinto/operators.o` - Operator definitions and precedence
- `rinto/parser.o` - Syntactic analysis
- `rinto/scanner.o` - Lexical analysis
- `rinto/statements.o` - Statement AST nodes
- `rinto/gcc-backend.o` - GCC tree generation
- `rinto/gcc-diagnostics.o` - GCC diagnostic bridge
- `rinto/rin1.o` - Language hooks (entry point)

Frontend `.cc` files are compiled using the pattern rule:

```makefile
rinto/%.o: rinto/frontend/%.cc
    $(COMPILE) $(RINTOINCLUDES) $<
    $(POSTCOMPILE)
```

With include paths: `-I $(srcdir)/rinto -I $(srcdir)/rinto/frontend`

## Build Steps

### 1. Configure GCC with Rinto enabled

```bash
cd $GCC_SRC
mkdir build && cd build
../configure \
    --enable-languages=c,c++,rinto \
    --disable-multilib \
    --prefix=/usr/local
```

### 2. Build

```bash
make -j$(nproc)
```

This produces two executables:

- `grin` - The compiler driver (like `gcc` or `g++`)
- `rin1` - The actual compiler frontend (invoked by `grin`)

### 3. Install (optional)

```bash
make install
```

This installs `grin` to `$prefix/bin/`.

## Usage

Once built and installed:

```bash
grin -o myprogram myfile.rin
./myprogram
```

The `.rin` file extension is registered in `lang-specs.h`, so GCC
automatically routes `.rin` files through the Rinto frontend.

## Build Artifacts

| Artifact | Description |
|----------|-------------|
| `rin1$(exeext)` | Rinto compiler frontend |
| `grin$(exeext)` | Rinto compiler driver |
| `rinto/*.o` | Compiled object files |

## Troubleshooting

- If `configure` does not recognize `rinto`, ensure `config-lang.in` is
  in the correct location (`$GCC_SRC/rinto/config-lang.in`).
- If frontend headers are not found, verify the include paths in
  `Make-lang.in` point to the correct `frontend/` subdirectory.
- MPFR precision is set to 256 bits in `rin_langhook_init()` (rin1.cc).
  Ensure your MPFR installation supports this.
