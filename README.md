# PAD

The userspace application debugging.

## Architecture Support

`x86-64` only.

## API

```c
static PAD_ENTER_POINT(breakpoint) { ... }

struct pad_probe {
    /* target function */
    unsigned long address;
    const char *name;

    unsigned long breakpoint;
    unsigned int flags;
};

int pad_register_probe(struct pad_probe *p);
int pad_unregister_probe(struct pad_probe *p);

/* For external (self-defined) handler, PAD_EXTERNAL_HANDLER_FLAG. */
int pad_init(pad_handler_t handler, unsigned int flags);
int pad_exit(void);
```

## Target function

Add `__pad_trace` attribute to the target function.

```c
static __pad_trace void function(...) { ... }
```

## External (self-defined) handler

Add `__pad_handler` attribute to the handler function.

```c
static __pad_handler handler(void) {
    ...
    /* Call all the breakpoints. */
    pad_builtin_handler();
}
```
