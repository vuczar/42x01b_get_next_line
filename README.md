*This project has been created as part of the 42 curriculum by vimauric.*

# get_next_line

## Description

`get_next_line` is a function that reads a file descriptor **one line at a time**.
Each call returns the next line of the stream; repeated calls walk through the whole
file without ever loading it into memory as a whole.

```c
char *get_next_line(int fd);
```

The returned line **includes** its terminating `\n`, except for the last line of a
file that does not end with one. When there is nothing left to read, or when an
error occurs, the function returns `NULL`. The returned string is heap-allocated
and belongs to the caller, who is responsible for freeing it.

The function works on regular files and on standard input alike, and it is
independent of the size of the chunks it reads: the amount of data pulled from the
file descriptor on each `read` is set at compile time through `BUFFER_SIZE` and has
no relation to the length of the lines.

The only external functions allowed in this project are `read`, `malloc` and `free`.
No library from `libft` is used, `lseek` is forbidden, and global variables are not
permitted — the reading state is carried across calls by a single `static` variable.

## Instructions

The project is built from three files: `get_next_line.c`, `get_next_line_utils.c`
and `get_next_line.h`. There is no Makefile; compile the sources together with your
own program.

`BUFFER_SIZE` sets how many bytes each `read` pulls from the file descriptor. It can
be defined at compile time:

```sh
cc -Wall -Wextra -Werror -D BUFFER_SIZE=42 get_next_line.c get_next_line_utils.c main.c -o gnl
```

If the flag is omitted, the header falls back to a default of `42`, so this also
compiles:

```sh
cc -Wall -Wextra -Werror get_next_line.c get_next_line_utils.c main.c -o gnl
```

### Usage

```c
#include "get_next_line.h"
#include <fcntl.h>
#include <stdio.h>

int	main(void)
{
	int		fd;
	char	*line;

	fd = open("file.txt", O_RDONLY);
	line = get_next_line(fd);
	while (line)
	{
		printf("%s", line);
		free(line);
		line = get_next_line(fd);
	}
	close(fd);
	return (0);
}
```

Passing `0` as the file descriptor reads from standard input instead of a file.

Every string returned by `get_next_line` must be freed by the caller. If the caller
stops reading before the end of the file, the leftover data held in the internal
buffer stays allocated until the file is read to completion — reading until `NULL`
releases it.

### Behaviour and limits

- An invalid or negative file descriptor returns `NULL`.
- A `BUFFER_SIZE` of zero or less returns `NULL`.
- An empty file returns `NULL` on the first call.
- A line containing only `\n` is a valid line and is returned as `"\n"`, not `NULL`.
- Once the end of the file is reached, every subsequent call keeps returning `NULL`.
- Behaviour is undefined on binary files and when the file behind the descriptor is
  modified between calls, as stated by the subject.

## The library

Seven functions split across two source files. None of them exceeds 25 lines or
three local variables, as required by the Norm.

### `get_next_line.c`

| Function | Signature | Role |
|---|---|---|
| `get_next_line` | `char *get_next_line(int fd)` | Entry point. Validates the input, drives the three helpers below, and owns the `static` state. |
| `fill_stash` | `char *fill_stash(int fd, char *stash, char *buf)` | Reads from `fd` in chunks of `BUFFER_SIZE` until the stash holds a `\n` or the file ends. |
| `extract_line` | `char *extract_line(char *stash)` | Allocates and returns a copy of the stash up to and including the first `\n`. |
| `advance_stash` | `char *advance_stash(char *stash)` | Allocates the remainder of the stash after the first `\n`, frees the old one, and returns the new one. |

**`get_next_line(int fd)`** — declares `static char *stash`, the only piece of state
that survives between calls. It rejects a negative `fd` and a non-positive
`BUFFER_SIZE`, allocates the read buffer, hands everything to `fill_stash`, and then
splits the result into the line to return and the remainder to keep. Returns the
line read, or `NULL` at end of file or on error.

**`fill_stash(int fd, char *stash, char *buf)`** — loops on `read` while no `\n` has
appeared in the stash and the descriptor is still producing bytes, concatenating each
chunk onto the stash. It null-terminates `buf` itself, since `read` delivers raw
bytes and not a C string. It distinguishes a return of `0` (end of file, a normal
exit) from a return of `-1` (a read error, which frees the stash and returns `NULL`).
The read buffer is allocated by the caller so that a single allocation serves the
whole call. Returns the filled stash, or `NULL` on error.

**`extract_line(char *stash)`** — measures the stash up to the first `\n`, includes
that `\n` in the count when there is one, then allocates and copies exactly that many
bytes. When the stash ends without a `\n`, the line is returned without one, which is
the end-of-file case described by the subject. Returns a freshly allocated line, or
`NULL` if the allocation fails.

**`advance_stash(char *stash)`** — copies whatever follows the first `\n` into a new
allocation and frees the old stash. When there is nothing after the `\n`, or no `\n`
at all, it frees the stash and returns `NULL` rather than an empty string. Returns
the new stash, or `NULL` when nothing is left.

