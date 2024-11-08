#include <System/DataTypes/SysAsyncQueue.h>
#include <System/Platform/Common/SysThread.h>

/**
 * this code from glib GAsyncQueue
 * see: ftp://ftp.gtk.org/pub/gtk/
 * license under GNU Lesser General Public
 */

/*
 * MT safe
 */

void sys_async_queue_init (SysAsyncQueue *queue) {
  sys_async_queue_init_full(queue, NULL);
}

void sys_async_queue_init_full (SysAsyncQueue *queue, SysDestroyFunc item_free_func) {
  sys_mutex_init (&queue->mutex);
  sys_cond_init (&queue->cond);
  sys_queue_init (&queue->queue);
  queue->waiting_threads = 0;
  queue->ref_count = 1;
  queue->item_free_func = item_free_func;
}

/**
 * sys_async_queue_new:
 *
 * Creates a new asynchronous queue.
 *
 * Returns: a new #SysAsyncQueue. Free with sys_async_queue_unref()
 */
SysAsyncQueue *
sys_async_queue_new (void)
{
  return sys_async_queue_new_full (NULL);
}

/**
 * sys_async_queue_new_full:
 * @item_free_func: (nullable): function to free queue elements
 *
 * Creates a new asynchronous queue and sets up a destroy notify
 * function that is used to free any remaining queue items when
 * the queue is destroyed after the final unref.
 *
 * Returns: a new #SysAsyncQueue. Free with sys_async_queue_unref()
 *
 * Since: 2.16
 */
SysAsyncQueue *
sys_async_queue_new_full (SysDestroyFunc item_free_func)
{
  SysAsyncQueue *queue;

  queue = sys_new (SysAsyncQueue, 1);
  sys_async_queue_init_full(queue, item_free_func);

  return queue;
}

/**
 * sys_async_queue_ref:
 * @queue: a #SysAsyncQueue
 *
 * Increases the reference count of the asynchronous @queue by 1.
 * You do not need to hold the lock to call this function.
 *
 * Returns: the @queue that was passed in (since 2.6)
 */
SysAsyncQueue *
sys_async_queue_ref (SysAsyncQueue *queue)
{
  sys_return_val_if_fail (queue, NULL);

  sys_atomic_int_inc (&queue->ref_count);

  return queue;
}

/**
 * sys_async_queue_clear_full:
 * @queue: a #SysAsyncQueue.
 *
 * Decreases the reference count of the asynchronous @queue by 1.
 *
 * If the reference count went to 0, the @queue will be destroyed
 * and the memory allocated will be freed. So you are not allowed
 * to use the @queue afterwards, as it might have disappeared.
 * You do not need to hold the lock to call this function.
 */
void sys_async_queue_clear_full(SysAsyncQueue *queue) {
  sys_return_if_fail (queue);
  sys_return_if_fail (queue->waiting_threads == 0);
  sys_mutex_clear (&queue->mutex);
  sys_cond_clear (&queue->cond);

  if (queue->item_free_func) {
    sys_queue_foreach (&(queue->queue), node) {
      queue->item_free_func(node->data);
    }
  }
  sys_queue_clear (&queue->queue);
}

void
sys_async_queue_unref (SysAsyncQueue *queue)
{
  sys_return_if_fail (queue);

  if (sys_atomic_int_dec_and_test (&queue->ref_count))
    {
      sys_return_if_fail (queue->waiting_threads == 0);

      sys_async_queue_clear_full(queue);
      sys_free (queue);
    }
}
/**
 * sys_async_queue_lock:
 * @queue: a #SysAsyncQueue
 *
 * Acquires the @queue's lock. If another thread is already
 * holding the lock, this call will block until the lock
 * becomes available.
 *
 * Call sys_async_queue_unlock() to drop the lock again.
 *
 * While holding the lock, you can only call the
 * sys_async_queue_*_unlocked() functions on @queue. Otherwise,
 * deadlock may occur.
 */
void
sys_async_queue_lock (SysAsyncQueue *queue)
{
  sys_return_if_fail (queue);

  sys_mutex_lock (&queue->mutex);
}

/**
 * sys_async_queue_unlock:
 * @queue: a #SysAsyncQueue
 *
 * Releases the queue's lock.
 *
 * Calling this function when you have not acquired
 * the with sys_async_queue_lock() leads to undefined
 * behaviour.
 */
void
sys_async_queue_unlock (SysAsyncQueue *queue)
{
  sys_return_if_fail (queue);

  sys_mutex_unlock (&queue->mutex);
}

/**
 * sys_async_queue_push:
 * @queue: a #SysAsyncQueue
 * @data: (not nullable): data to push onto the @queue
 *
 * Pushes the @data into the @queue.
 *
 * The @data parameter must not be %NULL.
 */
void
sys_async_queue_push (SysAsyncQueue *queue,
                    SysPointer     data)
{
  sys_return_if_fail (queue);
  sys_return_if_fail (data);

  sys_mutex_lock (&queue->mutex);
  sys_async_queue_push_unlocked (queue, data);
  sys_mutex_unlock (&queue->mutex);
}

