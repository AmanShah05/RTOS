#ifndef KERNEL_H
#define KERNEL_H

#define RUN_FIRST_THREAD 0x3
#define STACK_SIZE 0x200
#define STACK_POOL 32
#define YIELD 0x4
#include <stdio.h>
#include <stdbool.h>
extern uint32_t* stackptr;

#define SHPR2 *(uint32_t*)0xE000ED1C //for setting SVC priority, bits 31-24
#define SHPR3 *(uint32_t*)0xE000ED20 // PendSV is bits 23-16
#define _ICSR *(uint32_t*)0xE000ED04 //This lets us trigger PendSV

void osKernelInitalize();
void osKernelStart();
void osSched();
void osYield(void);


extern void runFirstThread(void);
bool osCreateThread(void (*thread_function)(void*), void* args);
bool osThreadCreateWithDeadline(void (*thread_function)(void*), void* args, int deadline);


typedef struct k_thread {
	int timeslice;   // Number of milliseconds the thread is allowed to run
	int runtime; 	 // Number of milliseconds left for the thread to run
    uint32_t* sp;                     // Stack pointer
    void (*thread_function)(void*);
    // Function pointer
} thread;

typedef struct {
    int var1;
    int var2;
} thread_output_data;



#endif /* KERNEL_H */
