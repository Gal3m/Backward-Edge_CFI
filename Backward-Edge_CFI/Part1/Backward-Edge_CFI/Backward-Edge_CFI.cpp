#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include "pin.H"

PIN_LOCK Lock;

const int MAX_BUF = 1<<25;
//number of hash table entry
const int NUM_BUCKETS = 5000;
//number of threads that a program may have
const int NUM_THREADS = 30;

char* buffer;
unsigned length = 0;
int numThreads = 0;

//data structure for storing stack value
typedef struct StackNode{
	ADDRINT address;
	ADDRINT calling_address;
	struct StackNode *Next;
}StackNode;

//data structure for pointing the top of stack
typedef struct Stack{
	StackNode *Front;
}Stack;

Stack _stacks[NUM_THREADS];

//data structure for storing data of hash table
typedef struct HashNode{
	ADDRINT calling_address;
	int count;
	char *function_name; 
	struct HashNode *Next;
}HashNode;

HashNode **hash_table;

//sotre all messages in the Pintool to a buffer in order to write at the end of pin tool because printf function is slow;
void msg_to_buffer(const char* fmt, ...)
{
   va_list argp;
   va_start (argp, fmt);
   length += vsnprintf(buffer+length, MAX_BUF-length, fmt, argp);
}

//calculate hash value of an address.
unsigned long hash(ADDRINT address)
{
  address = address * 2654435761 & (NUM_BUCKETS - 1);
  return address;
}
// find an address in the hash table
HashNode *HT_find_by_address(HashNode **htable, ADDRINT address)
{
	unsigned long hvalue = hash(address);
	HashNode *list = htable[hvalue];
	
	while (list != NULL) {
		if (list->calling_address == address)
			break;
		list = list->Next;
	}
	return list;
}
// insert an address and function name into hash table and update the count field if the address existed
void HT_insert_update(HashNode **htable, ADDRINT address, char *fname) 
{
	HashNode *node = HT_find_by_address(htable, address);
	
	if (node != NULL)
	{
		node->count = node->count + 1;
		return;
	}
	unsigned long hvalue = hash(address);
	HashNode *list = htable[hvalue];
	HashNode *new_node = (HashNode *)malloc(sizeof(HashNode));
	
	new_node->calling_address = address;
	new_node->function_name = (char *)malloc(strlen(fname)+1); 
	sprintf(new_node->function_name, "%s", fname);
	new_node->count = 1;

	new_node->Next = list; 
	list = new_node;
	htable[hvalue] = list;
}
// print stored value of hash table
void HT_print(HashNode **htable)
{
	printf("**************************************************************************************************************\n");
	for (int i = 0; i < NUM_BUCKETS; i++)
	{
		HashNode *list = htable[i];
		
		if (list == NULL)
			continue;
		
		while (list != NULL)
		{
			printf("Address: %p |Hit Count: %4d |Function Name: %s\n", (void *)list->calling_address, list->count, list->function_name);
			list = list->Next;
		}
	}
	printf("\n*************************************************************************--Report Generated From Hash Table--*\n");
}
void init_HT(HashNode **htable)
{
	for (int i = 0; i < NUM_BUCKETS; i++)
		htable[i] = NULL;
}

void init_Stack(Stack *stk)
{
	for(int i=0; i<NUM_THREADS;i++)
		stk[i].Front = NULL;
}

// push a value to stack with regard to Thread ID.
void push(ADDRINT iv, ADDRINT caddr, Stack *stk)
{
	StackNode *temp;
	temp = (StackNode *) malloc(sizeof(StackNode));
	temp->address = iv;
	temp->calling_address = caddr;
	if (stk->Front == NULL){
		stk->Front = temp;
		stk->Front->Next = NULL;
	}
	else{
		temp->Next = stk->Front;
		stk->Front = temp;
	}
}
//pop a value form stack with regard to Thread ID.
void pop(ADDRINT *ov, Stack *stk)
{
	StackNode *temp;
	*ov = stk->Front->address;
	temp = stk->Front;
	stk->Front = stk->Front->Next;
	free(temp);
}
// retun top of the stack with regard to Thread ID.
ADDRINT top(Stack *stk)
{
	return stk->Front->address;
}

