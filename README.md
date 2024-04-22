# shootgame

This is a rewrite of our submission to the 3rd 1-bit jam, in C & Raylib.

## Build

## Development Builds

To create a development build, just run `make`. You can speed up the build by passing a job count, e.g. `make -j8`.

You can vary the build with `BUILD_DEBUG` and `BUILD_RELEASE`:
```sh
# Makefile defaults to BUILD_DEBUG=1 and BUILD_RELEASE=0
# 
# BUILD_DEBUG:
#   1 enables debug symbols, and enables UB santization
#   0 disables debug symbols
#
# BUILD_RELEASE:
#   1 enables compiler optimizations, at the cost of compile times
#   0 uses compiler's default optimization settings

# To create a development build with debug symbols and optimizations enabled:
make -j8 BUILD_RELEASE=1

# To create a production-ready build with optimizations enabled and no debug symbols:
make -j8 BUILD_DEBUG=0 BUILD_RELEASE=1
```

## Published Builds

The makefile contains a target for producing a releasable build with all assets packed:

```sh
make publish -j8 BUILD_DEBUG=0 BUILD_RELEASE=1
```

This will output the ready-to-publish build to `publish/`.