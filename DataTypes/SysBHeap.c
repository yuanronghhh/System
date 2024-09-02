#include <System/DataTypes/SysHArray.h>
#include <System/DataTypes/SysBHeap.h>

/**
 * Binary heap implementation from:
 * http://eloquentjavascript.net/appendix2.html
 */

void sys_bheap_destroy(SysBHeap *hp) {
  sys_harray_destroy(&hp->array);
}

void sys_bheap_free(SysBHeap *hp, SysBool free_segment) {
  sys_return_if_fail(hp != NULL);

  sys_bheap_destroy(hp);

  sys_free(hp);
}

void sys_bheap_init(SysBHeap *hp, SysBHeapFunc func, SysDestroyFunc node_free) {
  hp->score_func = func;
  sys_harray_init_with_free_func(&hp->array, node_free);
}

SysBHeap *sys_bheap_new(SysBHeapFunc func, SysDestroyFunc node_free) {
  SysBHeap *hp = sys_new0(SysBHeap, 1);

  sys_bheap_init(hp, func, node_free);

  return hp;
}

static SysInt compare_func(const void* a, const void* b, SysPointer user_data) {
  SysBHeapFunc score_func = user_data;
  SysDouble sa = score_func((SysPointer)(*(SysUInt64 *)a));
  SysDouble sb = score_func((SysPointer)(*(SysUInt64 *)b));

  return (SysInt)(sa - sb);
}

SysBHeapIter *sys_bheap_iter_new(SysBHeap *hp) {
  sys_return_val_if_fail(hp != NULL, NULL);
  sys_return_val_if_fail(hp->array.len > 0, NULL);
  SysHArray* array = &hp->array;

  SysBHeapIter *iter = sys_malloc(sizeof(SysBHeapIter) + sizeof(SysPointer) * array->len - 1);
  *(&iter->pdata[0]) = ((SysUInt8*)iter) + offsetof(SysBHeapIter, pdata);
  iter->len = array->len;
  iter->position = 0;

  memcpy(iter->pdata, array->pdata, sizeof(SysPointer) * array->len);

  sys_qsort_with_data(iter->pdata,
    iter->len,
    sizeof(SysPointer),
    (SysCompareDataFunc)compare_func,
    hp->score_func);

  return iter;
}

SysBool sys_bheap_iter_prev(SysBHeapIter* iter, SysPointer* data) {
  sys_return_val_if_fail(iter != NULL, false);
  sys_return_val_if_fail(iter->position < iter->len, false);
  if (iter->position == 0) { return false; }

  --iter->position;
  *data = iter->pdata[iter->position];

  return true;
}

SysBool sys_bheap_iter_next(SysBHeapIter* iter, SysPointer *data) {
  sys_return_val_if_fail(iter != NULL, false);
  if ((iter->position + 1) >= iter->len) { return false; }

  ++iter->position;
  *data = iter->pdata[iter->position];

  return true;
}

void sys_bheap_iter_free(SysBHeapIter* iter) {
  sys_return_if_fail(iter != NULL);
  sys_return_if_fail(iter->len > 0);
  sys_return_if_fail(iter->position < iter->len);

  sys_free(iter);
}

static void sys_bheap_sinkdown(SysBHeap *hp, SysInt n) {
  sys_return_if_fail(hp != NULL);
  sys_return_if_fail(n >= 0);

  SysHArray *array = &hp->array;

  SysInt length = array->len;
  SysPointer node = array->pdata[n];
  SysDouble elem_score = hp->score_func(node);

  SysInt child2N;
  SysInt child1N;
  SysInt swap;

  SysPointer child1;
  SysPointer child2;
  SysDouble child2_score;
  SysDouble child1_score = 0;

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

  SysHArray *array = &hp->array;

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
  SysHArray *array = &hp->array;

  sys_harray_add(array, node);
  sys_bheap_bubbleup(hp, array->len - 1);
}

SysPointer sys_bheap_peek(SysBHeap *hp) {
  sys_return_val_if_fail(hp != NULL, NULL);
  SysHArray *array = &hp->array;

  return array->len > 0 ? array->pdata[0] : NULL;
}

SysBool sys_bheap_remove(SysBHeap *hp, SysPointer node) {
  sys_return_val_if_fail(hp != NULL, false);
  sys_return_val_if_fail(node != NULL, false);

  SysHArray *array = &hp->array;
  SysPointer end;
  SysInt len = array->len;

  for (SysInt i = 0; i < len; i++) {
    if (array->pdata[i] == node) {
      end = sys_harray_steal_index(array, array->len - 1);

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
  SysHArray *array = &hp->array;

  return array->len;
}

SysPointer sys_bheap_pop(SysBHeap *hp) {
  sys_return_val_if_fail(hp != NULL, NULL);

  SysHArray *array = &hp->array;
  if (array->len == 0) { return NULL; }

  SysPointer result = array->pdata[0];
  SysPointer end = sys_harray_steal_index(array, array->len - 1);

  array->pdata[0] = end;
  sys_bheap_sinkdown(hp, 0);

  return result;
}

SysHArray *sys_bheap_array(SysBHeap *hp) {
   sys_return_val_if_fail(hp != NULL, NULL);
   SysHArray *array = &hp->array;

   return array;
}

static SysInt bheap_index_node(SysBHeap *hp, SysPointer node) {
  SysHArray *array = &hp->array;

  for(SysUInt i = 0; i < array->len; i++) {
    if (array->pdata[i] == node) {
      return i;
    }
  }

  return -1;
}

SysPointer sys_bheap_parent(SysBHeap *hp, SysPointer node) {
  SysInt index = bheap_index_node(hp, node);
  if (index == -1) { return NULL; }
  SysInt pindex = (SysInt)floor((index - 1) / 2.0);

  if(pindex < 0) {
    return NULL;
  }

  return hp->array.pdata[index];
}
