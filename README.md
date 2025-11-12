# OS notes

Note: concise reference notes to jog memory.

---

## Contents

- [Virtualization](#virtualization)
  - [CPU](#cpu)
    - [Process](#process)
      - [Creation](#creation)
      - [States](#states)
      - [API to create](#api-to-create)
    - [Limited direct execution](#limited-direct-execution)
      - [Traps](#traps)
      - [Syscall flow](#syscall-flow)
      - [Kernel stack & Trap table](#kernel-stack--trap-table)
      - [How OS regains control](#how-os-regains-control)
      - [Context switch](#context-switch)
    - [Scheduling](#scheduling)
      - [Common algorithms](#common-algorithms)
      - [Multiprocessor scheduling](#multiprocessor-scheduling)
        - [Problems & solutions](#problems--solutions)
- [Memory](#memory)

---

# Virtualization

## CPU

### Process
#### Creation
 - Code and static data from disk loaded into memory
 - Some memory allocated to stack
 - OS may also create initial memory for heap
 - I/O setup
 - start program running at entrypoint (main())

#### States
Ready || Running || Blocked

#### API to create
- fork() — duplicate process.
- exec() — replace current process image.
- wait() — parent waits for child termination.

### Limited direct execution
Direct execution fast but gives process complete control over system thus we have limited direct execution
- Kernel mode
- User mode

#### Traps
- Intentional -> syscall
- Unintentional -> due to error (exceptions like divide by 0, segfault, overflow)

#### Syscall
1. User program executes trap instruction
2. Instruction jumps into kernel, simultaneously raises priviledge to kernel mode
3. Do privileged instruction
4. When finished, OS calls special return-from-trap instruction
5. Returns into calling user program and simultaneously return to user mode

To return correctly, processor will push program counter, flags, other registers onto a per process _kernel stack_; return-from-trap pops from this stack

> **💡 Note:**  kernel stack
> _kernel stack_ - each process has user stack (for user mode) and kernel stack (for  kernel mode) (uses same address space as kernel region of virtual memory is shared  among all processes (e.g top half of address space on x86))

##### Trap Table - how trap know which code to run
- Trap table set up at boot time
- Machine boots up in kernel mode
- OS tells hardware what code to execute (trap handler) when exceptional events occur (like a syscall)

##### How OS take control back from process
- Cooperative approach - wait for syscall
- Non-Cooperative approach - timer interrupt causes process to pause and interrupt handler in OS runs (hardware has duty here to save state of current process)

##### Context Switch
OS saves register values of current process (kernel stack) and restores register values of scheduled process

2 Types:

- Light - timer interrupts or traps have the hardware save minimal register state so interrupt handler (kernel code that responds to interrupts) can run safely and cpu can continue execution later of same process (if scheduler decides not to intervene)

- Expensive - OS scheduler deciding to switch, this involves software saving full register state to switch execution (PCB) and update page tables if process switch (much more expensive) 

### Scheduling
T_turnaround = T_completion - T_arrival

T_response = T_firstrun - T_arrival

Higher fairness schedulers yield good response times but bad turnaround times

- FIFO/FCFS
   - run jobs in simple queue
   - bad turnaround time (convoy problem)
- SJF (shortest job first)
   - run shortest job first (assuming cpu scheduler knows how long process takes lol)
   - bad turnaround time (convoy problem)
- STCF (shortest time to completion)
   - always run job that will end first fast (assuming cpu scheduler knows how long process takes lol)
   - bad response time
- RR 
   - run a job for time slice (quanta), then context switch to next in queue
   - good response time
   - bad turnaround time
- MLFQ (multi level feedback queue)
   - multiple queues with high to low priorities
   - New jobs have highest possible priority (topmost queue)
   - Once job uses up time allotment at given level, it goes to a lower priority queue
   - After some period S, all jobs jump back to highest queue (max priority)
   - priortiy(A) > priority(B), A runs and B doesn't
   - priortiy(A) == priority(B), A and B run in RR
   - idea is jobs that require more interaction (I/O-bound or short jobs) remain at higher priority levels longer than CPU-bound or long-running jobs. This allows short or interactive tasks to finish quickly, while longer, less interactive jobs gradually move to lower-priority queues.
   - it does not demand prior knowledge of job and instead prioritises jobs based on its behaviour at runtime
- PR (proportional share/fair share)
  - does not optimise for response or turnaround time
  - ensures each job gets % of cpu time
  - implemented using lottery scheduling (each job gets tickets proportional to its share; scheduler randomly picks a ticket to decide who runs) or stride scheduling (each job has a stride inversely proportional to its share; smallest stride runs next)
  - fair over time - every job gets CPU time proportional to its weight/share
  - can be adjusted dynamically to give more CPU to higher-priority or more important jobs
  - identifying ticket assignments is tricky (how many tickets should browser get??)

#### Multiprocess Scheduling - Multiple cpus more problems

##### problem 1
- We can write some code that runs on 1 cpu, or leverage threads so that work can be distributed across cpus
- Now data is shared across multiple processors each with their own cache
- l1 faster than l2 faster than l3 faster than main memory faster than disk; cpu caches leverage two types:
  - temporal locality: when data accessed, likely to be accessed again in near future
  - spatial locality: when data accessed, neighbour data likely to be accessed in near future
- problem? we don't have cache coherence - different cpu caches could have different data when they should be same as cpu updates data in cache first because its faster than writing all the way through to main memory
- one solution? bus snooping - cache pays attention to memory updates by observing bus that connects them to main memory
- MESI protocol (used to maintain cache coherence):
  - M (Modified): cache line has been changed and is different from main memory; this CPU “owns” the most recent copy
  - E (Exclusive): cache line is the same as main memory and is only present in this cache
  - S (Shared): cache line is the same as main memory and may be in multiple caches
  - I (Invalid): cache line is invalid (stale) — must be reloaded before use
- When one CPU modifies a cache line in M state, other CPUs’ copies are invalidated (set to I) to maintain consistency

##### problem 2
- Need thread synchronisation even with cache coherence protocols to ensure code executes as desired (2 threads reading and writing to same data strcture will not produce expected results - MESI doesn’t track the data loaded into CPU registers)

##### problem 3
- Cache affinity - when process runs on cpu, a lot of its state saved in cache and tlb thus we would want the same process to be rescheduled on same cpu instead of having to completely flush cache

##### scheduling on multicores
- SQMS (single queue multiprocessor scheduling)
  - all jobs in single queue, each job runs for a time slice then context switch
  - lack of scalability (needs locks to pull correct job scheduling from queue which makes scheduling slow)
  - no cache affinity - jobs bounce from cpu to cpu  
- MQMS (multi queue multiprocessor scheduling)
  - new job placed on a queue (ie could be queue with least jobs)
  - each cpu has its own queue ( better for cache affinity)
  - no need for locking as each cpu has its own queue
  - could end up with 2 jobs on cpu a and 1 job on cpu b so job on cpu b finishes quickly and hence nothing in queue for cpu b (its idle!)
  - solution? work stealing - queue low on jobs will peak at busy queue and steal to help balance load allowing for job migration (this could suffer from high overhead tho if queues are always checking to see if they can work steal)
- linux used in past: O(1) scheduler (similar to MLFQ), CFS (completely fair scheduler - similar to stride scheduling), BFS scheduler (single queue approach)


## Memory

Multiprogramming was created where multiple processes run concurrently, switching out registers is quick but switching out entire data for different processes in memory is inefficient. Solution - keep data in memory.

### Address space
Address space is an abstraction provided to each process by OS
Address space of process contains:
- code
- variables
- stack
- heap

#### goals
1. transparency - program shouldn't be aware its memory is virtualised, program behaves as though it has its own private physical memory, program doesn't know its memory addresses being translated
2. efficiency - virtualisation should be efficient in time and space
3. protection - processes memory protected from one another (helps isolation)

### Memory API

stack - allocations and deallocations implicitly handled by compiler (also called automatic memory)
heap - allocations and deallocations handled by programmer

1. malloc() - allocate to heap by passig size. Succeed? Returns pointer of type void to first byte in memory block. Fail? Returns NULL. Built on top of `brk` and `sbrk` syscall swhich change location of end of heap. `mmap` could also be used to build heap.
2. free() - takes one arg -> the pointer returned from malloc

> **💡 Note:**  Errors
> - forget to allocate memory -> likely lead to seg fault (trynna do something illegal with memory)
> - not allocate enough memory - lead to buffer overlfow
> - forget to initiaslise allocated memory - could lead to unitilaised read -> if something dodgy in there like not a 0 could break your program if it reads from it 
> - forget to free memory - memory leak 
> - free memory before done with it - dangling pointer (subsequent use can crash process or overwrite valid memory)
> - free memory repeatedly - double free - causes undefined behaviour (might be a crash)
> - using free incorrectly - not passing correct value in


### Address translation
Hardware transforms each memory access (eg. fetch, load, store) - change virtual address to physical address

mmu - cpu hardware that handles address translation
dynamic reloaction - can move address space even after program running

#### base and bound registers (old technique before paging)
Defines base and bound pair of memory region
Stored in PCB
pros: protects each process, transparent to process
cons: not space efficient - leads to internal fragmentation (wasted space inside of allocated unit)

#### base and bound registers + segmentation (old technique before paging)
Defines base and bound pair per logical segment of address space (code, stack, heap etc)

introduced things still in use:
code sharing - share memory segments between address spaces
protection bits - bits which identify if program can read, write, execute code that lies in segment

pros: better support for sparse address spaces (less internal fragmentation), code sharing
cons: external fragmentation ( we have variable sized segments in free list so satisfying memory allocations is hard. Solution? use compaction - stop existing processes and move their data into single contiguous block but this is very expensive), isn't flexible enough to support fully generalised sparse address space (we still have some internal fragmentation - ie we can have a large sparsely used heap as allocator requests block sizes and memory inside gets freed up but not completely)


### Free space management
Total free space can exceed request size but it is fragmented so can't succeed

#### free list
- linked list (or other ds) which keeps track of free blocks of memory (tracks which physical unused frames are available for allocation)
  - when malloc() called, allocator searches free list for block that is big enough.
  - kernel also also has free list for tracking memory blocks across entire system (ie for new processes page)
- The allocator allocates space for a header with metadata when allocating memory, ie when calling free(), the size of block does not need to be passed as its in the header.
- free list is built inside space itself

Splitting - Say we need 30 bytes. The allocator finds a 100-byte free block in free list, allocates 30 bytes, and splits the remainder (70 bytes) into a new free block.
Coalescing - Now suppose the allocated 30-byte block is freed. We now have two adjacent free blocks (30 + 70) — the allocator coalesces them into one 100-byte block again.


#### growing the heap
- heap out of space? 
  - malloc() could fail if no alternative
  - request more memory from OS using `sbrk` -> OS finds new physical pages, maps them into address space which makes heap bigger

#### strategies to manage free space in free list
- best fit
  - return smallest block in free list that satisfies request
  - pros: simple
  - cons: performance penalty searching through full list

- worst fit
  - find largest chunk and split and keep remaining chunk on free list
  - cons: performance penalty searching through full list and also causes excess fragmentation

- first fit 
  - find first block that satisfies request
  - pros: faster as no exhaustive search 
  - cons: pollutes start of list with small objects (address-based ordering can mitigate)

- next fit
  - like first fit but instead of starting at beginning of list, we start at location where we were last 
  - pros: fast like first fit, avoids splintering start of list

- segregated list
  - have more than one list - if program makes requests for certain size, have a list just for those requests and all other requests go to more general allocator
  - pros: less fragmentation
  - cons: how much memory should one allocate to pool that serves specialised requests? `slab allocator` mitigates this as wehn memory is running low it requests more memory from more general allocator

- buddy allocator
  - one single big space - allocator divides recursively in 2 until it finds smallest size that satisfies request
  - pros: nice coalescing as can recurseively free memory
  - cons: internalk fragmentation as only power of 2 blocks 