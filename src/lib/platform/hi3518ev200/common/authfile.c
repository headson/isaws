
#include "authfile.h"
#include <stdlib.h>

#include <errno.h>

int GetFirst(char* orignalstr,char* command,int* index,char separater)
{
	int i = 0;
	while(*(orignalstr+i) != separater&& *(orignalstr+i) != '\0')
	{
		*(command + i) = *(orignalstr+i);
		i++;
	}
	*(command + i) = '\0';
	if(index != NULL)
	{
		if(*(orignalstr+i) == '\0')
		{
			*index = -1;
			return 0;
		}
		else
		*index = i + 1;
	}
	return 1;
}

void GetValueByName(char* orignal,char* name,char* value)
{
	int i = 0;
	int index = 0;
	*value = '\0';
	char sub[512];
	char* tempori = orignal;
	while(GetFirst(tempori,sub,&index,','))
	{
		if(index == -1)//reach the end of orignal
			return;
		if(!strcmp(sub,name))
		{
			GetFirst(tempori + index,value,NULL,',');
			break;
		}
		else
		{
			tempori = tempori + index;
		}
	}
}

BOOL GetElemValue(_auth_char* orginal,_auth_char* elemname,_auth_char* value)
{
	_auth_char* temp = orginal;
	int i = 0;
	_auth_char* space = NULL;
	while(*temp ==' ')
	{
		temp++;
	}	
	while(*(temp + i) != '=' && *(temp + i) != '\0')
	{
		i++;
	}
	if(*(temp + i) == '\0')
	{
		*value = '\0';
		*elemname = '\0';
		*value = '\0';
		return FALSE;
	}
	//delete space
	space = temp + i - 1;
	while(*space == ' ')
	{
		space--;
	}
	*(++space) = '\0';	
	_auth_strcpy(elemname,temp);
	space = temp + i + 1;
	while(*space == ' ')
	{
		space++;
	}
	_auth_strcpy(value,space);
	//*value = '\0';
	return TRUE;
}

BOOL SetElemValue(_auth_char* orginal,_auth_char* elemname,_auth_char* value)
{
	if(_auth_strlen(elemname) == 0)
	{
		orginal = "";
		return TRUE;
	}
	_auth_snprintf(orginal,_auth_max_line_string_count,_auth_string("%s = %s\n"),elemname,value);
	return TRUE;
}

_auth_char* GetStringFromFile(FILE* file)
{
	int filelength = 0;
	_auth_char* filestring = NULL;
	_auth_char* tempstring = NULL;
	unsigned int usedsize = 0;
	char allocmemcount = 0;
//	char test;
	//int i;
	int character = 0;
	fseek(file,0,SEEK_END);
	filelength = ftell(file);
	filelength += 1;
	filelength = filelength < _auth_max_line_string_count?_auth_max_line_string_count:filelength;
	rewind(file);
	filestring = (_auth_char*)malloc(filelength * sizeof(_auth_char));
	memset(filestring,0,filelength * sizeof(_auth_char));
	tempstring = filestring;
	if(!filestring)
		printf("malloc error,size is %d",filelength);
//	fprintf(stderr,"getstringfromfile start\n");
	while(1)
	{
		
		if(usedsize == filelength*(1 + allocmemcount))//need more memory
		{
			fprintf(stderr,"enter need more memory!\n");
			allocmemcount++;
			tempstring = (_auth_char*)malloc(filelength * sizeof(_auth_char)*(1 + allocmemcount));
			memset(tempstring,0,filelength * sizeof(_auth_char)*(1 + allocmemcount));
			memcpy(tempstring,filestring,usedsize*sizeof(_auth_char));//copy to new allocated memory
			free(filestring);//free former memory
			filestring = tempstring;
			tempstring = filestring + usedsize;			
		}
		//fprintf(stderr,"3\n");
		character = _auth_fgetc(file);
		//*tempstring = _auth_fgetc(file);
		*tempstring = character;
		//sleep(1);
		if(character == 0xfeff)
			*tempstring = _auth_fgetc(file);
		tempstring++;
		if(character == (int)_auth_eof) break;			
		usedsize++;
	}
	//fprintf(stderr,"getstringfromfile end\n");
	//add null character to target string	
	if(*(tempstring - 2) != '\n')
	{
		*(tempstring - 1) = '\n';
		*(tempstring) = '\0';
	}
	else
	{
		*(tempstring - 1) = '\0';
	}
	//fprintf(stderr,"4\n");
	if(_auth_strlen(filestring) < 2)
		memset(filestring,0,filelength * sizeof(_auth_char));
	//fprintf(stderr,"5\n");
	return filestring;
}

