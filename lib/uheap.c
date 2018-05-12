#include <inc/lib.h>

// malloc()
//	This function use NEXT FIT strategy to allocate space in heap
//  with the given size and return void pointer to the start of the allocated space
struct Allocation{
	void* va;
	int size;
	bool Exist;
};
struct Allocation Allocations[(1+USER_HEAP_MAX-USER_HEAP_START)/PAGE_SIZE];
int AllocationIndex=0;
void * currentAddress=(void*)USER_HEAP_START;
bool IsPageTaken[(1+USER_HEAP_MAX-USER_HEAP_START)/PAGE_SIZE]={0};

//	To do this, we need to switch to the kernel, allocate the required space
//	in Page File then switch back to the user again.
//
//	We can use sys_allocateMem(uint32 virtual_address, uint32 size); which
//		switches to the kernel mode, calls allocateMem(struct Env* e, uint32 virtual_address, uint32 size) in
//		"memory_manager.c", then switch back to the user mode here
//	the allocateMem function is empty, make sure to implement it.

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

int getIndexAllocations()
{
	for(int i=0;i<(1+KERNEL_HEAP_MAX-KERNEL_HEAP_START)/PAGE_SIZE;i++)
	{
		if(Allocations[i].va==NULL)
		{
			return i;
		}
	}
	return 0;
}
void* malloc(uint32 size)
{

	//TODO: [PROJECT 2018 - MS2 - [4] User Heap] malloc() [User Side]
	// Write your code here, remove the panic and write your code
	if(sys_isUHeapPlacementStrategyNEXTFIT())
	{


		unsigned int frameCount=(size+PAGE_SIZE-1)/PAGE_SIZE;
		uint32 cn=0;
		int start=1;
		void*lastadd=NULL;
		bool ok=0;

		for(uint32 i=(uint32)currentAddress,j=0; j<USER_HEAP_MAX-USER_HEAP_START+1; i+=PAGE_SIZE,j+=PAGE_SIZE)
		{
			void* Current=(void*)i;

			if(start==0 && i==(uint32)currentAddress){
				break;
			}
			start=0;

			if(i>=USER_HEAP_MAX){
				cn=0;
				i=USER_HEAP_START;
			}
			uint32 index=((uint32)i-USER_HEAP_START)/PAGE_SIZE;
			if(IsPageTaken[index]==1){
				cn=0;
			}
			else{
				if(cn==0){
					lastadd=(void*)i;
				}
				cn++;
			}
			if(cn>=frameCount){
				currentAddress=(void*)lastadd;
				ok=1;
				break;
			}
		}
		if(ok==1)
		{

			void* ret=currentAddress;
			if(currentAddress>(void*)USER_HEAP_MAX-(frameCount*PAGE_SIZE)) //Overflow above the USER_HEAP_MAX
			{

				return NULL;
			}
			for(int i=0; i<frameCount; i++)
			{
				uint32 index=((uint32)currentAddress-USER_HEAP_START)/PAGE_SIZE;
				IsPageTaken[index]=1;
				currentAddress+=PAGE_SIZE;
			}

			sys_allocateMem((uint32)ret,size);
//			int NEWINDEX=getIndexAllocations();
			if(AllocationIndex==(1+USER_HEAP_MAX-USER_HEAP_START)/PAGE_SIZE){
				AllocationIndex=0;
			}
			while(Allocations[AllocationIndex].Exist==1){
				AllocationIndex++;
			}
			Allocations[AllocationIndex].Exist=1;
			Allocations[AllocationIndex].size=frameCount;
			Allocations[AllocationIndex].va=ret;
			AllocationIndex++;

			return ret;
		}
	}

	// Steps:
	//	1) Implement NEXT FIT strategy to search the heap for suitable space
	//		to the required allocation size (space should be on 4 KB BOUNDARY)
	//	2) if no suitable space found, return NULL
	//	 Else,
	//	3) Call sys_allocateMem to invoke the Kernel for allocation
	// 	4) Return pointer containing the virtual address of allocated space,
	//

	//This function should find the space of the required range
	// ******** ON 4KB BOUNDARY ******************* //

	//Use sys_isUHeapPlacementStrategyNEXTFIT()
	//to check the current strategy


	return 0;
}

// free():
//	This function frees the allocation of the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from page file and main memory then switch back to the user again.
//
//	We can use sys_freeMem(uint32 virtual_address, uint32 size); which
//		switches to the kernel mode, calls freeMem(struct Env* e, uint32 virtual_address, uint32 size) in
//		"memory_manager.c", then switch back to the user mode here
//	the freeMem function is empty, make sure to implement it.

