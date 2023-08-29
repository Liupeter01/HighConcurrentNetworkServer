#include"HelloClient.h"

int main() 
{
          /*Create Client Socket*/
          HelloClient client;

          /*Connect To Server IP Address = 127.0.0.1 ; Port = 4567*/
          client.connectServer(inet_addr("127.0.0.1"), 4567);

          /*Start Client Basic Logic Function To Handle Network Request*/
          client.clientMainFunction();
          return 0;
}