_auth_char* GetLineFromFile(FILE* file,_auth_char* buffer)
{
	int i = 0;
	int temp;
	char delspace = 0;
	while(1)
	{
retry:
		temp = _auth_fgetc(file);
		if(temp == 0xfeff) 
			goto retry;
		if(temp == '\n' || temp == _auth_eof)
		{
			*(buffer + i) = '\0';
			break;
		}
		*(buffer + i) = temp;
		if(temp != ' ' || delspace != 0)
		{		
			i++;
			delspace = 1;
		}
	}
	if(i == 0)
	 	return NULL;
	while(*(buffer + --i) == ' ')
	{
	   *(buffer + i) = '\0';
	}
	return buffer;
	
}

int GetLineFromString(_auth_char* orignal,_auth_char* buffer)
{
	int i = 0;
	int temp;
	char delspace = 0;
	int count = 0;
	while(1)
	{
retry:
		temp = *(orignal + count);
		count ++;
		if(temp == '\n' || temp == '\0')
		{
			*(buffer + i) = '\0';
			break;
		}
		*(buffer + i) = temp;
		if(temp != ' ' || delspace != 0)
		{		
			i++;
			delspace = 1;
		}
	}
	if(i == 0)
	 	return 0;
	while(*(buffer + --i) == ' ')
	{
	   *(buffer + i) = '\0';
	}
	return (count -1);
}
/*
_auth_char* GetProp_Value_From_String(_auth_char* orignal,_auth_char* propertyname,_auth_char* propertyvalue)
{
	_auth_char str[500];
	_auth_char temp;
	unsigned int i = 0;
	_auth_char* filestring = NULL;
	_auth_char* tempstring = NULL;
	_auth_char strpropertyname[_auth_max_line_string_count];
	tempstring = orignal;
	while(TRUE)
	{
		temp = *(tempstring++);
		if(temp != '\0' && temp != '\n')
		{
			*(str + i) = temp;
			i++;
		}
		else
		{
			*(str + i) = '\0';
			i = 0;
			GetElemValue(str,strpropertyname,propertyvalue);
			if(!_auth_strcmp(strpropertyname,propertyname))
			{
				//fclose(file);
				//free(filestring);
				return TRUE;
			}
			if(temp == '\0') break;
		}
	}
	//fclose(file);
	//free(filestring);
	return FALSE;
}
*/

//filepath,propertyname must be null-terminated
BOOL _get_property_value(_auth_char* filepath,_auth_char* propertyname,_auth_char* propertyvalue)
{
	_auth_char str[500];
	_auth_char temp;
	unsigned int i = 0;
	_auth_char* filestring = NULL;
	_auth_char* tempstring = NULL;
	_auth_char strpropertyname[_auth_max_line_string_count];
	//int temp = ' ';
	FILE *file;
	//fopen_s(&file,filepath,"r");
	#ifdef _AUTH_UNICODE
	file = _wfopen(filepath,_auth_string("r,ccs=UNICODE"));
	#else	
	file = fopen(filepath,"r");
	#endif
	if(!file)
	return FALSE;	
	filestring = GetStringFromFile(file);
	tempstring = filestring;
	while(TRUE)
	{
		temp = *(tempstring++);
		if(temp != '\0' && temp != '\n')
		{
			*(str + i) = temp;
			i++;
		}
		else
		{
			*(str + i) = '\0';
			i = 0;
			GetElemValue(str,strpropertyname,propertyvalue);
			if(!_auth_strcmp(strpropertyname,propertyname))
			{
				fclose(file);
				free(filestring);
				return TRUE;
			}
			if(temp == '\0') break;
		}
	}
	fclose(file);
	free(filestring);
	return FALSE;
}