### `get_next_line_utils.c`

| Function | Signature | Role |
|---|---|---|
| `ft_strlen` | `size_t ft_strlen(char *s)` | Length of a string. Returns `0` for `NULL`. |
| `ft_strchr` | `char *ft_strchr(char *s, int c)` | Pointer to the first occurrence of `c`, or `NULL`. Returns `NULL` for a `NULL` string. |
| `ft_strjoin_free` | `char *ft_strjoin_free(char *s1, char *s2)` | Concatenation of `s1` and `s2`, freeing `s1`. |

These are not the `libft` versions — the subject forbids using that library here, and
two of them carry different contracts on purpose.

`ft_strlen` and `ft_strchr` both accept `NULL`, because on the very first call the
stash has not been allocated yet and treating that as a length of `0` and a failed
search removes the need for a special case at every call site.

`ft_strjoin_free` frees its first argument and leaves the second untouched. That
asymmetry is deliberate: `s1` is always the previous stash, which stops existing the
moment the concatenation succeeds, while `s2` is the read buffer, which belongs to
the caller and gets reused on the next iteration. It also frees `s1` when its own
allocation fails, so the accumulated stash cannot leak in the one situation where it
could no longer be recovered.

## Algorithm

### The problem

`read` returns fixed-size chunks of bytes, and those chunks have nothing to do with
where the lines actually end. A single `read` may return half a line, three lines and
a half, or a single byte. Whatever is read past the end of the current line cannot be
given back: `lseek` is forbidden, so the file descriptor's internal offset only ever
moves forward. That surplus has to be stored somewhere until the next call, or it is
lost.

### The approach: a dynamic stash

The state kept between calls is a single heap-allocated string, the *stash*, held in
one `static` variable. Each call runs the same three-step cycle:

1. **Fill** — while the stash contains no `\n` and `read` is still returning bytes,
   read one chunk and append it to the stash.
2. **Extract** — copy the stash up to and including the first `\n` into a new string.
   That is the line returned to the caller.
3. **Advance** — copy whatever follows that `\n` into a new stash and free the old
   one. That remainder is what the next call starts from.

The loop has exactly two exits, and telling them apart is what makes the edge cases
work: either a `\n` appeared, and there is a complete line to return, or `read`
returned `0` and the file is over, in which case whatever is left in the stash is the
last line, returned without a trailing `\n`.

### Why a static variable

The surplus bytes have to outlive the call that read them, but globals are forbidden.
A `static` local gives exactly that: the variable lives for the whole program, like a
global, yet its name is visible only inside `get_next_line`. It is also zero-initialised
by the language, so it starts as `NULL` with no setup code.

The bonus extends this to `static char *stash[MAX_FD]`, indexed by file descriptor.
An array of pointers is still **one** static variable — one declaration, one object —
which satisfies both bonus requirements at once: a single static variable, and
independent reading state for each descriptor.

### Why not read the whole file

Reading everything up front and then slicing it would be simpler, and the subject
explicitly forbids it. The reason is practical: the function must work on standard
input and on pipes, where there is no "whole file" to read — the data arrives as the
other end produces it, and a function that waits for end of file would simply hang.
Stopping at the first `\n` is what makes the same code work for a text file and for
an interactive terminal.

### Why `NULL` instead of an empty stash

When nothing follows the `\n`, `advance_stash` returns `NULL` rather than `""`. This
collapses "the stash is empty" and "the stash does not exist yet" into a single state,
so no call site has to distinguish them. The end-of-file test in `get_next_line`
becomes the single check `if (!stash || !stash[0])`.

### Trade-off

Appending to the stash reallocates and recopies it on every chunk, which makes
building one line quadratic in its length. The cost is invisible at realistic buffer
sizes — a 2000-character line with `BUFFER_SIZE=1` takes a few milliseconds — and
only becomes measurable in the worst case of a very long line read one byte at a time.

The alternative, growing the buffer geometrically or chaining the chunks in a linked
list, is genuinely linear, but it costs extra state to track capacity and length, more
helper functions, and more places for a leak to hide. For a function whose job is to
hand back one line at a time, the simpler structure was worth more than the asymptotic
gain.

## Resources

- `man 2 read`, `man 2 open`, `man 3 malloc` — behaviour of the system calls used,
  in particular the distinction between a return of `0` and a return of `-1`.
- [The C Programming Language](https://en.wikipedia.org/wiki/The_C_Programming_Language),
  Kernighan & Ritchie — chapter 5 on pointers and memory, chapter 7 on input and output.
- [Static variables in C](https://en.cppreference.com/w/c/language/storage_duration) —
  storage duration and internal linkage.
- The 42 Norm, version 4.1 — formatting and structural constraints applied throughout.

### Use of AI

AI (Claude) was used to analyse the subject and the evaluation sheet, to plan the
order of the work, and to review the finished code. The implementation was written by
hand.