/**
 * sys_async_queue_push_unlocked:
 * @queue: a #SysAsyncQueue
 * @data: (not nullable): data to push onto the @queue
 *
 * Pushes the @data into the @queue.
 *
 * The @data parameter must not be %NULL.
 *
 * This function must be called while holding the @queue's lock.
 */
void
sys_async_queue_push_unlocked (SysAsyncQueue *queue,
                             SysPointer     data)
{
  sys_return_if_fail (queue);
  sys_return_if_fail (data);

  sys_queue_push_head (&queue->queue, data);
  if (queue->waiting_threads > 0)
    sys_cond_signal (&queue->cond);
}

static SysPointer
sys_async_queue_pop_intern_unlocked (SysAsyncQueue *queue,
                                   SysBool     wait,
                                   SysInt64       end_time)
{
  SysPointer retval;

  if (!sys_queue_peek_tail_link (&queue->queue) && wait)
    {
      queue->waiting_threads++;
      while (!sys_queue_peek_tail_link (&queue->queue))
      {
        if (end_time == -1)
          sys_cond_wait (&queue->cond, &queue->mutex);
        else
        {
          if (!sys_cond_wait_until (&queue->cond, &queue->mutex, end_time))
            break;
        }
      }
      queue->waiting_threads--;
    }

  retval = sys_queue_pop_tail (&queue->queue);

  sys_assert (retval || !wait || end_time > 0);

  return retval;
}

/**
 * sys_async_queue_pop:
 * @queue: a #SysAsyncQueue
 *
 * Pops data from the @queue. If @queue is empty, this function
 * blocks until data becomes available.
 *
 * Returns: data from the queue
 */
SysPointer
sys_async_queue_pop (SysAsyncQueue *queue)
{
  SysPointer retval;

  sys_return_val_if_fail (queue, NULL);

  sys_mutex_lock (&queue->mutex);
  retval = sys_async_queue_pop_intern_unlocked (queue, true, -1);
  sys_mutex_unlock (&queue->mutex);

  return retval;
}

/**
 * sys_async_queue_pop_unlocked:
 * @queue: a #SysAsyncQueue
 *
 * Pops data from the @queue. If @queue is empty, this function
 * blocks until data becomes available.
 *
 * This function must be called while holding the @queue's lock.
 *
 * Returns: data from the queue.
 */
SysPointer
sys_async_queue_pop_unlocked (SysAsyncQueue *queue)
{
  sys_return_val_if_fail (queue, NULL);

  return sys_async_queue_pop_intern_unlocked (queue, true, -1);
}

/**
 * sys_async_queue_try_pop:
 * @queue: a #SysAsyncQueue
 *
 * Tries to pop data from the @queue. If no data is available,
 * %NULL is returned.
 *
 * Returns: (nullable): data from the queue or %NULL, when no data is
 *   available immediately.
 */
SysPointer
sys_async_queue_try_pop (SysAsyncQueue *queue)
{
  SysPointer retval;

  sys_return_val_if_fail (queue, NULL);

  sys_mutex_lock (&queue->mutex);
  retval = sys_async_queue_pop_intern_unlocked (queue, false, -1);
  sys_mutex_unlock (&queue->mutex);

  return retval;
}

/**
 * sys_async_queue_try_pop_unlocked:
 * @queue: a #SysAsyncQueue
 *
 * Tries to pop data from the @queue. If no data is available,
 * %NULL is returned.
 *
 * This function must be called while holding the @queue's lock.
 *
 * Returns: (nullable): data from the queue or %NULL, when no data is
 *   available immediately.
 */
SysPointer
sys_async_queue_try_pop_unlocked (SysAsyncQueue *queue)
{
  sys_return_val_if_fail (queue, NULL);

  return sys_async_queue_pop_intern_unlocked (queue, false, -1);
}

/**
 * sys_async_queue_timeout_pop:
 * @queue: a #SysAsyncQueue
 * @timeout: the number of microseconds to wait
 *
 * Pops data from the @queue. If the queue is empty, blocks for
 * @timeout microseconds, or until data becomes available.
 *
 * If no data is received before the timeout, %NULL is returned.
 *
 * Returns: (nullable): data from the queue or %NULL, when no data is
 *   received before the timeout.
 */
SysPointer
sys_async_queue_timeout_pop (SysAsyncQueue *queue,
			   SysUInt64      timeout)
{
  SysInt64 end_time = sys_get_monotonic_time () + timeout;
  SysPointer retval;

  sys_return_val_if_fail (queue != NULL, NULL);

  sys_mutex_lock (&queue->mutex);
  retval = sys_async_queue_pop_intern_unlocked (queue, true, end_time);
  sys_mutex_unlock (&queue->mutex);

  return retval;
}

/**
 * sys_async_queue_timeout_pop_unlocked:
 * @queue: a #SysAsyncQueue
 * @timeout: the number of microseconds to wait
 *
 * Pops data from the @queue. If the queue is empty, blocks for
 * @timeout microseconds, or until data becomes available.
 *
 * If no data is received before the timeout, %NULL is returned.
 *
 * This function must be called while holding the @queue's lock.
 *
 * Returns: (nullable): data from the queue or %NULL, when no data is
 *   received before the timeout.
 */