DWORD _set_property_value(_auth_char* filepath,_auth_char* propertyname,_auth_char* propertyvalue,BOOL create)
{
	FILE *file;
	_auth_char str[_auth_max_line_string_count];
	_auth_char temp;
//	_auth_char* formerpropertyvalue;
	_auth_char* filestring;
	_auth_char* tempstring;
	_auth_char* newfilestring;
	int newfilesize = 0;
	unsigned int i = 0;
	unsigned int index = 0;
	unsigned int cutindex = 0;
	char needcreate = 1;
	_auth_char elemname[_auth_max_line_string_count];
	_auth_char elemvalue[_auth_max_line_string_count];
	#ifdef _AUTH_UNICODE
	file = _wfopen(filepath,_auth_string("a+,ccs=UNICODE"));
	#else
	file = fopen(filepath,"a+");
	#endif
	if(!file)
	return FALSE;
	filestring = GetStringFromFile(file);
	fclose(file);

	tempstring = filestring;
	newfilesize = (_auth_strlen(filestring) + _auth_max_line_string_count)*sizeof(_auth_char);
	newfilestring = (_auth_char*)malloc(newfilesize);
	memset(newfilestring,0,newfilesize);
	if(!newfilestring)
		printf("malloc error,function:_set_property_value,size is %d",newfilesize);
	while(TRUE)
	{		
		index++;
		temp = *(tempstring++);
		if(temp != '\0' && temp != '\n')
		{
			*(str + i) = temp;
			i++;
		}
		else
		{
			*(str + i) = '\n';
			*(str + i + 1) = '\0';
			i = 0;
			if(_auth_strlen(str) < 2)
			{
				break;
			}
			if(*str != '\0')
			{
				GetElemValue(str,elemname,elemvalue);
				if(_auth_strcmp(elemname,propertyname) == 0)
				{
					needcreate = 0;
					*(filestring + cutindex) = '\0'; 
					SetElemValue(str,propertyname,propertyvalue);					
					//*tempstring = '\0';
					_auth_strcpy(newfilestring,filestring);
					_auth_strcpy(_auth_strlen(newfilestring) + newfilestring,str);
					_auth_strcpy(_auth_strlen(newfilestring) + newfilestring,tempstring);
					break;
				}												
			}
			cutindex = index;
			if(temp == '\0')
			{
				break;
			}
		}		
	}	
	if(needcreate)	//write a new line in the end 
	{
		SetElemValue(str,propertyname,propertyvalue);
		_auth_strcpy(newfilestring,filestring);//copy old values
		if(create)
			_auth_strcpy(_auth_strlen(newfilestring) + newfilestring,str);//copy new line
	}
	//free GetStringFromFile's memory
	free(filestring);	
	
	#ifdef _AUTH_UNICODE	    
	file = _wfopen(filepath,_auth_string("w,ccs=UNICODE"));
	#else
	file = fopen(filepath,"w");	
	#endif	
	if(!file)
		goto FALSE_CONDITION;
	fwrite(newfilestring,sizeof(_auth_char),_auth_strlen(newfilestring),file);
	fclose(file);
	free(newfilestring);//free malloc memory
	return TRUE;
	FALSE_CONDITION:
	free(newfilestring);
	return FALSE;	
}

DWORD _set_property_variable(_auth_char* filepath,_auth_char* propertyname,_auth_char* propertyvalue,...)
{
	char buffer[VARIABLELENGTH];
	va_list arg_ptr;
	va_start(arg_ptr,propertyvalue);
	vsprintf(buffer, propertyvalue, arg_ptr);
	va_end(arg_ptr);
	return _set_property_value(filepath,propertyname, buffer,TRUE);
}


