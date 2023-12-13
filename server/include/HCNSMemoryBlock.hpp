#pragma once
#ifndef _HCNSMEMBLOCK_H_
#define _HCNSMEMBLOCK_H_
#include<iostream>

/*declare memory pool here*/
class MemoryPool;

/*------------------------------------------------------------------------------------------------------
* MemoryBlock
* 4B memory alignment
*------------------------------------------------------------------------------------------------------*/
#pragma pack(push)
#pragma pack(4)
struct MemoryBlock
{
		  MemoryBlock()
		  		:MemoryBlock(0, 0, nullptr, nullptr, false)
		  {
		  }

		  MemoryBlock(uint32_t _id, uint32_t _ref, MemoryPool* _pool, MemoryBlock* _next,bool _bstatus)
		            :_blockID(_id),
				     _blockRefCounter(_ref),
				     _belongstoPool(_pool),
				     _nextBlock(_next),
				     _blockStatus(_bstatus)
		 {
		 }

		  virtual ~MemoryBlock() = default;

		  uint32_t 		_blockID;			//Current Memblock Debug ID
		  uint32_t 		_blockRefCounter;	//Current Memblock Reference Counter
		  MemoryPool* 	_belongstoPool;		//Current Memblock belongs to which MemoryPool
		  MemoryBlock* 	_nextBlock;			//Find the next Memblock for next allocation
		  bool 			_blockStatus;		//Current Memblock inside a MemoryPool(this->_belongstoPool)
};
#pragma pack(pop)
#endif