#include <stdio.h>
#include "kernel.h"
#include "main.h"

uint32_t* last_stack_add;
uint32_t stack_counter = 0;
thread number_of_arrays[STACK_POOL];
int current_thread_index;
int default_timeout_val;



void SVC_Handler_Main( unsigned int *svc_args )
{
	unsigned int svc_number;
	/*
	* Stack contains:
	* r0, r1, r2, r3, r12, r14, the return address and xPSR
	* First argument (r0) is svc_args[0]
	*/
	svc_number = ( ( char * )svc_args[ 6 ] )[ -2 ] ;
	switch( svc_number )
	{
		case RUN_FIRST_THREAD:
			__set_PSP((uint32_t)stackptr);
			runFirstThread();
			break;
		case YIELD:
			// Pend an interrupt to do the context switch
			_ICSR |= 1 << 28;
			__asm("isb");
			break;
		default: /* unknown SVC */
			break;
	}
}

uint32_t* allocate_stack_memory(void) {
    last_stack_add -= STACK_SIZE;
    stack_counter++;

    if (stack_counter < STACK_POOL) {
        return last_stack_add;
    } else {
        return NULL;
    }
}

void osKernelInitalize(){
	last_stack_add = *(uint32_t**)0x0;
	current_thread_index = -1;
	default_timeout_val = 5;
	//set the priority of PendSV to almost the weakest
	SHPR3 |= 0xFE << 16; //shift the constant 0xFE 16 bits to set PendSV priority
	SHPR2 |= 0xFDU << 24; //Set the priority of SVC higher than PendSV
}


bool osCreateThread(void (*thread_function)(void*), void* args) {
    stackptr = allocate_stack_memory();

    if (stackptr != NULL) {
        *(--stackptr) = 1 << 24;  // xPSR register
        *(--stackptr) = (uint32_t)thread_function;  // Function address

        for (int i = 0; i < 5; i++) {
            *(--stackptr) = 0xA;  // Arbitrary values
        }
        *(--stackptr) = (uint32_t)args;
        for (int i = 0; i < 8; i++) {
			*(--stackptr) = 0xA;  // Arbitrary values
		}

        current_thread_index++;
        number_of_arrays[current_thread_index].deadline = default_timeout_val;
		number_of_arrays[current_thread_index].runtime = default_timeout_val;
        number_of_arrays[current_thread_index].sp = stackptr;
        number_of_arrays[current_thread_index].thread_function = thread_function;

        return true;
    } else {
        printf("Stack Limit Reached");
        return false;
    }

}

bool osThreadCreateWithDeadline(void (*thread_function)(void*), void* args, int deadline) {
    stackptr = allocate_stack_memory();

    if (stackptr != NULL) {
        *(--stackptr) = 1 << 24;  // xPSR register
        *(--stackptr) = (uint32_t)thread_function;  // Function address

        for (int i = 0; i < 5; i++) {
            *(--stackptr) = 0xA;  // Arbitrary values
        }
        *(--stackptr) = (uint32_t)args;
        for (int i = 0; i < 8; i++) {
            *(--stackptr) = 0xA;  // Arbitrary values
        }

        current_thread_index++;
        number_of_arrays[current_thread_index].runtime = deadline;
        number_of_arrays[current_thread_index].deadline = deadline;
        number_of_arrays[current_thread_index].sp = stackptr;
        number_of_arrays[current_thread_index].thread_function = thread_function;

        return true;
    } else {
        printf("Stack Limit Reached");
        return false;
    }
}

void osKernelStart(){
	__asm("SVC #3");
}

void osSched() {
    // Save the stack pointer of the current thread
	number_of_arrays[current_thread_index].sp = (uint32_t*)(__get_PSP() - 8 * 4);

    // Change the current thread to the next thread
	current_thread_index = (++current_thread_index) % stack_counter;

    // Set PSP to the new current thread's stack pointer
    __set_PSP((uint32_t)(number_of_arrays[current_thread_index].sp));
}





void osYield(void)
{
    // Trigger the system call for context switching
    __asm("SVC #4");
}