BOOL _delete_property_by_name(_auth_char* filepath,_auth_char* propertyname)
{
	FILE *file;
	_auth_char str[_auth_max_line_string_count];
	_auth_char temp;
//	_auth_char* formerpropertyvalue;
	_auth_char* filestring;
	_auth_char* tempstring;
	_auth_char* newfilestring;
	int newfilesize = 0;
	unsigned int i = 0;
	unsigned int index = 0;
	unsigned int cutindex = 0;
	_auth_char elemname[_auth_max_line_string_count];
	_auth_char elemvalue[_auth_max_line_string_count];
	char foundproperty = 0;
	#ifdef _AUTH_UNICODE
	file = _wfopen(filepath,_auth_string("a+,ccs=UNICODE"));
	#else	
	file = fopen(filepath,"a+");
	#endif
	if(!file)
	return -1;
	filestring = GetStringFromFile(file);
	fclose(file);
	tempstring = filestring;
	newfilesize = (_auth_strlen(filestring)+1)*sizeof(_auth_char);
	newfilestring = (_auth_char*)malloc(newfilesize);
	memset(newfilestring,0,newfilesize);
	if(!newfilestring)
		printf("malloc error,function:_set_property_value,size is %d",newfilesize);
	while(TRUE)
	{		
		index++;
		temp = *(tempstring++);
		if(temp != '\0' && temp != '\n')
		{
			*(str + i) = temp;
			i++;
		}
		else
		{
			*(str + i) = '\n';
			*(str + i + 1) = '\0';
			if(_auth_strlen(str) < 2)
			{
				break;
			}
			i = 0;
			if(*str != '\0')
			{
				GetElemValue(str,elemname,elemvalue);
				if(_auth_strcmp(elemname,propertyname) == 0)
				{
					//SetElemValue(str,propertyname,propertyvalue);
					foundproperty = 1;
					*(filestring + cutindex) = '\0'; 					
					//*((char*)tempstring) = '\0';
					_auth_strcpy(newfilestring,filestring);
					//_auth_strcpy(_auth_strlen((char*)newfilestring) + newfilestring,str);
					_auth_strcpy(_auth_strlen(newfilestring) + newfilestring,tempstring);
					break;
				}												
			}
			cutindex = index;
			if(temp == '\0')
			{
				break;
			}
		}		
	}
	//free GetStringFromFile's memory
	free(filestring);	
	if(foundproperty)//if found the property ,need to rewrite the file
	{
		#ifdef _AUTH_UNICODE	    
		file = _wfopen(filepath,_auth_string("w,ccs=UNICODE"));
		#else
		file = fopen(filepath,"w");		
		#endif
		if(!file)
			goto FALSE_CONDITION;
		fwrite(newfilestring,sizeof(_auth_char),_auth_strlen(newfilestring),file);
		fclose(file);
	}
	free(newfilestring);//free malloc memory
	return TRUE;
	FALSE_CONDITION:
	free(newfilestring);
	return FALSE;
}

BOOL WriteElemToFile(FILE* file,pElemType pElem)
{
	if(!file) return FALSE;
	fwrite(pElem->elemName,sizeof(_auth_char),strlen(pElem->elemName),file);
	fwrite(_auth_string("="),sizeof(_auth_char),1,file);
	fwrite(pElem->elemValue,sizeof(_auth_char),strlen(pElem->elemValue),file);
	fwrite(_auth_string("\n"),sizeof(_auth_char),1,file);
	return TRUE;
}