void free(void* virtual_address)
{
	int index=0;
	for(int i=0; i<AllocationIndex; i++ )
		{
			if(virtual_address==Allocations[i].va)
			{
				index=i;
				break;
			}
		}
	Allocations[index].Exist=0;
	for(int i=0; i<Allocations[index].size; i++)
	{
		uint32 index=((uint32)virtual_address-USER_HEAP_START)/PAGE_SIZE;
	  IsPageTaken[index]=0;

				 virtual_address+=PAGE_SIZE;
	}
		sys_freeMem((uint32)Allocations[index].va,Allocations[index].size);

}
	//TODO: [PROJECT 2018 - MS2 - [4] User Heap] free() [User Side]
	// Write your code here, remove the panic and write your code

	//you shold get the size of the given allocation using its address
	//you need to call sys_freeMem()
	//refer to the project presentation and documentation for details



//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//

//===============
// [2] realloc():
//===============

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to malloc().
//	A call with new_size = zero is equivalent to free().

//  Hint: you may need to use the sys_moveMem(uint32 src_virtual_address, uint32 dst_virtual_address, uint32 size)
//		which switches to the kernel mode, calls moveMem(struct Env* e, uint32 src_virtual_address, uint32 dst_virtual_address, uint32 size)
//		in "memory_manager.c", then switch back to the user mode here
//	the moveMem function is empty, make sure to implement it.

void *realloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT 2018 - BONUS3] User Heap Realloc [User Side]
	// Write your code here, remove the panic and write your code
	panic("realloc() is not implemented yet...!!");

	return NULL;
}



//==================================================================================//
//============================= OTHER FUNCTIONS ====================================//
//==================================================================================//

void expand(uint32 newSize)
{
}

void shrink(uint32 newSize)
{
}

void freeHeap(void* virtual_address)
{
	return;
}


//=============
// [1] sfree():
//=============
//	This function frees the shared variable at the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from main memory then switch back to the user again.
//
//	use sys_freeSharedObject(...); which switches to the kernel mode,
//	calls freeSharedObject(...) in "shared_memory_manager.c", then switch back to the user mode here
//	the freeSharedObject() function is empty, make sure to implement it.

void sfree(void* virtual_address)
{
	//[] Free Shared Variable [User Side]
	// Write your code here, remove the panic and write your code
	//panic("sfree() is not implemented yet...!!");

	//	1) you should find the ID of the shared variable at the given address
	//	2) you need to call sys_freeSharedObject()

}

void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable)
{
	//[[6] Shared Variables: Creation] smalloc() [User Side]
	// Write your code here, remove the panic and write your code
	panic("smalloc() is not implemented yet...!!");

	// Steps:
	//	1) Implement FIRST FIT strategy to search the heap for suitable space
	//		to the required allocation size (space should be on 4 KB BOUNDARY)
	//	2) if no suitable space found, return NULL
	//	 Else,
	//	3) Call sys_createSharedObject(...) to invoke the Kernel for allocation of shared variable
	//		sys_createSharedObject(): if succeed, it returns the ID of the created variable. Else, it returns -ve
	//	4) If the Kernel successfully creates the shared variable, return its virtual address
	//	   Else, return NULL

	//This function should find the space of the required range
	// ******** ON 4KB BOUNDARY ******************* //

	//Use sys_isUHeapPlacementStrategyFIRSTFIT() to check the current strategy

	return 0;
}

void* sget(int32 ownerEnvID, char *sharedVarName)
{
	//[[6] Shared Variables: Get] sget() [User Side]
	// Write your code here, remove the panic and write your code
	panic("sget() is not implemented yet...!!");

	// Steps:
	//	1) Get the size of the shared variable (use sys_getSizeOfSharedObject())
	//	2) If not exists, return NULL
	//	3) Implement FIRST FIT strategy to search the heap for suitable space
	//		to share the variable (should be on 4 KB BOUNDARY)
	//	4) if no suitable space found, return NULL
	//	 Else,
	//	5) Call sys_getSharedObject(...) to invoke the Kernel for sharing this variable
	//		sys_getSharedObject(): if succeed, it returns the ID of the shared variable. Else, it returns -ve
	//	6) If the Kernel successfully share the variable, return its virtual address
	//	   Else, return NULL
	//

	//This function should find the space for sharing the variable
	// ******** ON 4KB BOUNDARY ******************* //

	//Use sys_isUHeapPlacementStrategyFIRSTFIT() to check the current strategy

	return NULL;
}



