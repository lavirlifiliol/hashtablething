#include <stdbool.h>
#include <stdio.h>

/*
** P is a prefix for all symbols of the table
** K is the key type
** V is the value type
** H is an expression which, given `it` of type K returns the hash as an
*unsigned int
** E is an expression which, given `l` and `r` of type K returns whether l and r
*are equal
*/
#define NB_START 8
#define SB_START 4
typedef unsigned long long _flag_t;
#define CREATE_TABLE(P, K, V, H, E)                                            \
  typedef struct {                                                             \
    K *keys;                                                                   \
    V *values;                                                                 \
    size_t *f_bucket;                                                          \
    size_t n_bucket;                                                           \
    size_t s_bucket;                                                           \
  } _##P##t;                                                                   \
  typedef _##P##t *P##t;                                                       \
  size_t P##bsize(P##t ptr) { return ptr->n_bucket * ptr->s_bucket; }          \
  size_t P##bstart(P##t ptr, size_t bucket) { return bucket * ptr->s_bucket; } \
  P##t _##P##new (size_t nb, size_t sb) {                                      \
    P##t ptr = malloc(sizeof(_##P##t));                                        \
    ptr->n_bucket = nb;                                                        \
    ptr->s_bucket = sb;                                                        \
    size_t total_size = P##bsize(ptr);                                         \
    ptr->keys = calloc(total_size, sizeof(K));                                 \
    ptr->values = calloc(total_size, sizeof(V));                               \
    ptr->f_bucket = calloc(ptr->n_bucket, sizeof(size_t));                     \
    return ptr;                                                                \
  }                                                                            \
                                                                               \
  P##t P##new (void) { return _##P##new (NB_START, SB_START); }                \
  void P##free(P##t ptr) {                                                     \
    free(ptr->keys);                                                           \
    free(ptr->values);                                                         \
    free(ptr->f_bucket);                                                       \
    free(ptr);                                                                 \
  }                                                                            \
  void P##put(P##t, K, V);                                                     \
  size_t P##nexti(P##t, size_t);                                               \
  size_t P##bufsize(P##t);                                                     \
  void _##P##resize(P##t table) {                                              \
    size_t ncount = table->n_bucket * 2;                                       \
    size_t nsize = table->s_bucket * 2;                                        \
    P##t n = _##P##new (ncount, nsize);                                        \
    for (size_t i = 0; i < P##bsize(table); i = P##nexti(table, i)) {          \
      P##put(n, table->keys[i], table->values[i]);                             \
    }                                                                          \
    free(table->keys);                                                         \
    free(table->values);                                                       \
    free(table->f_bucket);                                                     \
    *table = *n;                                                               \
    free(n);                                                                   \
  }                                                                            \
  static inline unsigned int P##hash(K it) { return H; }                       \
  static inline _Bool P##eq(K l, K r) { return E; }                            \
  void P##put(P##t table, K k, V v) {                                          \
    unsigned int hash = P##hash(k);                                            \
    size_t bucket = hash % table->n_bucket;                                    \
    size_t start = P##bstart(table, bucket);                                   \
    size_t insertion_point = start;                                            \
    for (;;) {                                                                 \
      K candidate = table->keys[insertion_point];                              \
      if (table->f_bucket[bucket] && !P##eq(candidate, k) &&                   \
          insertion_point - start < table->f_bucket[bucket]) {                 \
        if (insertion_point - start < table->s_bucket - 1) {                   \
          insertion_point++;                                                   \
        } else {                                                               \
          _##P##resize(table);                                                 \
          P##put(table, k, v);                                                 \
          break;                                                               \
        }                                                                      \
      } else {                                                                 \
        table->keys[insertion_point] = k;                                      \
        table->values[insertion_point] = v;                                    \
        table->f_bucket[bucket]++;                                             \
        break;                                                                 \
      }                                                                        \
    }                                                                          \
  }                                                                            \
  _Bool P##get(P##t table, K k, V *place) {                                    \
    unsigned int hash = P##hash(k);                                            \
    size_t bucket = hash % table->n_bucket;                                    \
    size_t start = P##bstart(table, bucket);                                   \
    for (size_t i = 0; i <= table->f_bucket[bucket]; ++i) {                    \
      if (P##eq(k, table->keys[start + i])) {                                  \
        *place = table->values[start + i];                                     \
        return true;                                                           \
      }                                                                        \
    }                                                                          \
    return false;                                                              \
  }                                                                            \
                                                                               \
  size_t P##nexti(P##t table, size_t idx) {                                    \
    if (idx >= P##bsize(table) - 1)                                            \
      return idx + 1;                                                          \
    idx++;                                                                     \
    if (table->keys[idx])                                                      \
      return idx;                                                              \
    return P##nexti(table, idx);                                               \
  }

unsigned int hash_str(char const *ptr) {
  unsigned int res = 123525;
  for (; *ptr; ptr++) {
    res *= (unsigned char)*ptr;
  }
  return res;
}

#ifndef SNOW_ENABLED
int main() {}
#else
#include "snow.h"
CREATE_TABLE(ii_, int, int, it, l == r)
CREATE_TABLE(ss_, char const *, char const *, hash_str(it), strcmp(l, r) == 0)
CREATE_TABLE(c_, int, int, it * 0, l == r)
describe(table) {
  it("can be created") {
    ii_t t = ii_new();
    ii_free(t);
  }
  it("doesn't explode when inserted into") {
    ii_t t = ii_new();
    ii_put(t, 1, 3);
    ii_put(t, 2, 3);
    ii_put(t, 3, 3);
    ii_put(t, 11, 3);
    ii_put(t, 4, 3);
    ii_put(t, 8, 3);
    ii_put(t, 7, 3);
    ii_put(t, 7, 3);
    ii_free(t);
  }
  it("can be used for strings too") {
    ss_t t = ss_new();
    ss_put(t, "1", "2");
    ss_put(t, "2", "3");
    ss_put(t, "6", "9");
    ss_put(t, "3", "2");
    ss_put(t, "1", "2");
    ss_free(t);
  }
  it("can be iterated over") {
    ii_t t = ii_new();
    for (int i = 10000; i < 10016; ++i) {
      ii_put(t, i, i * 2);
    }
    for (size_t i = 0; i < ii_bsize(t); i = ii_nexti(t, i)) {
      asserteq(t->keys[i] * 2, t->values[i]);
    }
    ii_free(t);
  }
  it("can be accessed") {
    ii_t t = ii_new();
    for (int i = 10000; i < 10016; ++i) {
      ii_put(t, i, i * 2);
    }
    for (int i = 10000; i < 10016; ++i) {
      int u;
      assert(ii_get(t, i, &u));
      asserteq(u, i * 2);
    }
    ii_free(t);
  }
  it("can be used for strings too 2") {
    ss_t t = ss_new();
    ss_put(t, "1", "2");
    ss_put(t, "2", "3");
    ss_put(t, "3", "4");
    ss_put(t, "4", "5");
    ss_put(t, "5", "1");
    char const *ptr = "1";
    do {
      assert(ss_get(t, ptr, &ptr));
    } while (*ptr != '1');
    ss_free(t);
  }
  it("handles hash collisions") {
    c_t t = c_new();
    c_put(t, 1, 5);
    c_put(t, 2, 6);
    c_put(t, 3, 7);
    c_put(t, 4, 8);
    c_put(t, 5, 9);
    for (int i = 1; i < 6; ++i) {
      int a;
      assert(c_get(t, i, &a));
      asserteq(a, i + 4);
    }
    c_free(t);
  }
}
snow_main();
#endif
