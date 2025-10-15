# OS notes
Note: not intended to be in depth - just to jog memory

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

#### Api to create
fork() (clone parent) -> wait() (so parent waits and child process may start) -> exec() (load desired data into current process)

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

##### Trap Table - how trap know which code to run
- Trap table set up at boot time
- Machine boots up in kernel mode
- OS tells hardware what code to execute (trap handler) when exceptional events occur (like a syscall)

##### How OS take control back from process
- Cooperative approach - wait for syscall
- Non-Cooperative approach - timer interrupt causes process to pause and interrupt handler in OS runs ( hardware has duty here to save state of current process)

##### Context Switch
OS saves register values of current process (kernel stack) and restores register values of scheduled process

2 Types:

- Light - timer interrupts or traps have the hardware save minimal register state so interrupt handler (kernel code that responds to interrupts) can run safely and cpu can continue execution later of same process (if scheduler decides not to intervene)

- Expensive - OS scheduler deciding to switch involves software saving full register state to switch execution (PCB) and update page tables if process switch (much more expensive) 

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
- Need thread synchronisation even with cache coherence protocols to ensure code executes as desired (2 threads reading from same data strcture will not produce expected results)

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






