#include <System/DataTypes/SysArray.h>
#include <System/DataTypes/SysBHeap.h>

/**
 * Binary heap implementation from:
 * http://eloquentjavascript.net/appendix2.html
 */

struct _SysBHeap {
  SysPtrArray *array;
  SysBHeapFunc score_func;
};

void sys_bheap_free(SysBHeap *hp, SysBool free_segment) {
  sys_return_if_fail(hp != NULL);
  SysPtrArray *array = hp->array;

  sys_ptr_array_free(array, free_segment);

  sys_free_N(hp);
}

SysBHeap *sys_bheap_new(SysBHeapFunc func, SysDestroyFunc node_free) {
  SysBHeap *hp = sys_new0_N(SysBHeap, 1);

  hp->score_func = func;
  hp->array = sys_ptr_array_new_with_free_func(node_free);

  return hp;
}

static void sys_bheap_sinkdown(SysBHeap *hp, SysInt n) {
  sys_return_if_fail(hp != NULL);
  sys_return_if_fail(n >= 0);

  SysPtrArray *array = hp->array;

  SysInt length = array->len;
  SysPointer node = array->pdata[n];
  SysDouble elem_score = hp->score_func(node);

  SysInt child2N;
  SysInt child1N;
  SysInt swap;

  SysPointer child1;
  SysDouble child1_score;
  SysPointer child2;
  SysDouble child2_score;

  while(true) {
    child2N = (n + 1) * 2;
    child1N = child2N - 1;

    swap = -1;

    if (child1N < length) {
      child1 = array->pdata[child1N];
      child1_score = hp->score_func(child1);

      if (child1_score < elem_score) {
        swap = child1N;
      }
    }

    if (child2N < length) {
      child2 = array->pdata[child2N];
      child2_score = hp->score_func(child2);

      if (child2_score < (swap == -1 ? elem_score : child1_score)){
        swap = child2N;
      }
    }

    if (swap != -1) {
      array->pdata[n] = array->pdata[swap];
      array->pdata[swap] = node;

      n = swap;

    } else {
      break;
    }
  }
}

static void sys_bheap_bubbleup(SysBHeap *hp, SysInt n) {
  sys_return_if_fail(hp != NULL);

  SysPtrArray *array = hp->array;

  SysInt parentN;
  SysPointer parent;
  SysPointer node = array->pdata[n];

  while (n > 0) {
    parentN = ((n + 1) / 2) - 1;
    parent = array->pdata[parentN];

    if (hp->score_func(node) < hp->score_func(parent)) {
      array->pdata[parentN] = node;
      array->pdata[n] = parent;
      n = parentN;

    } else {
      break;
    }
  }
}

void sys_bheap_push(SysBHeap *hp, SysPointer node) {
  sys_return_if_fail(hp != NULL);
  SysPtrArray *array = hp->array;

  sys_ptr_array_add(array, node);
  sys_bheap_bubbleup(hp, array->len - 1);
}

SysPointer sys_bheap_peek(SysBHeap *hp) {
  sys_return_val_if_fail(hp != NULL, NULL);
  SysPtrArray *array = hp->array;

  return array->len > 0 ? array->pdata[0] : NULL;
}

SysBool sys_bheap_remove(SysBHeap *hp, SysPointer node) {
  sys_return_val_if_fail(hp != NULL, false);
  sys_return_val_if_fail(node != NULL, false);

  SysPtrArray *array = hp->array;
  SysPointer end;
  SysInt len = array->len;

  for (SysInt i = 0; i < len; i++) {
    if (array->pdata[i] == node) {
      end = sys_ptr_array_steal_index(array, array->len - 1);

      if (i != len - 1) {
        array->pdata[i] = end;

        if (hp->score_func(end) < hp->score_func(node)) {
          sys_bheap_bubbleup(hp, i);

        } else {
          sys_bheap_sinkdown(hp, i);
        }
      }

      return true;
    }
  }

  sys_assert("node not found.");

  return false;
}

SysInt sys_bheap_size(SysBHeap *hp) {
  sys_return_val_if_fail(hp != NULL, -1);
  SysPtrArray *array = hp->array;

  return array->len;
}

SysPointer sys_bheap_pop(SysBHeap *hp) {
  sys_return_val_if_fail(hp != NULL, NULL);

  SysPtrArray *array = hp->array;
  SysPointer result = array->pdata[0];
  SysPointer end = sys_ptr_array_steal_index(array, array->len - 1);

  if (array->len > 0) {
    array->pdata[0] = end;
    sys_bheap_sinkdown(hp, 0);
  }

  return result;
}

SysPtrArray *sys_bheap_array(SysBHeap *hp) {
   sys_return_val_if_fail(hp != NULL, NULL);
   SysPtrArray *array = hp->array;

   return array;
}
