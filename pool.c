///////////////////////////////////////////////////////////////////////////// 
// INTEGRITY STATEMENT (v3)
//
// By signing your name and ID below you are stating that you have agreed
// to the online academic integrity statement:
// https://student.cs.uwaterloo.ca/~cs136/current/assignments/integrity.shtml
/////////////////////////////////////////////////////////////////////////////
// I received help from and/or collaborated with: 

// None 
//  
// Name: Zhichen Ni 
// login ID: z29ni 
///////////////////////////////////////////////////////////////////////////// 

#include "pool.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>
#include "cs136-trace.h"


struct node {
  int begin;
  int end;
  int len;
  bool using;
  char *store;
  struct node *next;
  struct node *pre;
};

struct pool {
  char *storage;
  struct node *root;
  int max_space;
  int max_begin;
};



// The following applies to all functions
// requires: pointers to a pool (e.g., p) are valid (i.e., not NULL)
//           all sizes (e.g., size) are greater than zero
// time: (n) is the number of "active" allocations, which is the number
//       of successful allocs that have not been freed

// SEE ASSIGNMENT TEXT for the behaviour rules of pool_alloc and pool_realloc

// pool_create(size) creates a new pool of size characters
// effects: allocates memory (caller must call pool_destroy)
// time: O(1)
struct pool *pool_create(int size) {
  assert(size > 0);
  struct pool *p = malloc(sizeof(struct pool));
  p->storage = malloc(sizeof(char) * size);
  struct node *n = malloc(sizeof(struct node));
  n->store = NULL;
  n->next = NULL;
  n->pre = NULL;
  n->begin = 0;
  n->end = size - 1;
  n->len = size;
  n->using = false;
  p->root = n;
  p->max_space = size;
  p->max_begin = 0;
  return p;
}

// pool_destroy(p) destroys the pool p if there are no active allocations
//   returns true if successful or false if there are active allocations
// effects: the memory at p is invalid (freed) if successful
// time: O(1)
bool pool_destroy(struct pool *p) {
  assert(p);
  if((p->root->using == false) && (p->root->next == NULL)) {
    free(p->root);
    free(p->storage);
    free(p);
    return true;
  }
  return false;
}

void max(struct node *nd, int *max_space, int *max_begin) {
  if (nd == NULL) {
    return;
  }
  if (nd->using) {
    max(nd->next, max_space, max_begin);
  }
  if ((*max_space) < (nd->len)) {
    *max_space = nd->len;
    *max_begin = nd->begin;
  }
  max(nd->next, max_space, max_begin);
}



char *node_alloc(struct node *nd, int size, struct pool *p) {
  if (nd == NULL) {
    return NULL;
  }
  if ((size > (nd->len)) || (true == (nd->using))) {
    return node_alloc(nd->next, size, p);
  }
  if (size == (nd->len)) {
    nd->using = true;
    nd->store = p->storage + nd->begin;
  }
  if (size < (nd->len)) {
    struct node *n = malloc(sizeof(struct node));
    trace_int(nd->begin);
    n->begin = size + nd->begin;
    n->end = nd->end;
    n->len = nd->len - size;
    n->using = false;
    n->store = NULL;
    n->next = nd->next;
    n->pre = nd;
    if (nd->next != NULL) {
      nd->next->pre = n;
    }
    nd->len = size;
    nd->end = size + nd->begin - 1;
    nd->using = true;
    nd->store = p->storage + nd->begin;
    nd->next = n;
  }
  return nd->store;
}

// pool_alloc(p, size) returns a pointer to an uninitialized char array 
//   of size from within pool p, or NULL if no block of size is available
// effects: modifies p if successful
// time: O(n)
char *pool_alloc(struct pool *p, int size) {
  assert(p);
  assert(size > 0);
  char * result = node_alloc(p->root, size, p);
  max(p->root, &(p->max_space), &(p->max_begin));
  return result;
}

