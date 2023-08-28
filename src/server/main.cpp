#define _WINDOWS
#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN
#include<Windows.h>
#include<WinSock2.h>

#pragma comment(lib,"ws2_32.lib")
#endif

class HelloSocket
{
public:
		  HelloSocket();
		  ~HelloSocket();

private:
#ifdef _WINDOWS
          WSADATA m_wsadata;
#endif // _WINDOWS 
};

HelloSocket::HelloSocket()
{
#ifdef _WINDOWS
          WSAStartup(MAKEWORD(2, 2), &m_wsadata);
#endif // _WINDOWS
}

HelloSocket::~HelloSocket()
{
#ifdef _WINDOWS
          WSACleanup();
#endif // _WINDOWS
}

int main() 
{
          HelloSocket network;

          return 0;
}