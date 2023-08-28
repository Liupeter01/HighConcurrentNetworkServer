#include"HelloClient.h"

int main() 
{
          HelloClient client;
          client.connectServer(inet_addr("127.0.0.1"), 4567);
          char str[256]{ 0 };
          client.reciveDataFromServer(str, sizeof(str) / sizeof(char));
          std::cout << str << std::endl;
          system("pause");
          return 0;
}