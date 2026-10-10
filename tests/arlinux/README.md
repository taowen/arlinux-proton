# Android compatibility regressions

`dxvk-singleton.cpp` uses DXVK's actual singleton and reference-counting headers.
It verifies repeated constructor exceptions, shared successful acquisitions,
destruction and reinitialization. It does not require a working Vulkan device.

Build from the GE checkout with LLVM-MinGW and run in an existing Wine prefix:

```sh
x86_64-w64-mingw32-clang++ -static -std=c++17 -I dxvk/src \
    tests/arlinux/dxvk-singleton.cpp -o dxvk-singleton.exe
```

The process returns zero on success and writes `C:\singleton-probe.txt`.
Unpatched DXVK returns a null object after its first constructor exception;
the patched code retries construction and passes all lifecycle checks.