bool node_free(struct node *nd, char *addr) {
  if (nd == NULL) {
    return false;
  }
  if (nd->using == false) {
    return node_free(nd->next, addr);
  }
  if (nd->store != addr) {

    return node_free(nd->next, addr);
  }
  if (nd->store == addr) {

    if (nd->next == NULL) {
      if (nd->pre == NULL) {
        nd->using = false;
        nd->store = NULL;
        return true;
      } 
      if (nd->pre->using == true) {
        
        nd->using = false;
        nd->store = NULL;
        return true;
      } else {
        nd->pre->end = nd->end;
        nd->pre->len = nd->pre->end - nd->pre->begin + 1;
        nd->pre->next = nd->next;
        free(nd);
        return true;
      }
    } else {
      if (nd->next->using == true) {
        if (nd->pre == NULL) {
          nd->using = false;
          nd->store = NULL;
          return true;
        } 
        if (nd->pre->using == false) {
          nd->pre->end = nd->end;
          nd->pre->len = nd->pre->end - nd->pre->begin + 1;
          nd->pre->next = nd->next;
          nd->next->pre = nd->pre;
          free(nd);
          return true;
        } else {

          nd->using = false;
          nd->store = NULL;
          return true;
        }
      }
      if (nd->next->using == false) {

        if (nd->pre == NULL) {

          nd->using = false;
          nd->store = NULL;
          nd->end = nd->next->end;

          nd->len = nd->end - nd->begin + 1;
          if (nd->next->next != NULL) {
            nd->next->next->pre = nd;
          }
           nd->next = nd->next->next;

          free(nd->next);
          return true;
        } 
        if (nd->pre->using == false) {
          nd->pre->end = nd->next->end;
          nd->pre->len = nd->pre->end - nd->pre->begin + 1;
          nd->pre->next = nd->next->next;
          if (nd->next->next != NULL) {
            nd->next->next->pre = nd->pre;
          }
          free(nd->next);
          free(nd);
          return true;
        }
        if (nd->pre->using == true) {

          nd->end = nd->next->end;
          nd->len = nd->end - nd->begin + 1;
          nd->using = false;
          nd->store = NULL;
          struct node *temp = nd->next;
          nd->next = nd->next->next;
          if (temp->next != NULL) {
            temp->next->pre = nd;
          }
          free(temp);
          return true;
        }
      }
    }
  }
  return true;
}





// pool_free(p, addr) makes the active allocation at addr available
//   returns true if successful (addr corresponds to an active allocation
//   from a previous call to pool_alloc or pool_realloc) or false otherwise
// effects: modifies p if successful
// time: O(n)
bool pool_free(struct pool *p, char *addr) {
  assert(p);
  bool result = node_free(p->root, addr);
  max(p->root, &(p->max_space), &(p->max_begin));
  return result;
}


