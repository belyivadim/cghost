# 🧿 cghost

**cghost** is my personal, header-only C library that brings together all the little helpers, macros, and data structures I've needed over the years. It’s meant to make my life easier when hacking on C projects — whether it’s a tool, a game, or just experimenting with ideas. It’s not production-grade (yet), but it’s real-world tested in my own side-projects. If you write a lot of plain C and find yourself reinventing the same stuff over and over, this might save you some time.

This is a work-in-progress (I am introducing new functionality as soon as I need it), but here’s what’s currently included: dynamic arrays with familiar semantics (push, pop, insert, etc.), a string builder for efficient string construction, zero-copy string views, and some basic file I/O utilities.

To use it, just drop `cghost.h` into your project:

```c
#define CGHOST_STATIC  // optional: make all functions static
#define CGHOST_IMPLEMENTATION // if you want implementations in the current file
#include "cghost.h"
```

Dynamic arrays use macros and work with any struct that has fields `items`, `count` and `capacity`, if you need plain dynamic array you can use `DA_STRUCT` macro to define such a struct:

```c
DA_STRUCT(int, IntArray);

IntArray a;
da_alloc(a);
da_push(a, 123);
da_push(a, 456);
printf("%d\n", da_back(a));
da_free(a);
```

String views let you slice and inspect strings efficiently:

```c
StringView sv = sv_from_cstr("hello world");

if (sv_starts_with_cstr(&sv, "hello")) {
    puts("It starts with hello");
}

StringView word = sv_split(&sv, " ");  // "hello", then sv is "world"
printf(sv_farg "\n", sv_expand(word));
```

The string builder makes it easy to build strings dynamically:

```c
StringBuilder sb = sb_create(128);
sb_append_cstr(&sb, "score: ");
sb_appendf(&sb, "%d", 9000);
puts(sb_get_cstr(&sb));
sb_free(sb);
```

You can also load entire files into a string builder:

```c
StringBuilder sb = sb_create(0);
if (read_entire_file("data.txt", &sb)) {
    puts(sb_get_cstr(&sb));
}
sb_free(sb);
```

You can override some defaults before including the header:

```c
#define DA_INIT_CAPACITY 32
#define DA_GROW_FACTOR 1.5
```

Why does this exist? Because I’ve written this stuff too many times. Because I like C. Because I want a personal, battle-tested toolbox that grows with my projects.

This is not production-ready or API-stable. It changes as I go. I break things. You’ve been warned.

Like a ghost in your code: invisible, helpful, quiet. Sometimes spooky.

MIT / public domain — use at your own risk.
