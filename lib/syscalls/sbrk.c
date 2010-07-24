#include <reent.h>
#include <errno.h>
#include <stdlib.h>
#include "lib/syscalls/heap.h"

// FIXME heap_end is not reentrant-safe I think...

/* Low-level bulk RAM allocator -- used by Newlib's Malloc */
void *heap_end = NULL;
void *_sbrk_r(struct _reent *ptr, ptrdiff_t incr)
{
	void *prev_heap_end, *next_heap_end, *ret;

	/* Initialize on first call */
	if (heap_end == NULL)
	{
		heap_end = (void *)&__start_of_heap__;
	}

	prev_heap_end = heap_end;

	/* Align to always be on 8-byte boundaries */
	next_heap_end = (void *)((((unsigned int)heap_end + incr) + 7) & ~7);  

	/* Check if this allocation would collide with the heap */
	if (next_heap_end > (void *)&__stack_min__)
	{
		ptr->_errno = ENOMEM;
		ret = NULL;
	}
	else
	{
		heap_end = next_heap_end;
		ret = (void *)prev_heap_end;
	}
	return ret;
}
