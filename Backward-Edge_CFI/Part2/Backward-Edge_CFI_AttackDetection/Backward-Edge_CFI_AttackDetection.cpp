#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include "pin.H"

PIN_LOCK Lock;

const int MAX_BUF = 1<<25;
const int NUM_BUCKETS = 5000;
const int NUM_THREADS = 100;

char* buffer;
unsigned length = 0;

typedef struct StackNode{
	ADDRINT address;
	struct StackNode *Next;
}StackNode;

typedef struct Stack{
	StackNode *Front;
}Stack;

Stack _stack;

void msg_to_buffer(const char* fmt, ...)
{
   va_list argp;
   va_start (argp, fmt);
   length += vsnprintf(buffer+length, MAX_BUF-length, fmt, argp);
}

void init_Stack(Stack *stk)
{
	stk->Front = NULL;
}

void push(ADDRINT iv, Stack *stk)
{
	StackNode *temp;
	temp = (StackNode *) malloc(sizeof(StackNode));
	temp->address = iv;
	if (stk->Front == NULL){
		stk->Front = temp;
		stk->Front->Next = NULL;
	}
	else{
		temp->Next = stk->Front;
		stk->Front = temp;
	}
}

void pop(ADDRINT *ov, Stack *stk)
{
	StackNode *temp;
	*ov = stk->Front->address;
	temp = stk->Front;
	stk->Front = stk->Front->Next;
	free(temp);
}

ADDRINT top(Stack *stk)
{
	return stk->Front->address;
}


void isCall(ADDRINT address)
{
	//PIN_GetLock(&Lock, 1);
	push(address,&_stack);
	//printf("push: adr: %p ",(void *)address);
	//PIN_ReleaseLock(&Lock);
}

void isReturn(ADDRINT  address, THREADID tid)
{	
	ADDRINT adr;
	
	if(top(&_stack) != address)
	{
		PIN_GetLock(&Lock, 1);
		PIN_ERROR("****Attack Detected, Return Address doesn't Match!****\n");
		PIN_ReleaseLock(&Lock);
	}
	else
	{
		pop(&adr, &_stack);
	}
	
}

// Pin calls this function every time a new instruction is encountered
void Instruction(INS ins, void *v)
{
    ADDRINT nextAddress;
    if(INS_IsCall(ins)){
        nextAddress = INS_NextAddress(ins);
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)isCall, IARG_ADDRINT, nextAddress, IARG_THREAD_ID, IARG_END);
    }
    if(INS_IsRet(ins)){
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)isReturn, IARG_BRANCH_TARGET_ADDR, IARG_THREAD_ID, IARG_END);
    }
}
// This function is called when the application exits
void Fini(INT32 code, void *v)
{
        
}

INT32 Usage()
{
    PIN_ERROR("This Pintool Implement Backward-Edge CFI.\n"
              + KNOB_BASE::StringKnobSummary() + "\n");
    return -1;
}

int main(int argc, char * argv[])
{
	buffer = (char *)malloc(MAX_BUF); 
	init_Stack(&_stack);

	 // Initialize pin & symbol manager
    PIN_InitSymbols();
	PIN_InitLock(&Lock);
    if (PIN_Init(argc, argv))
    {
        return Usage();
    }
	// Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);
	
    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    PIN_StartProgram();
	
    return 0;
}

