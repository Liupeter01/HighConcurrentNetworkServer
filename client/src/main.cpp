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
          const int _ClientAmmount = 1023;
          HelloClient* clientPool[_ClientAmmount];
          _LoginData loginData("client-loopback404", "1234567abc");
          for (int i = 0; i < _ClientAmmount; ++i) {
                    clientPool[i] = new HelloClient;
                    clientPool[i]->connectServer(inet_addr("127.0.0.1"), 4567);
          }
          while (true) 
          {
                    for (int i = 0; i < _ClientAmmount; ++i)
                    {
                              clientPool[i]->sendDataToServer(clientPool[i]->getClientSocket(), &loginData, sizeof(loginData));
                    }
          }
          delete[]clientPool;
          return 0;
}