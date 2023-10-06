# HighConcurrentNetworkServer

 ## Feature
    1. Boardcast: Boardcast messages about newly joined client to other clients 
    2. Support Connections: up to 1024 clients
    3. Support CrossPlatform:Linux MacOS Windows
    4. Support MemoryPool for memory management
    5. Support clientDataProcessingLayer for plugin(INetEvent)

 ## Update
     1.Now Network Server won't be blocked by accept client's connection and recv data from the client
     2.introduce multithread processing unit for clients input interface
     3.fix a problem when client enter "exit" in input interface, the client program can not exit normally
     4.separate the receiving and sending thread

 ## Install
    make -j10   
     
