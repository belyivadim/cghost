#define CGHOST_ALLOCATOR_STACK_SIZE 64
#define CGHOST_IMPLEMENTATION
#include "../src/cghost.h"
#include <stdio.h>

#define assert_msg(cond, msg)                                                  \
  if (!(cond)) {                                                               \
    fprintf(stderr, "Test failed: %s\n", msg);                                 \
    return 1;                                                                  \
  }

int test_arena_basic_alloc() {
  Arena arena = {0};
  CgAllocator alloc = arena_create_allocator(&arena);

  CG_ALLOCATOR_PUSH(alloc);

  void *ptr1 = CG_MALLOC(CG_ALLOCATOR_INSTANCE, 100);
  assert_msg(ptr1 != NULL, "Arena alloc failed");

  void *ptr2 = CG_MALLOC(CG_ALLOCATOR_INSTANCE, 200);
  assert_msg(ptr2 != NULL, "Arena second alloc failed");

  // Reuse test
  arena_return(&arena, ptr1);
  arena_return(&arena, ptr2);

  assert_msg(arena.chunks != NULL, "Arena chunk should still exist");

  // Should have reused chunk
  void *ptr3 = CG_MALLOC(CG_ALLOCATOR_INSTANCE, 150);
  assert_msg(ptr3 != NULL, "Arena reuse failed");

  arena_return(&arena, ptr3);
  arena_free(&arena);

  CG_ALLOCATOR_POP();
  return 0;
}

int test_allocator_stack() {
  Arena arena = {0};
  CgAllocator alloc = arena_create_allocator(&arena);

  CG_ALLOCATOR_PUSH(alloc);
  void *p = CG_MALLOC(CG_ALLOCATOR_INSTANCE, 64);
  assert_msg(p != NULL, "Allocator stack failed to allocate");

  CG_ALLOCATOR_POP();
  arena_free(&arena);
  return 0;
}

DA_STRUCT(int, IntArray);

int test_dynamic_array() {
  Arena arena = {0};
  CgAllocator alloc = arena_create_allocator(&arena);

  CG_ALLOCATOR_PUSH(alloc);

  IntArray arr = {0};
  da_alloc_reserved(arr, 8);
  for (int i = 0; i < 10; ++i) {
    da_push(arr, i * 10);
  }

  assert_msg(arr.count == 10, "Array count mismatch");
  assert_msg(arr.items[9] == 90, "Array content mismatch");

  CG_ALLOCATOR_POP();
  arena_free(&arena);
  return 0;
}

#ifdef __CGHOST_MEMORY_DEBUG
int test_debug_header() {
  Arena arena = {0};
  void *ptr = arena_alloc(&arena, 32);
  assert_msg(ptr != NULL, "Debug alloc failed");
  ARENA_TRACE_PTR(ptr);

  ArenaAllocHeader *header =
      (ArenaAllocHeader *)((char *)ptr - offsetof(ArenaAllocHeader, data));
  assert_msg(header->owner != NULL, "Header owner null");
  assert_msg(header->file != NULL, "Header file null");

  arena_return(&arena, ptr);
  arena_free(&arena);
  return 0;
}
#endif

int test_allocator_stack_recursive_inner(int depth, int max_depth) {
  Arena arena = {0};
  CgAllocator alloc = arena_create_allocator(&arena);

  CG_ALLOCATOR_PUSH(alloc);

  // Allocate some data at this recursion level
  for (int i = 0; i < 1000; i += 1) {
    void *mem = CG_MALLOC(CG_ALLOCATOR_INSTANCE, 10 * 1024);
    assert_msg(mem, "Failed alloc in recursion");
  }

  Arena arena2 = {0};
  CgAllocator alloc2 = arena_create_allocator(&arena2);

  CG_ALLOCATOR_PUSH(alloc2);

  // Allocate some data at this recursion level
  for (int i = 0; i < 1000; i += 1) {
    void *mem = CG_MALLOC(CG_ALLOCATOR_INSTANCE, 1024);
    assert_msg(mem, "Failed alloc in recursion");
  }

  // Optional: Use dynamic array at certain depths
  IntArray arr = {0};
  if (depth % 2 == 0) {
    da_alloc(arr);
    for (int i = 0; i < 5; ++i)
      da_push(arr, depth * 100 + i);
    assert_msg(arr.count == 5, "Array count mismatch in recursion");
  }

  // Recurse
  if (depth < max_depth) {
    int r = test_allocator_stack_recursive_inner(depth + 1, max_depth);
    if (r != 0)
      return r;
  }

  if (depth % 2 == 0)
    da_free(arr);

  CG_ALLOCATOR_POP();
  CG_ALLOCATOR_POP();
  arena_free(&arena2);
  arena_free(&arena);
  return 0;
}

int test_allocator_stack_recursive() {
  return test_allocator_stack_recursive_inner(0, 16);
}

int main() {
  int failed = 0;

  failed |= test_arena_basic_alloc();
  failed |= test_allocator_stack();
  failed |= test_dynamic_array();
  failed |= test_allocator_stack_recursive();
#ifdef __CGHOST_MEMORY_DEBUG
  failed |= test_debug_header();
#endif

  if (failed) {
    fprintf(stderr, "Some tests FAILED.\n");
    return 1;
  }

  printf("All tests passed.\n");
  return 0;
}
