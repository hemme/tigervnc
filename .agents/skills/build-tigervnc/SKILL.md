---
name: build-tigervnc
description: How to build TigerVNC / vncviewer on this Windows machine. Use whenever the user asks to build, compile, rebuild, link, or test-build anything in C:\git\tigervnc (vncviewer, vncserver, common libs), even for a quick syntax check after editing sources.
---

# TigerVNC build (Windows / MSYS2 MinGW64)

- Repo: `C:/git/tigervnc`
- Build dir: `C:/git/tigervnc/build` (CMake, "MinGW Makefiles" generator)
- Toolchain: `E:/Utility/msys64/mingw64` (gcc/g++ 16.1)

## 1. Set PATH before anything else

The agent shell does NOT inherit the MSYS2 PATH. `g++.exe` spawns `cc1plus.exe`,
whose DLLs (libgmp, libmpfr, libisl, ...) live in `mingw64/bin`. Without the PATH
entry, compilation fails with **exit code 1 and NO error message** — if g++
fails silently, it is the PATH, not the code. Fix:

```bash
export PATH="/e/utility/msys64/mingw64/bin:$PATH"
```

Quick sanity check when in doubt (in some scratch dir):

```bash
echo 'int main(){}' > t.cpp && g++ -c t.cpp -o t.o && echo OK
```

## 2. Build commands

```bash
cd C:/git/tigervnc/build
mingw32-make vncviewer   # viewer only (the usual case)
mingw32-make             # everything
```

The build uses `-Werror`: any warning fails the build.

## 3. Running vncviewer.exe locks the link — ASK the user, never kill on your own

`vncviewer.exe` is often left running (it has a system tray icon and stays in
the background). Windows locks a running executable, so the link step fails with:

```
ld.exe: cannot open output file vncviewer.exe: Permission denied
```

Therefore, **before every build**: check whether it is running,

```bash
tasklist //FI "IMAGENAME eq vncviewer.exe"
```

If a process is listed, ask the user first and wait for their answer, e.g.
"vncviewer.exe (PID xxxx) è in esecuzione e blocca il link: vuoi che lo termini?".
The running viewer may be the user's ACTIVE VNC session — killing it
unprompted can disconnect their remote desktop. Only after an explicit "yes":

```bash
taskkill //IM vncviewer.exe //F     # or: taskkill //PID <pid> //F
```

then re-run the build. If the user declines, compilation is still valid
verification — just report that the link needs the viewer closed.
