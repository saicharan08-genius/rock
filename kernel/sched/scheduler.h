#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <mm/vmm.h>
#include <asmutils.h>
#include <memutils.h>

extern void start_task(uint64_t ss, uint64_t rsp, uint64_t cs, uint64_t rip);
extern void switch_task(uint64_t rsp, uint64_t ss);

enum {
    WAITING,
    WAITING_TO_START,
    RUNNING
};

typedef struct thread {
    int tid;
    int pid;
    int idle_cnt;
    int status;
    regs_t regs;
    uint64_t kernel_stack;
    uint64_t user_stack;
    uint64_t starting_addr;
    int ks_page_cnt, us_page_cnt;
    
    struct thread *next;
    struct thread *last;
} thread_t;

typedef struct task {
    int pid, ppid, pgrp, uid;
    int thread_cnt;
    int tid_cnt;
    int file_handle_cnt;
    int idle_cnt;
    int status;
    uint16_t cs, ss;
    pagestruct_t *pagestruct;
    thread_t *threads;
    int *file_handles;

    struct task *next;
    struct task *last;
} task_t;

task_t *get_current_task();

void scheduler_init();

void scheduler_main(regs_t *regs);

int kill_task(int pid);

int create_task(uint64_t starting_addr, uint16_t cs);

int create_task_thread(int pid, uint64_t starting_addr);

int kill_thread(int pid, int tid); 

#endif
