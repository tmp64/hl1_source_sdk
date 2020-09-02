This directory contains libraries to be used for linking by CMake.

*.so files have been patched to fix linking issue: https://github.com/ValveSoftware/halflife/issues/1912

```bash
patchelf --set-soname libtier0.so libtier0.so
patchelf --set-soname libvstdlib.so libvstdlib.so
```
