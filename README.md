# PAD

PAD is the userspace application debugger.
PAD provides a remote and an in-program probe interfaces to insert the breakpoint before the function is executed.

This project includes two parts, PAD core and libpad.
PAD core program allows user to compile the probe program and insert the probe program to the target process.
libpad provides the interface for the process, allowing the user to define the traceable function and in-program probe interface.

---

## Get start

### Build

Build the PAD binaries, core and libpad:

```
$ make              # Build pad core binary
$ make lib          # Build libpad
$ make clean        # Delete generated files
```

### Makefile parameters

- `DEBUG`: Set `1` to enable the debug mode.
- `ARCH`: The target architecture.
- `static`: Build static library instead of dynamic library.

### Architecture Support

Currently, `x86-64` only.

---

## How to use?

### PAD core

```
PAD - the userspace application debugger
Usage: pad [options] file...
Options:
  --COMPILER       The compiler for building probe program
  --CFLAGS         The flag pass to the compiler
  --PROGRAM        The file of probe program to compile
  --TARGET_PID     The pid of process to probe
  --SYMBOL         The symbol of function want to probe
  --ACTION         The action of pad <LOAD|UNLOAD|DEBUG>
```

### libpad - APIs

```c
#include "include/uapi/pad.h"

PAD_ENTER_POINT(breakpoint) { ... }

struct pad_probe {
    /* target function */
    unsigned long address;
    const char *name;

    unsigned long breakpoint;
    unsigned int flags;
};

int pad_register_probe(struct pad_probe *p);
int pad_unregister_probe(struct pad_probe *p);

/*
 * For external (self-defined) handler, PAD_EXTERNAL_HANDLER_FLAG.
 * To allow the PAD core insert the breakpoint, PAD_SET_SHMEM_FLAG.
 */
int pad_init(pad_handler_t handler, unsigned int flags);
int pad_exit(void);
```

#### Target function

Add `__pad_trace` attribute to the target function.

```c
static void __pad_trace function(...) { ... }
```

#### External (self-defined) handler

Add `__pad_handler` attribute to the handler function.

```c
static void __pad_handler handler(void) {
    ...
    /* Call all the breakpoints. */
    pad_builtin_handler();
}
```

---

## TODO List

- double free while the target program failed to handle the signal
- set the prefix of shmem it is located in /dev/shmem
- UNLOAD and DEBUG actions
- eBPF verifier
