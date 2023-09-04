#include"HelloClient.h"

int main() 
{
          /*Create Client Socket*/
          //HelloClient client;

          /*Connect To Server IP Address = 127.0.0.1 ; Port = 4567*/
          //client.connectServer(inet_addr("127.0.0.1"), 4567);

          /*Start Client Basic Logic Function To Handle Network Request*/
          //client.clientMainFunction();

          /*Create Client Socket*/
          HelloClient*  clientPool(new HelloClient[64]);
          std::vector<std::thread> threadPool;

          for (int i = 0; i < 64; ++i) {
                    clientPool[i].connectServer(inet_addr("127.0.0.1"), 4567);
                    threadPool.emplace_back(&HelloClient::clientMainFunction, &clientPool[i]);
          }
          for (int i = 0; i < 64; ++i) {
                    threadPool[i].join();
          }
          delete[]clientPool;
          return 0;
}