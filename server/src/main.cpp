#include<HCNSTcpServer.hpp>

int main() 
{
          /*
          *Server IP Address = INADDR_ANY ; Port = 4567
          *Start Server Listening and Setup Listening Queue Number
          * Timeout Setting 5min=300000ms
          */
          HCNSTcpServer<_ClientSocket> server(4567, 300000);

          /*Start Server Basic Logic Function To Handle Network Request*/
          server.serverMainFunction(4);

          return 0;
}