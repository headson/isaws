#ifndef _AUTH_H
#define _AUTH_H
#include "string.h"

/*target conditions describtion*/
//#define _AUTH_UNICODE
#define LINUX_PLATFORM

#include <stdio.h>
#include "stdarg.h"

#define _auth_max_string_count 2048
#define _auth_max_line_string_count 256
#ifdef _AUTH_UNICODE
#define _auth_char wchar_t
#define _auth_snprintf _snwprintf
#define _auth_string(x) L##x
#define _auth_eof WEOF
#define _auth_strcmp   wcscmp
#define _auth_strcpy   wcscpy
#define _auth_fgetc    fgetwc
#define _auth_strlen   wcslen
#else
#define _auth_char char
#define _auth_snprintf snprintf
#define _auth_string(x) x
#define _auth_eof EOF
#define _auth_strcmp   strcmp
#define _auth_strcpy   strcpy
#define _auth_fgetc    fgetc
#define _auth_strlen   strlen
#endif
#define _C_PLATFORM
#define LOGPATH "/tmp/log.txt"
#define DWORD int
#define BOOL int
#include "wchar.h"
#define TRUE 1
#define FALSE 0
#ifdef LINUX_PLATFORM
#undef _auth_snprintf
#define _auth_snprintf snprintf
#endif
#define VARIABLELENGTH 512

typedef struct _auth_file
{
	_auth_char username[255];
	_auth_char password[255];
}_authFile,*pAuthFile;

typedef struct _s_elem
{
	_auth_char elemName[255];
	_auth_char elemValue[255];
	_auth_char separater;
	#ifndef _C_PLATFORM
	_s_elem(_auth_char* name,_auth_char* value,_auth_char sepa)
	{
		_auth_strcpy(elemName,name);
		_auth_strcpy(elemValue,value);	
		separater = sepa;
	}
	_s_elem()
	{
	
	}
	#endif
}_elemType,*pElemType;



typedef void(* pRecvHandle)(char* buffer,int* length);
typedef struct _msg
{
	char* msg;
	int msglen;
	int port;
	pRecvHandle recvhandle;
	char* buffer;
	int* length;
	int* reValue;
}__msg;


void GetValueByName(char* orignal,char* name,char* value);



//you must free memory for the return pointer
_auth_char* GetStringFromFile(FILE* file);
_auth_char* GetLineFromFile(FILE* file,_auth_char* buffer);
int GetLineFromString(_auth_char* orignal,_auth_char* buffer);
// _auth_char* GetProp_Value_From_String(_auth_char* orignal,_auth_char* propertyname,_auth_char* propertyvalue);
BOOL WriteElemToFile(FILE* file,pElemType pElem);

// _set_property_value("./test.txt", "ip", "192.168.1.11", 1);
DWORD _set_property_value(_auth_char* filepath,_auth_char* propertyname,_auth_char* propertyvalue,BOOL create);
// char buf[128];
// _get_property_value("./test.txt", "ip", buf);
BOOL _get_property_value(_auth_char* filepath,_auth_char* propertyname,_auth_char* propertyvalue);

DWORD _set_property_variable(_auth_char* filepath,_auth_char* propertyname,_auth_char* propertyvalue,...);
BOOL _delete_property_by_name(_auth_char* filepath,_auth_char* propertyname);
BOOL GetElemValue(_auth_char* orginal,_auth_char* elemname,_auth_char* value);
BOOL SetElemValue(_auth_char* orginal,_auth_char* elemname,_auth_char* value);

int SendRcvMsg(char* msg,int msglen,int port,pRecvHandle recvhandle,char* buffer,int* length);

//返回值:true,校验用户名/密码成功
BOOL CheckAuth(int pass_level,_auth_char* username,_auth_char* password,_auth_char* fathername);
//返回值:true,成功创建用户,false,用户已存在或其他错误
BOOL CreateAuth(int pass_level,_auth_char* username,_auth_char* password,_auth_char* fathername);
//返回值:TRUE,成功更改密码,false,无此用户
BOOL ChangePwd(int pass_level,_auth_char* username,_auth_char* password,_auth_char* fathername);
//返回值,TRUE ,成功获取用户名和密码；FALSE，获取失败，可能是未找到授权文件
BOOL GetAuth(int pass_level,_auth_char*username,_auth_char* password,_auth_char* fathername);
wchar_t* ctowc(wchar_t* des,char* src);
char*    wctoc(char* dst,wchar_t* src);
char* StrToMac(char* str,char* mac);
//1,fp can be null,this function will open macro LOGPATH file for writing
int WriteLog(FILE* fp,const char * format,...);
#endif
