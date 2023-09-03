#include "DataPackage.h"

_PackageHeader::_PackageHeader()
          :_PackageHeader(0, CMD_UNKOWN)
{
}

_PackageHeader::_PackageHeader(unsigned long _len)
          :_PackageHeader(_len, CMD_ERROR)
{
}

_PackageHeader::_PackageHeader(
          unsigned long _len,
          unsigned long _cmd)
          :_packageLength(_len),
          _packageCmd(_cmd)
{
}

_PackageHeader::~_PackageHeader()
{
}

_LoginData::_LoginData()
          :_LoginData("Unkown User", "Unkown Password")
{
}

_LoginData::_LoginData(
          const std::string _usrName,
          const std::string _usrPassword)
          :_PackageHeader(sizeof(_LoginData), CMD_LOGIN)
{
#if _WIN32 || WIN32  //windows
          strcpy_s(this->userName, _usrName.c_str());
          strcpy_s(this->userPassword, _usrPassword.c_str());
#else                //Unix/Linux/Macos
          strcpy(this->userName, _usrName.c_str());
          strcpy(this->userPassword, _usrPassword.c_str());
#endif
}

_LoginData::~_LoginData()
{
}

_LogoutData::_LogoutData()
          : _LogoutData("Unkown User")
{
}

_LogoutData::_LogoutData(const std::string _usrName)
          : _PackageHeader(sizeof(_LogoutData), CMD_LOGOUT)
{
#if _WIN32 || WIN32  //windows
          strcpy_s(this->userName, _usrName.c_str());
#else                //Unix/Linux/Macos
          strcpy(this->userName, _usrName.c_str());
#endif
}

_LogoutData:: ~_LogoutData()
{
}

_SystemData::_SystemData()
          :_SystemData("Unkown Host", "Unkown RunTime")
{
}

_SystemData::_SystemData(
          const std::string _serverName,
          const std::string _serverRunTime)
          : _PackageHeader(sizeof(_SystemData), CMD_SYSTEM)
{
#if _WIN32 || WIN32  //windows
          strcpy_s(this->serverName, _serverName.c_str());
          strcpy_s(this->serverRunTime, _serverRunTime.c_str());
#else                //Unix/Linux/Macos
          strcpy(this->serverName, _serverName.c_str());
          strcpy(this->serverRunTime, _serverRunTime.c_str());
#endif
}

_SystemData::~_SystemData()
{
}

_BoardCast::_BoardCast()
          :_BoardCast(std::string("0.0.0.0"), 0)
{
}

_BoardCast::_BoardCast(
          const std::string _ip,
          const  unsigned short _port)
          :_PackageHeader(sizeof(_BoardCast), CMD_BOARDCAST),
          new_port(_port)
{
#if _WIN32 || WIN32  //windows
          strcpy_s(this->new_ip, _ip.c_str());
#else                //Unix/Linux/Macos
          strcpy(this->new_ip, _ip.c_str());
#endif
}

_BoardCast::~_BoardCast()
{
}