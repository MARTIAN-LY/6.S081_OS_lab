// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

#define NLIST 4
void freerange(void *pa_start, void *pa_end);
void init_kfree(int cpu, void *pa);
inline struct run* pop_page(int cpu);
struct run* steal_page(int cur_list);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct kmem {
  struct spinlock lock;
  struct run *freelist;
};
// freelists
struct kmem kmems[NLIST];

void
kinit()
{
  // Initialize all of the locks.
  for (int i = 0; i < NLIST; i++) {
    initlock(& kmems[i].lock, "kmem");
  }
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{  
  char *p = (char*)PGROUNDUP((uint64)pa_start);
  uint64 pc_pages  = ((uint64)pa_end - (uint64)p) / ( PGSIZE * NLIST );
  
  // Allocate pages for each list.
  for (int i = 0; i < NLIST; i++)
  {
    for (uint64 j = 0; j < pc_pages; j++)
    {
      init_kfree(i,p);
      p += PGSIZE;
    }
  }
  // Give the leftover memory to list0.
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE){
    init_kfree(0, p);
  }  
}

void
init_kfree(int list, void *pa)
{
  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("init_kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  struct run* r = (struct run*)pa;

  // acquire(&kmems[cpu].lock);
  r->next = kmems[list].freelist;
  kmems[list].freelist = r;
  // release(&kmems[cpu].lock);
}

/****************************************************
 * The original kree. Now it's not used in initialing
 ****************************************************/
// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");
  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);
  struct run* r = (struct run*)pa;

  push_off();
  int i = cpuid() % NLIST;
  acquire(&kmems[i].lock);
  r->next = kmems[i].freelist;
  kmems[i].freelist = r;
  release(&kmems[i].lock);
  pop_off();
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  push_off();
  int i = cpuid() % NLIST;
  struct run* r = pop_page(i);
  if (!r) {
    r = steal_page(i);
  }
  pop_off();

  if (r) {
    memset((char*)r, 5, PGSIZE);
  }
  return (void*)r;
}

// Steal a page. Preference: 3->0
struct run*
steal_page(int cur_list)
{
  struct run* r = 0;
  for (int i = NLIST - 1; i >= 0; i--)
  {
    if (i == cur_list) continue;
    r = pop_page(i);
    if (r)  break;
  }
  return r;
}

struct run* 
pop_page(int list)
{
  acquire(&kmems[list].lock);
  struct run* r = kmems[list].freelist;
  if(r) {
    kmems[list].freelist = r->next;
  } 
  release(&kmems[list].lock);
  return r;
}