ADDRINT top_cadr(Stack *stk)
{
	return stk->Front->calling_address;
}
// retun k top of the stack
ADDRINT topn(Stack *stk, int n)
{
    switch(n)
	{
	  case 1 : //top of stack
		return stk->Front->address;
		break;
      case 2 : //2nd value of stack
         return stk->Front->Next->address;
         break;
      case 3 : //3rd value of stack
        return stk->Front->Next->Next->address;
         break;
      case 4 : //4th value of stack
         return stk->Front->Next->Next->Next->address;
         break;
      default :
         printf("Invalid number\n");
    }
	return 0;
}
//analysis function: push the next address of call instruction to stack of desired Thread
void isCall(ADDRINT address, THREADID tid, ADDRINT target_addr)
{
	push(address, target_addr, &_stacks[tid]);
}

//analysis function: check return and call is pair or not with regard to Thread ID
void isReturn(ADDRINT  address, THREADID tid)
{	
	PIN_GetLock(&Lock, 1);
	ADDRINT adr;
	int isMatched = 0;
	char *fname;
	fname = (char *)malloc(200);
	
	if(top(&_stacks[tid]) != address)
	{
		ADDRINT cadr;
		cadr = top_cadr(&_stacks[tid]);
		strcpy(fname, RTN_FindNameByAddress(cadr).c_str());
		msg_to_buffer("mismatch at: %p, function name: %s!\n", (void *)cadr, fname);
		HT_insert_update(hash_table, cadr, fname);
		isMatched = 0;
		
		for (int i=2;i < 5;i++)
		{
			if(topn(&_stacks[tid],i) != address)
			{
				adr = topn(&_stacks[tid],i);
			}
			else
			{
				pop(&adr, &_stacks[tid]);
				isMatched = i+1;
			}
			if (isMatched > 0)
				break;
		}
	}
	else
	{
		pop(&adr, &_stacks[tid]);
	}
	if ((isMatched > 0) && (isMatched < 5))
	{ 
		for (int j=1;j <= isMatched;j++)
		{
			pop(&adr, &_stacks[tid]);
		}
	}
	free(fname);
	PIN_ReleaseLock(&Lock);
}

// Pin calls this function every time a new instruction is encountered
void Instruction(INS ins, void *v)
{
    ADDRINT nextAddress;
    if(INS_IsCall(ins))
	{
        nextAddress = INS_NextAddress(ins);
		// call target address is in the IARG_BRANCH_TARGET_ADDR
		INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)isCall, IARG_ADDRINT, nextAddress, IARG_THREAD_ID, IARG_BRANCH_TARGET_ADDR, IARG_END);
    }
    if(INS_IsRet(ins)){// return address is in the IARG_BRANCH_TARGET_ADDR
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)isReturn, IARG_BRANCH_TARGET_ADDR, IARG_THREAD_ID, IARG_END);
    }
}
// This function is called when the application exits
void Fini(INT32 code, void *v)
{
	// Write to a file since printf makes our pintool slow.
	PIN_GetLock(&Lock, 1);
	FILE *output;
    output = fopen("./result.txt", "w+");
	fprintf(output,"Number of Thread[s]: %d\n",numThreads);
	fprintf(output, "%s", buffer);
    fclose(output);
	PIN_ReleaseLock(&Lock);
	HT_print(hash_table);
	//printf("Number of Threads: %d",numThreads);
        
}
//Count number of threads
VOID ThreadStart(THREADID threadid, CONTEXT* ctxt, INT32 flags, VOID* v)
{
    numThreads++;
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
	init_Stack(_stacks);
	hash_table = (HashNode**)malloc(NUM_BUCKETS * (sizeof(HashNode *)));
	init_HT(hash_table);
	
	 // Initialize pin & symbol manager
    PIN_InitSymbols();
	
	PIN_InitLock(&Lock);
	
    if (PIN_Init(argc, argv))
    {
        return Usage();
    }
	// Register function to be called for every thread before it starts running
	PIN_AddThreadStartFunction(ThreadStart, NULL);
	 
	// Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);
	
    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    PIN_StartProgram();
	
	

    return 0;
}