SysPointer
sys_async_queue_timeout_pop_unlocked (SysAsyncQueue *queue,
				    SysUInt64      timeout)
{
  SysInt64 end_time = sys_get_monotonic_time () + timeout;

  sys_return_val_if_fail (queue != NULL, NULL);

  return sys_async_queue_pop_intern_unlocked (queue, true, end_time);
}

/**
 * sys_async_queue_length:
 * @queue: a #SysAsyncQueue.
 *
 * Returns the length of the queue.
 *
 * Actually this function returns the number of data items in
 * the queue minus the number of waiting threads, so a negative
 * value means waiting threads, and a positive value means available
 * entries in the @queue. A return value of 0 could mean n entries
 * in the queue and n threads waiting. This can happen due to locking
 * of the queue or due to scheduling.
 *
 * Returns: the length of the @queue
 */
SysInt
sys_async_queue_length (SysAsyncQueue *queue)
{
  SysInt retval;

  sys_return_val_if_fail (queue, 0);

  sys_mutex_lock (&queue->mutex);
  retval = queue->queue.length - queue->waiting_threads;
  sys_mutex_unlock (&queue->mutex);

  return retval;
}

/**
 * sys_async_queue_length_unlocked:
 * @queue: a #SysAsyncQueue
 *
 * Returns the length of the queue.
 *
 * Actually this function returns the number of data items in
 * the queue minus the number of waiting threads, so a negative
 * value means waiting threads, and a positive value means available
 * entries in the @queue. A return value of 0 could mean n entries
 * in the queue and n threads waiting. This can happen due to locking
 * of the queue or due to scheduling.
 *
 * This function must be called while holding the @queue's lock.
 *
 * Returns: the length of the @queue.
 */
SysInt
sys_async_queue_length_unlocked (SysAsyncQueue *queue)
{
  sys_return_val_if_fail (queue, 0);

  return queue->queue.length - queue->waiting_threads;
}

/**
 * sys_async_queue_remove:
 * @queue: a #SysAsyncQueue
 * @item: (not nullable): the data to remove from the @queue
 *
 * Remove an item from the queue.
 *
 * Returns: %true if the item was removed
 *
 * Since: 2.46
 */
SysBool
sys_async_queue_remove (SysAsyncQueue *queue,
                      SysPointer     item)
{
  SysBool ret;

  sys_return_val_if_fail (queue != NULL, false);
  sys_return_val_if_fail (item != NULL, false);

  sys_mutex_lock (&queue->mutex);
  ret = sys_async_queue_remove_unlocked (queue, item);
  sys_mutex_unlock (&queue->mutex);

  return ret;
}

/**
 * sys_async_queue_remove_unlocked:
 * @queue: a #SysAsyncQueue
 * @item: the data to remove from the @queue
 *
 * Remove an item from the queue.
 *
 * This function must be called while holding the @queue's lock.
 *
 * Returns: %true if the item was removed
 *
 * Since: 2.46
 */
SysBool
sys_async_queue_remove_unlocked (SysAsyncQueue *queue,
                               SysPointer     item)
{
  sys_return_val_if_fail (queue != NULL, false);
  sys_return_val_if_fail (item != NULL, false);

  return sys_queue_remove (&queue->queue, item);
}

/**
 * sys_async_queue_push_front:
 * @queue: a #SysAsyncQueue
 * @item: (not nullable): data to push into the @queue
 *
 * Pushes the @item into the @queue. @item must not be %NULL.
 * In contrast to sys_async_queue_push(), this function
 * pushes the new item ahead of the items already in the queue,
 * so that it will be the next one to be popped off the queue.
 *
 * Since: 2.46
 */
void
sys_async_queue_push_front (SysAsyncQueue *queue,
                          SysPointer     item)
{
  sys_return_if_fail (queue != NULL);
  sys_return_if_fail (item != NULL);

  sys_mutex_lock (&queue->mutex);
  sys_async_queue_push_front_unlocked (queue, item);
  sys_mutex_unlock (&queue->mutex);
}

/**
 * sys_async_queue_push_front_unlocked:
 * @queue: a #SysAsyncQueue
 * @item: (not nullable): data to push into the @queue
 *
 * Pushes the @item into the @queue. @item must not be %NULL.
 * In contrast to sys_async_queue_push_unlocked(), this function
 * pushes the new item ahead of the items already in the queue,
 * so that it will be the next one to be popped off the queue.
 *
 * This function must be called while holding the @queue's lock.
 *
 * Since: 2.46
 */
void
sys_async_queue_push_front_unlocked (SysAsyncQueue *queue,
                                   SysPointer     item)
{
  sys_return_if_fail (queue != NULL);
  sys_return_if_fail (item != NULL);

  sys_queue_push_tail (&queue->queue, item);
  if (queue->waiting_threads > 0)
    sys_cond_signal (&queue->cond);
}