char *node_realloc(struct node *nd, char *addr, int size,
                   int *change, int *orig_len, int *orig_beg) {
  if (nd == NULL) {
    return NULL;
  }
  if ((nd->using == false) || (nd->store != addr)) {
    return node_realloc(nd->next, addr, size, change, orig_len, orig_beg);
  }
  if (size == (nd->len)) {
    return nd->store;
  }
  if (size < (nd->len)) {
    if (nd->next == NULL) {
      struct node *new = malloc(sizeof(struct node));
      new->begin = nd->begin + size;
      new->end = nd->end;
      new->len = new->end - new->begin + 1;
      new->using = false;
      new->store = NULL;
      new->next = nd->next;
      new->pre = nd;
      nd->end = nd->begin + size - 1;
      nd->len = nd->end - nd->begin;
      nd->next = new;
      return nd->store;
    }
    if (nd->next->using == true) {
      struct node *new = malloc(sizeof(struct node));
      new->begin = nd->begin + size;
      new->end = nd->end;
      new->len = new->end - new->begin + 1;
      new->using = false;
      new->store = NULL;
      new->next = nd->next;
      new->pre = nd;
      nd->next->pre = new;
      nd->end = nd->begin + size - 1;
      nd->len = nd->end - nd->begin;
      nd->next = new;
      return nd->store;
    }
    if (nd->next->using == false) {
      nd->next->begin = nd->begin + size;
      nd->next->len = nd->next->end - nd->next->begin + 1;
      return nd->store;
    }
  }
  if (size > (nd->len)) {
    if (nd->next != NULL) {
      if (nd->next->using == false) {
        int need = size - nd->len;
        if (nd->next->len == need) {
          nd->end = nd->next->end;
          nd->len = nd->end - nd->begin + 1;
          nd->next = nd->next->next;
          if (nd->next->next != NULL) {
            nd->next->next->pre = nd;
          }
          free(nd->next);
          return nd->store;
        }
        if (nd->next->len > need) {
          nd->next->begin = nd->begin + size;
          nd->next->len = nd->next->end - nd->next->begin + 1;
          nd->end = nd->begin + size - 1;
          nd->len = nd->end - nd->begin + 1;
          return nd->store;
        }
      }
    }
  }
  *change = 1;
  *orig_len = nd->len;
  *orig_beg = nd->begin;
  return nd->store;
}


// pool_realloc(p, addr, size) changes the size of the active allocation at
//   addr and returns the new address for the allocation.
//   returns NULL if addr does not correspond to an active allocation 
//   or the pool can not be resized (in which case the original allocation
//   does not change)
// effects: modifies p if successful
// time: O(n) + O(k) where k is min(size, m) and 
//       m is the size of the original allocation
char *pool_realloc(struct pool *p, char *addr, int size) {
  assert(p);
  assert(size > 0);
  int change = 0;
  int orig_len = 0;
  int orig_beg = 0;
  char *result = node_realloc(p->root, addr, size, 
                              &change, &orig_len, &orig_beg);
  if (change == 1) {
    char *curr = pool_alloc(p, size);
    if (curr == NULL) {
      max(p->root, &(p->max_space), &(p->max_begin));
      return NULL;
    }
    for (int i = 0; i < orig_len; i++) {
      curr[i] = p->storage[orig_beg];
    }
    pool_free(p, addr);
    max(p->root, &(p->max_space), &(p->max_begin));
    return curr;
  }  
  max(p->root, &(p->max_space), &(p->max_begin));
  return result;
}

void my_print(struct node *nd, int *first, bool use) {
  if (nd == NULL) {
    return;
  }
  if (nd->using == use) {
    if ((*first) == 0) {
      *first = 1;
      printf(" %d [%d]", nd->begin, nd->len);
    } else {
      printf(", %d [%d]", nd->begin, nd->len);
    }
  }
  my_print(nd->next, first, use);
}

// pool_print_active(p) prints out a description of the active allocations 
//   in pool p using the following format:
//   "active: index1 [size1], index2 [size2], ..., indexN [sizeN]\n" or
//   "active: none\n" if there are no active allocations
//   where the index of an allocation is relative to the start of the pool
// effects: produces output
// time: O(n)
void pool_print_active(const struct pool *p) {
  assert(p);
  int first = 0;
  printf("active:");
  my_print(p->root, &first, true);
  if (first == 0) {
    printf(" none");
  }
  printf("\n");
}

// pool_print_available(p) prints out a description of the available 
//   contiguous blocks of memory still available in pool p:
//   "available: index1 [size1], index2 [size2], ..., indexM [sizeM]\n" or
//   "available: none\n" if all of the pool has been allocated
// notes: It is impossible for two blocks of available memory to be adjacent.
//        If two blocks are adjacent, they should be merged to be a single,
//        larger block. In other words: index_K+1 > index_K + size_K
// effects: produces output
// time: O(n)
void pool_print_available(const struct pool *p) {
  assert(p);
  int first = 0;
  printf("available:");
  my_print(p->root, &first, false);
  if (first == 0) {
    printf(" none");
  }
  printf("\n");
}
