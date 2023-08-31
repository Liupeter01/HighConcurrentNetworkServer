#include "DataPackage.h"

_PackageHeader::_PackageHeader()
          :_PackageHeader(0, 0)
{
}

_PackageHeader::_PackageHeader(unsigned long _len)
          :_PackageHeader(_len, CMD_UNKOWN)
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
          :_PackageHeader(sizeof(_LoginData), CMD_LOGIN)
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
          : _PackageHeader(sizeof(_LogoutData), CMD_LOGOUT)
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
          : _PackageHeader(sizeof(_SystemData), CMD_SYSTEM)
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