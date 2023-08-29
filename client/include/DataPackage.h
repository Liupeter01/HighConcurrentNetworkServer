#pragma once
#include<string>

/*------------------------------------------------------------------------------------------------------
* @enum class£ºenum PackageCommand
*------------------------------------------------------------------------------------------------------*/
enum PackageCommand
{
          CMD_UNKOWN,              //package still in init status
          CMD_LOGIN,                    //login command
          CMD_LOGOUT,                 //login logout
          CMD_SYSTEM,                   //acquire server system info
          CMD_ERROR
};

/*------------------------------------------------------------------------------------------------------
* @struct£º_PackageHeader
* @function: define the length of the package and pass the command to server
*------------------------------------------------------------------------------------------------------*/
struct _PackageHeader
{
          _PackageHeader();
          _PackageHeader(unsigned long _len);
          _PackageHeader(
                    unsigned long _len, 
                    unsigned long _cmd
          );
          virtual ~_PackageHeader();
          unsigned long _packageLength;
          unsigned long _packageCmd;
};

struct _LoginData :public _PackageHeader
{
          _LoginData();
          _LoginData(
                    const std::string _usrName, 
                    const std::string _usrPassword
          );
          virtual ~_LoginData();

public:
          char userName[32]{ 0 };
          char userPassword[32]{ 0 };

          bool loginStatus = false;
};

struct _LogoutData :public _PackageHeader
{
          _LogoutData();
          _LogoutData(const std::string _usrName);
          virtual ~_LogoutData();

public:
          char userName[32]{ 0 };
          bool logoutStatus = false;
};

struct _SystemData :public _PackageHeader
{
          _SystemData();
          _SystemData(
                    const std::string _serverName,
                    const std::string _serverRunTime
          );
          virtual ~_SystemData();

public:
          char serverName[32]{ 0 };
          char serverRunTime[32]{ 0 };
};