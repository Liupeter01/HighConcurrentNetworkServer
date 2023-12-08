# HighConcurrentNetworkServer

 ## Feature
    1. Boardcast: Boardcast messages about newly joined client to other clients .
    2. Support Connections: up to 1024 clients.
    3. Support CrossPlatform: Linux/MacOS/Windows.
    4. Support MemoryObject pool for memory management.
    5. Support clientDataProcessingLayer for plugin(INetEvent).

 ## Update
     1.Now Network Server won't be blocked by accept client's connection and recv data from the client
     2.Introduce multithread processing unit for clients input interface
     3.Fix a problem when client enter "exit" in input interface, the client program can not exit normally
     4.Separate the receiving and sending thread
     5.Add shared_ptr for memory allocation/deallocation.
     6.Using std::move and transfer left value to right value to enhance performance.
     7.All the native pointer are replaced by generatic pointer(iterator).

 ## MemoryObjectPool Usage
    1.MemoryObjectPool Macro Headerfile: #include<HCNSMemoryAllocator.hpp>    
    2.Macro Defination:
      	1) How Many MemoryPool Blocks(per one size) will be created?
           MEMORYPOOL_BLOCK_COUNT

       2) MemoryPool Blocks Size List:
         		MEMORYPOOL_BLOCK_SZ8
         		MEMORYPOOL_BLOCK_SZ16
         		MEMORYPOOL_BLOCK_SZ32
         		MEMORYPOOL_BLOCK_SZ64
         		MEMORYPOOL_BLOCK_SZ128
         		MEMORYPOOL_BLOCK_SZ256
         		MEMORYPOOL_BLOCK_SZ512
         		MEMORYPOOL_BLOCK_SZ1024
         		MEMORYPOOL_BLOCK_SZ2048
         		MEMORYPOOL_BLOCK_SZ4096
           
    3.MemoryObjectPool Example: 
      template<typename ...MemorySpaceArgs>
      static MemoryAllocator& getInstance(uint32_t blocks_count,MemorySpaceArgs ... args);
    
 ## Install
    make -j10
