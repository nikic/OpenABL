# OpenABL

OpenABL is a work-in-progress domain-specific language for agent based simulations. It it designed to compile to multiple backends.

## Installation

Requirements: `flex` and `bison`. (`sudo apt-get install flex bison`)

For an out-of-source build:

```sh
mkdir ./build
cd ./build
cmake ..
make -j4
```

## Running

Examples are located in the `examples` directory.

To compile the `examples/circle.abl` example using the C backend:

```sh
./OpenABL -i ../examples/circle.abl -o ./output -b c
```

The result will be written into the `./output` directory. To run the generated code:

```sh
cd ./output
./build.sh
./run.sh
```

For the `circle.abl` example, this will create a `points.json` file.

You can also automatically build and run the generated code (if this is supported by the backend):

```sh
# Generate + Build
./OpenABL -i ../examples/circle.abl -o ./output -b c -B
# Generate + Build + Run
./OpenABL -i ../examples/circle.abl -o ./output -b c -R
```

## Help

Output of `./OpenABL --help`:

```
Usage: ./OpenABL -i input.abl -o ./output-dir -b backend

Options:
  -A, --asset-dir    Asset directory (default: ./asset)
  -b, --backend      Backend (default: c)
  -B, --build        Build the generated code
  -h, --help         Display this help
  -i, --input        Input file
  -o, --output-dir   Output directory
  -R, --run          Build and run the generated code

Available backends:
 * c        (working)
 * flame    (mostly working)
 * flamegpu (mostly working)
 * mason    (mostly working)
 * dmason   (mostly working)
```

## Environment configuration

To use the automatic build and run scripts, some environment configuration is required for the
different backends.

 * `c` backend: No dependencies.
 * `flame` backend: `libmboard` must be in PATH. The `FLAME_XPARSER_DIR` environment variable must
   be set to the xparser directory.
 * `flamegpu` backend: The `FLAMEGPU_DIR` environment variable must be set to the FLAMEGPU directory.
 * `mason` backend: Mason must be in the `CLASS_PATH`.
 * `dmason` backend: TODO.

Please report if any necessary environment configuration is missing.
