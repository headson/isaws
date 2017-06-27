////////////////////////////////////////////////////////////////////////////////
// CppSQLite3 - A C++ wrapper around the SQLite3 embedded database library.
//
// Copyright (c) 2004..2007 Rob Groves. All Rights Reserved. rob.groves@btinternet.com
// 
// Permission to use, copy, modify, and distribute this software and its
// documentation for any purpose, without fee, and without a written
// agreement, is hereby granted, provided that the above copyright notice, 
// this paragraph and the following two paragraphs appear in all copies, 
// modifications, and distributions.
//
// IN NO EVENT SHALL THE AUTHOR BE LIABLE TO ANY PARTY FOR DIRECT,
// INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST
// PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
// EVEN IF THE AUTHOR HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// THE AUTHOR SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
// PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF
// ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". THE AUTHOR HAS NO OBLIGATION
// TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
//
// V3.0		03/08/2004	-Initial Version for sqlite3
//
// V3.1		16/09/2004	-Implemented getXXXXField using sqlite3 functions
//						-Added CppSQLiteDB3::tableExists()
//
// V3.2		01/07/2005	-Fixed execScalar to handle a NULL result
//			12/07/2007	-Added int64 functions to CppSQLite3Query
//						-Throw exception from CppSQLite3DB::close() if error
//						-Trap above exception in CppSQLite3DB::~CppSQLite3DB()
//						-Fix to CppSQLite3DB::compile() as provided by Dave Rollins.
//						-sqlite3_prepare replaced with sqlite3_prepare_v2
//						-Added Name based parameter binding to CppSQLite3Statement.
////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <string.h>
#include "FuzzyQuery.h"
////////////////////////////////////////////////////////////////////////////////

__inline int get_utf8_char_len(const char *utf8_str)
{
	if ((unsigned char)utf8_str[0] == 0)
	{
		return 0;
	}
	else if ((unsigned char)utf8_str[0] < 0x80)
	{
		return 1;
	}
	for(int i = 0; i < 4; i++)
	{
		if ((unsigned char)utf8_str[i] < 0x80)
		{			
			return i;
		}
	}
	return 0;
}
int get_diff_cnt(const char* des, const char* src, int len)
{
	int cnt = 0, des_i = 0, src_i = 0, des_utf8_len, src_utf8_len;
	while(des_i < len && src_i< len)
	{
		des_utf8_len = get_utf8_char_len(&des[des_i]);
		src_utf8_len = get_utf8_char_len(&src[src_i]);
		//printf("get_diff_cnt:%d,%d %d,%d\n",des_utf8_len,des_i,src_utf8_len,src_i);
		if (des_utf8_len == src_utf8_len)
		{
			for(int i = 0; i < des_utf8_len; i++)
			{
				if (des[des_i + i] != src[src_i + i])
				{			
					cnt++;
					break;
				}	
			}
		}
		else
		{
			cnt++;
		}
		des_i += des_utf8_len;
		src_i += src_utf8_len;
	}
	return cnt;
}

