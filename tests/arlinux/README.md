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

`dxcore-properties.c` checks adapter-memory and detachable-property queries,
including support reporting, required sizes, short buffers and null buffers.
Build it with LLVM-MinGW against the prepared Wine headers and run it through
the normal launcher in ARM64, x86-64 and x86 prefixes. It returns zero on
success and writes `C:\dxcore-properties-result.txt`. Memory values come from
Wine's existing adapter inventory; unimplemented properties remain unsupported.

`ucrt-locale.c` loads UCRT dynamically and checks locale names with UTF-8 or
numeric code-page suffixes, multibyte/wide-character round trips and invalid
locale rejection. Run ARM64, x86-64 and x86 builds in an existing prefix.
It returns zero on success and writes `C:\ucrt-locale-result.txt`.

`exception-observer.c` is a bounded, read-only Windows debug-event collector.
It launches one executable, records first-chance C++ types and exception-object
strings, module paths and available backtraces, then detaches. It forwards
exceptions to the application and never writes thread contexts or calls into
the target. Child processes are not debugged. Capture stops after the requested
duration or 16 MiB of output. Raw stack words are evidence, not validated frames;
Wine's ARM64 unwinder can return an incomplete backtrace.

`exception-observer-probe.c` supplies synthetic MSVC exception metadata and
UTF-8/UTF-16 messages. Its own handler must handle both exceptions, with an
unchanged thread environment, in normal and observed runs. The host tool's
`--self-test` checks these invariants before using the collector on real apps.

Build on Linux with LLVM-MinGW:

```sh
aarch64-w64-mingw32-clang -Wall -Wextra -Werror -municode \
    tests/arlinux/exception-observer.c -ldbghelp -o exception-observer.exe
```

Copy the executable into the guest and use the build framework's
`tools/windows-debug.py` through `arlinux-py`, or run that script in the guest:

```sh
python3 windows-debug.py --prefix ~/windows-work/compat \
    --observer ~/exception-observer.exe --output ~/reports/startup-1 \
    --seconds 60 '/path/to/application.exe'
```

Use an existing Proton data directory containing `pfx`. Every capture needs a
new output directory; reports include the runtime manifest and executable hashes.
The target resumes after detachment. A first-chance exception is not necessarily
a failure: the application may handle it. Compare against a normal launch.
These private logs can contain document text or credentials; review and redact
them before sharing. No full-memory dump is collected.
