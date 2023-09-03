#include<HelloServer.hpp>

int main() 
{
          /*
          *Server IP Address = INADDR_ANY ; Port = 9876
          *Start Server Listening and Setup Listening Queue Number
          */
          HelloServer<_ClientSocket> server(4567);

          /*Start Server Listening and Setup Listening Queue Number = SOMAXCONN*/
          server.startServerListening();

          /*Start Server Basic Logic Function To Handle Network Request*/
          server.serverMainFunction();
}