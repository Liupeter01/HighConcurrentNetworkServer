#pragma once
#ifndef _HCNSMEMBLOCK_H_
#define _HCNSMEMBLOCK_H_
#include<iostream>
#include<mutex>
#include<array>
#include<vector>
#include<initializer_list>

/*declare memory pool here*/
class MemoryPool;

/*------------------------------------------------------------------------------------------------------
* MemoryBlock
* 4B memory alignment
*------------------------------------------------------------------------------------------------------*/
#pragma pack(push)
#pragma pack(4)
class MemoryBlock
{
public:
		  MemoryBlock();
		  MemoryBlock(
					uint32_t _id, 
					uint32_t _ref, 
					MemoryPool* _pool, 
					MemoryBlock* _next,
					bool _bstatus
		  );
		  virtual ~MemoryBlock();

public:
		  void setBlockStatus(const bool status);
		  void setBlockID(const uint32_t _id);
		  void setBlockRef(const uint32_t _ref);
		  void setBlockRelatePool(MemoryPool* _pool);
		  void setNextBlock(MemoryBlock* _next);

		  MemoryBlock*& getNextBlock();
		  MemoryPool*& getMemoryPool();
		  uint32_t& getBlockRef() ;
		  bool& getBlockStatus();

private:
		  uint32_t m_nID;						  //the number of current memory block
		  uint32_t m_nRef;						  //the number of memory block reference
		  MemoryPool* m_pPool;			//which memory pool owns this memory block
		  MemoryBlock* m_pNext;			//find the next memory block
		  bool m_bStatus;						  //is the memory block inside the memory pool?
};
#pragma pack(pop)
#endif