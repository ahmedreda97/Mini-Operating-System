#include <inc/memlayout.h>
#include <kern/kheap.h>
#include <kern/memory_manager.h>

//NOTE: All kernel heap allocations are multiples of PAGE_SIZE (4KB)
struct Allocation{
	void* va;
	int size;
	int Exist;

};
int Index=0;
struct Allocation Allocations[(1+KERNEL_HEAP_MAX-KERNEL_HEAP_START)/PAGE_SIZE];
uint32 PaToVa[(1+KERNEL_HEAP_MAX-KERNEL_HEAP_START)/PAGE_SIZE]={0};
void * currentAddress=(void*)KERNEL_HEAP_START;
int errorr;
bool first=1;

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
void* kmalloc(unsigned int size)
{
	if(first){
		for(uint32 i=0;i<(1+KERNEL_HEAP_MAX-KERNEL_HEAP_START)/PAGE_SIZE;i++){
			Allocations[i].Exist=0;
			Allocations[i].va=NULL;
			Allocations[i].size=0;
		}
	}
	first=0;
	//TODO: [PROJECT 2018 - MS1 - [1] Kernel Heap] kmalloc()
	// Write your code here, remove the panic and write your code
	//kpanic_into_prompt("kmalloc() is not implemented yet...!!");

	//NOTE: Allocation is based on NEXT FIT strategy
	//NOTE: All kernel heap allocations are multiples of PAGE_SIZE (4KB)
	//refer to the project presentation and documentation for details
	bool ok=0;
	unsigned int frameCount=(size+PAGE_SIZE-1)/PAGE_SIZE;

	uint32* ptr_page_table=NULL; // dummy for return
	if(isKHeapPlacementStrategyNEXTFIT()){

		uint32 cn=0;
		int start=1;
		void* lastadd=NULL;

		for(uint32 i=(uint32)currentAddress,j=0; j<KERNEL_HEAP_MAX-KERNEL_HEAP_START+1; i+=PAGE_SIZE,j+=PAGE_SIZE){
			if(start==0 && i==(uint32)currentAddress){
				break;
			}
			start=0;

			if(i>=KERNEL_HEAP_MAX){
				cn=0;
				i=KERNEL_HEAP_START;
			}
			if(get_frame_info(ptr_page_directory,(void*)i,&ptr_page_table)!=0){
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
	}
	if(ok){
		void* ret=(void*)currentAddress;
		if(currentAddress>(void*)KERNEL_HEAP_MAX-(frameCount*PAGE_SIZE)) //Overflow above the kernel_heap_max
		{
			return NULL;
		}
		for(int i=0; i<frameCount; i++)
		{

			struct Frame_Info* ptrFrame;
			errorr=allocate_frame(&ptrFrame);
			if(errorr<0)
			{
				return NULL;
			}


			map_frame(ptr_page_directory,ptrFrame,currentAddress,PERM_PRESENT|PERM_WRITEABLE);
			PaToVa[kheap_physical_address((uint32)currentAddress)/PAGE_SIZE]=(uint32)currentAddress;
			currentAddress+=PAGE_SIZE;


		}
//		int NEWINDEX=getIndexAllocations();
		if(Index==(1+USER_HEAP_MAX-USER_HEAP_START)/PAGE_SIZE){
			Index=0;
		}
		while(Allocations[Index].Exist==1){
			Index++;
		}
		Allocations[Index].Exist=1;
		Allocations[Index].size=frameCount;
		Allocations[Index].va=ret;
		Index++;


		return ret;
	}

	//TODO: [PROJECT 2018 - BONUS1] Implement the BEST FIT strategy for Kernel allocation
	// Beside the NEXT FIT
	// use "isKHeapPlacementStrategyBESTFIT() ..." functions to check the current strategy

	//change this "return" according to your answer
	if(isKHeapPlacementStrategyBESTFIT()){
		uint32 cn=0;
		uint32 chk=-1,StAdd=0;
		void * BestFitAdd=NULL;
		for(uint32 i=KERNEL_HEAP_START;i<KERNEL_HEAP_MAX;i+=PAGE_SIZE){
			if(get_frame_info(ptr_page_directory,(void*)i,&ptr_page_table)!=0){
				if(cn>=frameCount){
					if(chk==-1 || chk>cn){
						chk=cn;
						BestFitAdd=(void*)StAdd;
					}
				}
				cn=0;
			}
			else{
				if(cn==0){
					StAdd=i;
				}
				cn++;
			}
		}
		if(cn>=frameCount){
			if(chk==-1 || chk>cn){
				chk=cn;
				BestFitAdd=(void*)StAdd;
			}
		}
		if(chk==-1){
			return NULL;
		}
		void* ret=(void*)BestFitAdd;
		if(BestFitAdd>(void*)KERNEL_HEAP_MAX-(frameCount*PAGE_SIZE)) //Overflow above the kernel_heap_max
		{

			return NULL;
		}
		for(int i=0; i<frameCount; i++)
		{

			struct Frame_Info* ptrFrame;
			errorr=allocate_frame(&ptrFrame);
			if(errorr<0)
			{
				return NULL;
			}


			map_frame(ptr_page_directory,ptrFrame,BestFitAdd,PERM_PRESENT|PERM_WRITEABLE);
			PaToVa[kheap_physical_address((uint32)BestFitAdd)/PAGE_SIZE]=(uint32)BestFitAdd;
			BestFitAdd+=PAGE_SIZE;
		}
		if(Index==(1+USER_HEAP_MAX-USER_HEAP_START)/PAGE_SIZE){
			Index=0;
		}
		while(Allocations[Index].Exist==1){
			Index++;
		}
		Allocations[Index].Exist=1;
		Allocations[Index].size=frameCount;
		Allocations[Index].va=ret;
		Index++;
		return ret;

	}
	return NULL;

}

void kfree(void* virtual_address)
{
	//TODO: [PROJECT 2018 - MS1 - [1] Kernel Heap] kfree()
	// Write your code here, remove the panic and write your code
	//	panic("kfree() is not implemented yet...!!");
	//you need to get the size of the given allocation using its address
	//refer to the project presentation and documentation for details

	int Position=-1;

	for(uint32 i=0; i<((1+KERNEL_HEAP_MAX-KERNEL_HEAP_START)/PAGE_SIZE); i++)
	{
		if(Allocations[i].Exist)
		{
			if(Allocations[i].va==virtual_address )
			{
				Position=i;
				break;
			}
		}
	}
	if(Position==-1)
	{
		return ;
	}
	PaToVa[kheap_physical_address((uint32)virtual_address)/PAGE_SIZE]=0;

	uint32* ptr_page_table=NULL;
	for(int i=0; i<Allocations[Position].size; i++)
	{
		free_frame(get_frame_info(ptr_page_directory,virtual_address,&ptr_page_table));
		unmap_frame(ptr_page_directory,virtual_address);
		virtual_address+=PAGE_SIZE;
		PaToVa[kheap_physical_address((uint32)virtual_address)/PAGE_SIZE]=0;


	}
	PaToVa[kheap_physical_address((uint32)virtual_address)/PAGE_SIZE]=(uint32)virtual_address;

	Allocations[Position].Exist=0;
	Allocations[Position].size=0;
	Allocations[Position].va=0;


}

unsigned int kheap_virtual_address(unsigned int physical_address)
{


	return PaToVa[physical_address/PAGE_SIZE];
}

unsigned int kheap_physical_address(unsigned int virtual_address)
{
	uint32* ptrPageTable=NULL;
	int state=get_page_table(ptr_page_directory,(void*)virtual_address,&ptrPageTable);
	if(state==TABLE_NOT_EXIST)
		return 0;
	int index=PTX(virtual_address);
	int frameNo=ptrPageTable[index]>>12;

	return frameNo*PAGE_SIZE;
}


//=================================================================================//
//============================== BONUS FUNCTION ===================================//
//=================================================================================//
// krealloc():

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to kmalloc().
//	A call with new_size = zero is equivalent to kfree().

void *krealloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT 2018 - BONUS2] Kernel Heap Realloc
	// Write your code here, remove the panic and write your code

	panic("krealloc() is not implemented yet...!!");
	return NULL;

}
