#include<HCNSMemoryBlock.hpp>

MemoryBlock::MemoryBlock()
          :MemoryBlock(0, 0, nullptr, nullptr, false)
{
}

MemoryBlock::MemoryBlock(uint32_t _id, uint32_t _ref, MemoryPool* _pool, MemoryBlock* _next, bool _bstatus)
          :m_nID(_id),
          m_nRef(_ref),
          m_pPool(_pool),
          m_pNext(_next),
          m_bStatus(_bstatus)
{
}

MemoryBlock::~MemoryBlock()
{

}

/*------------------------------------------------------------------------------------------------------
* set memory block status, is it inside the memory pool
* @function: void setBlockStatus(const bool status)
* @param :  [IN] const bool status
*------------------------------------------------------------------------------------------------------*/
void MemoryBlock::setBlockStatus(const bool status)
{
          this->m_bStatus = status;
}

/*------------------------------------------------------------------------------------------------------
* set memory block id for debugging
* @function: void setBlockID(const uint32_t _id)
* @param :  [IN] const bool status
*------------------------------------------------------------------------------------------------------*/
void MemoryBlock::setBlockID(const uint32_t _id)
{
          this->m_nID = _id;
}

/*------------------------------------------------------------------------------------------------------
* set memory block reference detail
* @function: void setBlockRef(const uint32_t _ref)
* @param :  [IN] const bool status
*------------------------------------------------------------------------------------------------------*/
void MemoryBlock::setBlockRef(const uint32_t _ref)
{
          this->m_nRef = _ref;
}

/*------------------------------------------------------------------------------------------------------
* set memory block related memory pool
* @function: void setBlockRelatePool(MemoryPool* _pool)
* @param :  [IN] MemoryPool* _pool
*------------------------------------------------------------------------------------------------------*/
void MemoryBlock::setBlockRelatePool(MemoryPool* _pool)
{
          this->m_pPool = _pool;
}

/*------------------------------------------------------------------------------------------------------
* set memory block's next block
* @function: void setNextBlock(MemoryBlock* _next)
* @param :  [IN] MemoryBlock* _next
*------------------------------------------------------------------------------------------------------*/
void MemoryBlock::setNextBlock(MemoryBlock* _next)
{
          this->m_pNext = _next;
}

/*------------------------------------------------------------------------------------------------------
* get memory block's next block
* @function: MemoryBlock*&  getNextBlock()
* @retvalue: MemoryBlock*&
*------------------------------------------------------------------------------------------------------*/
MemoryBlock*& MemoryBlock::getNextBlock()
{
          return this->m_pNext;
}

/*------------------------------------------------------------------------------------------------------
* get which MemoryPool/MemoryAllocator does memory block's belongs to
* @function: MemoryPool*& getMemoryPool
* @retvalue: MemoryPool*&
*------------------------------------------------------------------------------------------------------*/
MemoryPool*& MemoryBlock::getMemoryPool()
{
          return this->m_pPool;
}

/*------------------------------------------------------------------------------------------------------
* get memory block ref number
* @function: uint32_t& getBlockRef
* @retvalue: uint32_t&
*------------------------------------------------------------------------------------------------------*/
uint32_t& MemoryBlock::getBlockRef()
{
          return this->m_nRef;
}

/*------------------------------------------------------------------------------------------------------
* get memory block block status
* @function: bool &getBlockStatus() 
* @retvalue: bool &
*------------------------------------------------------------------------------------------------------*/
bool& MemoryBlock::getBlockStatus()
{
          return this->m_bStatus;
}