BOOL CheckAuth(int pass_level,_auth_char* username,_auth_char* password,_auth_char* fathername)
{
	_auth_char nameBuffer[255];
	_auth_char passwordBuffer[255];
	_auth_char usernameBuffer[255];
	memset(nameBuffer,0,255*sizeof(_auth_char));
	if(fathername == NULL)
	{
		_auth_snprintf(nameBuffer,255,_auth_string("auth%d"),pass_level);	
	}
	else
	{
		if(_auth_strlen(fathername) == 0)
		{
			_auth_snprintf(nameBuffer,255,_auth_string("auth%d"),pass_level);
		}
		else
		{			
			_auth_snprintf(nameBuffer,255,_auth_string("auth%d_%s"),pass_level,fathername);	
		}
	}
	if(pass_level == 0)
	{
		_get_property_value(nameBuffer,_auth_string("username"),usernameBuffer);
		_get_property_value(nameBuffer,_auth_string("password"),passwordBuffer);
		if(!_auth_strcmp(password,passwordBuffer)&&!_auth_strcmp(usernameBuffer,username))
		{
			return TRUE;
		}
	}
	else
	{
		if(_get_property_value(nameBuffer,username,passwordBuffer)) 
		{
			if(!_auth_strcmp(password,passwordBuffer))
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

BOOL CreateAuth(int pass_level,_auth_char* username,_auth_char* password,_auth_char* fathername)
{
   _auth_char nameBuffer[255];
	memset(nameBuffer,0,255*sizeof(_auth_char));
	if(fathername == NULL)
	{
		_auth_snprintf(nameBuffer,255,_auth_string("auth%d"),pass_level);	
	}
	else
	{
		if(_auth_strlen(fathername) == 0)
		{
			_auth_snprintf(nameBuffer,255,_auth_string("auth%d"),pass_level);
		}
		else
		{			
			_auth_snprintf(nameBuffer,255,_auth_string("auth%d_%s"),pass_level,fathername);	
		}
	}
	if(pass_level == 0)
	{
		if(_set_property_value(nameBuffer,_auth_string("username"),username,TRUE) && \
			_set_property_value(nameBuffer,_auth_string("password"),password,TRUE))
			return TRUE;
	}
	else
	{
		if(_set_property_value(nameBuffer,username,password,TRUE)) 
			return TRUE;
	}
	return FALSE;
}



BOOL GetAuth(int pass_level,_auth_char*username,_auth_char* password,_auth_char* fathername)
{
	_auth_char nameBuffer[255];
	memset(nameBuffer,0,255*sizeof(_auth_char));
	if(fathername == NULL)
	{
		_auth_snprintf(nameBuffer,255,_auth_string("auth%d"),pass_level);	
	}
	else
	{
#ifdef _AUTH_UNICODE
		if(wcslen(fathername) == 0)
#else
		if(strlen(fathername) == 0)
#endif
		{
			_auth_snprintf(nameBuffer,255,_auth_string("auth%d"),pass_level);
		}
		else
		{			
			_auth_snprintf(nameBuffer,255,_auth_string("auth%d_%s"),pass_level,fathername);	
		}
	}
	if(pass_level == 0)
	{
	if(_get_property_value(nameBuffer,_auth_string("username"),username)&&\
			_get_property_value(nameBuffer,_auth_string("password"),password))
	{
		return TRUE;
	}
	}
	else
	{
		if(_get_property_value(nameBuffer,username,password))
	{
		return TRUE;
	}
	}
	return FALSE;
}

wchar_t* ctowc(wchar_t* des,char* src)//src must be null terminated,will copy null character,
{
	int i;
	for(i = 0;i<strlen(src) + 1;i++)
	{
		*(des + i) = *(src + i);
	}
	return des;
}

char* wctoc(char* dst,wchar_t* src)
{
	int i;
	for(i = 0;i<wcslen(src) + 1;i++)
	{
		#pragma warning(disable:4244)
		*(dst + i) = *(src + i);
		#pragma warning(default:4244)
	}	
	return dst;
}

char* StrToMac(char* str,char* mac)
{
	char temp[3];
	int i =0;
	memset(temp,0,3);
	for(;i<6;i++)
	{
		memcpy(temp,str + i*2,2);
		*(mac + i*2) = atoi(temp);	
	}
	return mac;
}

#define BUFFSIZE 1024
static void Die(char *mess) 
{ 
   perror(mess); 
  // exit(1); 
}



#undef BUFFSIZE

int WriteLog(FILE* fp,const char * format,...)
{
	va_list arg_ptr;
	char needclosefile = 0;
	int nWrittenBytes;
	if(!fp)
	{
		fp = fopen(LOGPATH,"a");
		if(!fp)
			return -1;
		needclosefile = 1;
	}
	va_start(arg_ptr,format);
	nWrittenBytes = vfprintf(fp, format, arg_ptr);
	va_end(arg_ptr);
	if(needclosefile)
		fclose(fp);
	return nWrittenBytes;
}