int do_cmp_licence_all(const char * licence_in_db, const char * licence_to_cmp,int diff_len_permit)
{

	//普通车牌	汉字+字母+5个字符	7/8	吉BB94SR
	//教练车	汉字+字母+4个字符+学	7/9	粤S2511学
	//港澳车	粤Z+4个字符+港/澳	7/9	粤ZF023港
	//警车	汉字+字母+4个字符+警	7/9	预H0231警
	//军车	2个字符+5个字符	7/7	BA09053
	//使馆车	使+6个字符	7/8	使206013
	//武警车1	WJ汉字+5个字符	8/9	WJ川71029
	//武警车2	WJ + 5个字符	7/7	WJ00065

	//汉字在只在开头 8
	//汉字在开头和结尾 9
	//汉字在中间 WJ+CC 9
	//没汉字  7

	int db_len = strlen(licence_in_db);
	int cmp_len = strlen(licence_to_cmp);
	//for(int i =0; i< 16; i++) printf("%.2X ",licence_to_cmp[i]);
	//printf("\n");
	//for(int i =0; i< 16; i++) printf("%.2X ",licence_in_db[i]);
	//printf("\n[do_cmp_licence]:%d,%d,%d\n",db_len,cmp_len,diff_len_permit);
	if (db_len <7 || db_len >15 ||
		cmp_len <7 || cmp_len >15 ||
		diff_len_permit <0 || diff_len_permit> 5)
	{
		//参数检测
		return 0;
	}
	//printf("\n[do_cmp_licence2]:%d,%d,%d\n",db_len,cmp_len,diff_len_permit);
	int len = cmp_len<db_len?db_len:cmp_len;
	int diff_len = 0;	
	if (licence_in_db[0] == 'W' && licence_to_cmp[0] == 'W' &&
		licence_in_db[1] == 'J' && licence_to_cmp[1] == 'J' )
	{
		if (cmp_len == 7 && db_len == 9 )
		{
			diff_len = get_diff_cnt(&licence_in_db[4], &licence_to_cmp[2], 5) + 1;
		}
		else if (cmp_len == 9 && db_len == 7 )
		{
			diff_len = get_diff_cnt(&licence_in_db[2], &licence_to_cmp[4], 5) + 1;
		}
		else
		{
			diff_len = get_diff_cnt(&licence_in_db[0], &licence_to_cmp[0], len);
		}
	}
	else
	{
		diff_len = get_diff_cnt(&licence_in_db[0], &licence_to_cmp[0], len);
	}
	//printf("\n[do_cmp_licence3]:%d,%d\n",diff_len,diff_len<diff_len_permit);
	return diff_len <= diff_len_permit?1:0;
}
int do_cmp_licence_same_type(const char * licence_in_db, const char * licence_to_cmp,int diff_len_permit,int ignore_cc)
{

	int db_len = strlen(licence_in_db);
	int cmp_len = strlen(licence_to_cmp);

	if (db_len != cmp_len ||
		db_len <7 || db_len >15 ||
		diff_len_permit <0 || diff_len_permit> 5)
	{
		//参数检测
		return 0;
	}
	//printf("\n[do_cmp_licence2]:%d,%d,%d\n",db_len,cmp_len,diff_len_permit);
	int len = cmp_len<db_len?db_len:cmp_len;
	int diff_len = 0;	
	int db_c_len, cmp_c_len;
	int i = 0;
	while( i < cmp_len)
	{
		db_c_len = get_utf8_char_len(&licence_in_db[i]);
		cmp_c_len = get_utf8_char_len(&licence_to_cmp[i]);
		if (db_c_len != cmp_c_len)
		{
			return 0;
		}

		if (db_c_len > 1)
		{
			//汉字忽略
			if (!ignore_cc)
			{
				for (int j = 0; j< db_c_len; j++)
				{
					if (licence_in_db[i+j] != licence_to_cmp[i+j])
					{
						//汉字不匹配
						diff_len++;
						break;
					}
				}
			}
		}
		else if (db_c_len == 1)
		{
			if (licence_in_db[i] != licence_to_cmp[i])
			{
				diff_len++;
				
			}
		}
		else
		{
			//异常字符
			return 0;
		}
		i += db_c_len;
	}
	//printf("diff_len:%d\n",diff_len);
	return diff_len <= diff_len_permit?1:0;
}
//#define _MAKE_WORD(c1,c2) ((((short)c1)<<8)&((short)c2)
int check_similar_cc(char c1,char c2)
{
	const char similar_c[][2]={{'8','B'},{'0','D'},{'0','Q'},{'E','F'}};	
	for (int i = 0; i< sizeof(similar_c)/2;i++)
	{
		if ((c1 == similar_c[i][0] && c2 == similar_c[i][1])||
			(c2 == similar_c[i][0] && c1 == similar_c[i][1]))
		{
			return 1;
		}		
	}
	return 0;
}
int do_cmp_licence_similar(const char * licence_in_db, const char * licence_to_cmp,int ignore_cc)
{

	int db_len = strlen(licence_in_db);
	int cmp_len = strlen(licence_to_cmp);
	
#if 0
	for(int i =0; i< 16; i++) printf("%.2X ",licence_to_cmp[i]);
	printf("\n");
	for(int i =0; i< 16; i++) printf("%.2X ",licence_in_db[i]);
	printf("\n[do_cmp_licence]:%d,%d,%d\n",db_len,cmp_len,ignore_cc);
#endif
	if (db_len != cmp_len ||
		db_len <7 || db_len >15 )
	{
		//参数检测，只检测相同类型的车牌
		return 0;
	}

	int db_c_len, cmp_c_len;
	int i = 0;
	while( i < cmp_len)
	{
		db_c_len = get_utf8_char_len(&licence_in_db[i]);
		cmp_c_len = get_utf8_char_len(&licence_to_cmp[i]);
		if (db_c_len != cmp_c_len)
		{
			return 0;
		}

		if (db_c_len > 1)
		{
			//汉字忽略
			if (!ignore_cc)
			{
				for (int j = 0; j< db_c_len; j++)
				{
					if (licence_in_db[i+j] != licence_to_cmp[i+j])
					{
						//汉字不匹配
						return 0;
					}
				}
			}
		}
		else if (db_c_len == 1)
		{
			if (licence_in_db[i] != licence_to_cmp[i])
			{
				if (!check_similar_cc(licence_in_db[i],licence_to_cmp[i]))
				{
					//非相似字符出错
					return 0;
				}
			}
		}
		else
		{
			//异常字符
			return 0;
		}
		i += db_c_len;
	}
	
	return 1;
}
void cmp_licence(sqlite3_context * ctx, int argc, sqlite3_value ** argv)
{
	//printf("[cmp_licence]argc:%d,%d,%d,%d\n",argc,sqlite3_value_type( argv[ 0 ] ),
	//	sqlite3_value_type( argv[ 1 ] ),sqlite3_value_type( argv[2 ] ));
	if ( argc != 5 )
	{
		return;
	}

	if( sqlite3_value_type( argv[ 0 ] ) != SQLITE_TEXT)
	{
		return;
	}
	if( sqlite3_value_type( argv[ 1 ] ) != SQLITE_TEXT)
	{
		return;
	}
	if( sqlite3_value_type( argv[ 2 ] ) != SQLITE_INTEGER)
	{
		return;
	}
	if( sqlite3_value_type( argv[ 3 ] ) != SQLITE_INTEGER)
	{
		return;
	}
	if( sqlite3_value_type( argv[ 4 ] ) != SQLITE_INTEGER)
	{
		return;
	}
	const char * licence_in_db = (const char * )sqlite3_value_text( argv[ 0 ] );

	const char * licence_to_cmp = (const char * )sqlite3_value_text( argv[ 1 ] );
	int diff_len_permit = (int )sqlite3_value_int( argv[ 2 ] );
	int ignore_cc = (int )sqlite3_value_int( argv[ 3 ] );
	int cmp_type = (int )sqlite3_value_int( argv[ 4 ] );
	//printf("[cmp_licence2]:%s,%s,%d\n",licence_in_db,licence_to_cmp,diff_len_permit);
	switch(cmp_type)
	{
	case SIMILAR_CC:
		//printf("similar cc\n");
		
		//sqlite3_result_int(ctx, strcmp(licence_to_cmp, licence_in_db));
		sqlite3_result_int(ctx, do_cmp_licence_similar(licence_in_db, licence_to_cmp, ignore_cc));
		break;
	case SAME_TYPE:
	
	default:
		sqlite3_result_int(ctx, do_cmp_licence_same_type(licence_in_db, licence_to_cmp, diff_len_permit,ignore_cc));
	}	
}
