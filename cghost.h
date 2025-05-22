#ifndef __CGHOST_H__
#define __CGHOST_H__

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h> // perror
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // exit

//---------------------------------------| DECLARATIONS |

// Dynamic array works on any struct with *items, count and capacity fields
#ifndef DA_GROW_FACTOR
#define DA_GROW_FACTOR 2
#endif

#ifndef DA_INIT_CAPACITY
#define DA_INIT_CAPACITY 64
#endif

#define DA_EMBED(T)                                                            \
  T *items;                                                                    \
  size_t count;                                                                \
  size_t capacity;

#define DA_STRUCT(T, name)                                                     \
  typedef struct name {                                                        \
    DA_EMBED(T)                                                                \
  } name;

#define da_alloc_reserved(da, capacity_)                                       \
  do {                                                                         \
    size_t cap = (capacity_);                                                  \
    (da).items = calloc(cap, sizeof(*(da).items));                             \
    if (NULL == (da).items) {                                                  \
      perror("Failed to allocate memory for dynamic array");                   \
      exit(1);                                                                 \
    }                                                                          \
    (da).capacity = cap;                                                       \
    (da).count = 0;                                                            \
  } while (0)

#define da_alloc(da) da_alloc_reserved((da), DA_INIT_CAPACITY)

#define da_free(da)                                                            \
  do {                                                                         \
    free((da).items);                                                          \
    (da).items = NULL;                                                         \
    (da).count = 0;                                                            \
    (da).capacity = 0;                                                         \
  } while (0)

#define da_is_empty(da) ((da).count == 0)

#define da_push(da, el)                                                        \
  do {                                                                         \
    da_maybe_expand((da));                                                     \
    (da).items[(da).count++] = (el);                                           \
  } while (0)

#define da_pop(da)                                                             \
  (assert(!da_is_empty((da))                                                   \
  && "Attempt to pop from dynamic array with size of 0" ), (da).count -= 1)

// NOTE: this macro is unsafe since it does not check if da is empty,
// so make sure that da has it least 1 item before calling this macro
#define da_back(da) (da).items[(da).count-1]


#define da_swap_remove(da, index)                                              \
  do {                                                                         \
    (da).items[(index)] = (da).items[--(da).count];                            \
  } while (0)

#define da_append_da(da1, da2)                                                 \
  do {                                                                         \
    da_resize((da1), (da1).count + (da2).count);                               \
    for (size_t _i = 0; _i <= (da2).count; _i += 1) {                          \
      (da1).items[(da1).count++] = (da2).items[_i];                            \
    }                                                                          \
  } while (0)

#define da_resize(da, new_capacity)                                            \
  do {                                                                         \
    (da).items = realloc((da).items, sizeof(*(da).items) * (new_capacity));    \
    if (NULL == (da).items) {                                                  \
      perror("Failed to reallocate memory for dynamic array");                 \
      exit(1);                                                                 \
    }                                                                          \
    (da).capacity = new_capacity;                                              \
  } while (0)

#define da_maybe_expand(da)                                                    \
  if ((da).count >= (da).capacity)                                             \
  da_resize((da),                                                              \
            (da).capacity ? (da).capacity *DA_GROW_FACTOR : DA_INIT_CAPACITY)

#define da_insert(da, index, el)                                               \
  do {                                                                         \
    if ((index) > (da).count) {                                                \
      fprintf(stderr, "Index %zu out of bounds for insert (size: %zu)\n",      \
              (size_t)(index), (size_t)(da).count);                            \
      exit(1);                                                                 \
    }                                                                          \
    da_maybe_expand((da));                                                     \
    for (size_t _i = (da).count; _i > (index); --_i) {                         \
      (da).items[_i] = (da).items[_i - 1];                                     \
    }                                                                          \
    (da).items[index] = (el);                                                  \
    (da).count += 1;                                                           \
  } while (0)

#ifndef CGHOST_API
#ifdef CGHOST_STATIC
#define CGHOST_API static
#else
#define CGHOST_API extern
#endif // CGHOST_STATIC
#endif // CGHOST_API

// StringView
typedef struct {
  const char *begin;
  size_t length;
} StringView;

#define sv_empty ((StringView){0})
#define sv_from_cstr(str) ((StringView){.begin = str, .length = strlen(str)})
#define sv_from_cstr_slice(str, offset, len)                                   \
  ((StringView){.begin = (str) + (offset), .length = len})
#define sv_from_constant(c) ((StringView){.begin = (c), .length = sizeof((c))})
#define sv_from_sb(sb) ((StringView){.begin = (sb).items, .length = (sb).count})

#define sv_farg "%.*s"
#define sv_expand(sv) (int)(sv).length, (sv).begin

#define sv_slice(sv, offset, len)                                              \
  (sv_from_cstr_slice((sv).begin, (offset), (len)))

#define sv_advance(sv) (++(sv).begin, --(sv).length)

CGHOST_API bool sv_equals(const StringView *lhs, const StringView *rhs);
CGHOST_API bool sv_starts_with_cstr(const StringView *sv, const char *start);
CGHOST_API bool sv_ends_with_cstr(const StringView *sv, const char *end);

CGHOST_API int sv_index_of(const StringView *sv, int rune);
CGHOST_API int sv_last_index_of(const StringView *sv, int rune);
CGHOST_API int sv_index_of_str(const StringView *sv, const char *str);
CGHOST_API StringView sv_split(StringView *sv, const char *delim);
CGHOST_API StringView sv_split_exclude_delim(StringView *sv, const char *delim);

// StringBuilder

typedef struct {
  char *items;
  size_t count;
  size_t capacity;
} StringBuilder;

