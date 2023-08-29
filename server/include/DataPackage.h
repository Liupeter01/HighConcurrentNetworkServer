
/*------------------------------------------------------------------------------------------------------
* @enum class£ºenum PackageCommand
*------------------------------------------------------------------------------------------------------*/
enum PackageCommand
{
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
          unsigned long _packageLength;
          unsigned long _packageCmd;
};

struct _SystemData
{
          char serverName[32];
          char serverRunTime[32];
};

struct _LoginData {
          char userName[32];
          char userPassword[32];
          bool loginStatus = false;
};

struct _LogoutData {
          char userName[32];
          bool logoutStatus = false;
};