#define sb_free(sb) da_free((sb))
#define sb_length(sb) da_count((sb))
#define sb_expand(sb, new_capacity)                                            \
  do {                                                                         \
    if ((sb).capacity < (new_capacity)) {                                      \
      da_resize((sb), (new_capacity));                                         \
    }                                                                          \
  } while (0)

CGHOST_API StringBuilder sb_create(size_t capacity);
CGHOST_API void sb_append_rune(StringBuilder *sb, int rune);
CGHOST_API void sb_append_string_view(StringBuilder *sb, const StringView *sv);
CGHOST_API void sb_append_cstr(StringBuilder *sb, const char *cstr);
CGHOST_API char *sb_get_cstr(StringBuilder *sb);
CGHOST_API void sb_appendf(StringBuilder *sb, const char *fmt, ...);

// IO
CGHOST_API bool read_entire_file(const char *path, StringBuilder *sb);

//---------------------------------------| IMPLEMENATION |
#ifdef CGHOST_IMPLEMENTATION

#include <stdarg.h>

// StringView
CGHOST_API bool sv_equals(const StringView *lhs, const StringView *rhs) {
  assert(NULL != lhs);
  assert(NULL != rhs);

  return lhs->length == rhs->length &&
         0 == strncmp(lhs->begin, rhs->begin, lhs->length);
}

CGHOST_API bool sv_starts_with_cstr(const StringView *sv, const char *start) {
  assert(NULL != sv);

  size_t start_len = strlen(start);
  return sv->length >= start_len && 0 == strncmp(sv->begin, start, start_len);
}

CGHOST_API bool sv_ends_with_cstr(const StringView *sv, const char *end) {
  assert(NULL != sv);

  size_t end_len = strlen(end);
  return sv->length >= end_len &&
         0 == strncmp(sv->begin + sv->length - end_len, end, end_len);
}

CGHOST_API int sv_index_of(const StringView *sv, int rune) {
  assert(NULL != sv);

  const char *found = strchr(sv->begin, rune);
  return found - sv->begin;
}

CGHOST_API int sv_last_index_of(const StringView *sv, int rune) {
  assert(NULL != sv);

  const char *found = strrchr(sv->begin, rune);
  return found - sv->begin;
}

CGHOST_API int sv_index_of_str(const StringView *sv, const char *str) {
  assert(NULL != sv);
  const char *substr = strstr(sv->begin, str);
  return NULL == substr ? -1 : substr - sv->begin;
}

CGHOST_API StringView sv_split(StringView *sv, const char *delim) {
  // returns StringView before the delim (not including)
  // sv becomes StringView from the delim (including) to the end of the given sv
  int index = sv_index_of_str(sv, delim);
  if (index < 0)
    return *sv;

  StringView result = sv_slice(*sv, 0, index);
  *sv = sv_slice(*sv, index, sv->length - index);
  return result;
}

CGHOST_API StringView sv_split_exclude_delim(StringView *sv, const char *delim) {
  int index = sv_index_of_str(sv, delim);
  if (index < 0)
    return *sv;

  StringView result = sv_slice(*sv, 0, index);
  size_t delimlen = strlen(delim);
  *sv = sv_slice(*sv, index+delimlen, sv->length - (index+delimlen));
  return result;
}

// StringBuilder
CGHOST_API StringBuilder sb_create(size_t capacity) {
  StringBuilder sb = {0};
  da_alloc_reserved(sb, capacity ? capacity : DA_INIT_CAPACITY);
  return sb;
}

CGHOST_API void sb_append_rune(StringBuilder *sb, int rune) {
  assert(rune >= -128 && rune <= 127);
  da_push(*sb, (char)rune);
}

CGHOST_API void sb_append_string_view(StringBuilder *sb, const StringView *sv) {
  for (size_t i = 0; i < sv->length; ++i) {
    sb_append_rune(sb, sv->begin[i]);
  }
}

CGHOST_API void sb_append_cstr(StringBuilder *sb, const char *cstr) {
  StringView sv = sv_from_cstr(cstr);
  sb_append_string_view(sb, &sv);
}

CGHOST_API char *sb_get_cstr(StringBuilder *sb) {
  if ('\0' != sb->items[sb->count])
    da_push(*sb, '\0');

  return sb->items;
}

CGHOST_API void sb_appendf(StringBuilder *sb, const char *fmt, ...) {
  assert(NULL != sb);
  va_list args;
  va_start(args, fmt);
  int n = vsnprintf(NULL, 0, fmt, args);
  va_end(args);
  sb_expand(*sb, sb->count + n + 1);
  va_start(args, fmt);
  vsnprintf(sb->items + sb->count, n + 1, fmt, args);
  va_end(args);
  sb->count += n;
}

// IO
CGHOST_API bool read_entire_file(const char *path, StringBuilder *sb) {
  bool result = true;

  FILE *f = fopen(path, "rb");
  if (f == NULL) {
    result = false;
    goto defer;
  }

  if (fseek(f, 0, SEEK_END) < 0) {
    result = false;
    goto defer;
  }

  long m = ftell(f);
  if (m < 0) {
    result = false;
    goto defer;
  }

  if (fseek(f, 0, SEEK_SET) < 0) {
    result = false;
    goto defer;
  }

  size_t new_count = sb->count + m;
  sb_expand(*sb, new_count);

  fread(sb->items + sb->count, m, 1, f);
  if (ferror(f)) {
    result = false;
    goto defer;
  }
  sb->count = new_count;

defer:
  if (!result)
    fprintf(stderr, "Could not read file %s: %s\n", path,
            strerror(errno));
  if (f)
    fclose(f);
  return result;
}

#endif // CGHOST_IMPLEMENTATION
#endif // __CGHOST_H__
