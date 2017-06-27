/*
** 2001 September 15
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
*/
#ifndef _FUZZY_QUERY_H
#define _FUZZY_QUERY_H
#include "sqlite3.h"

enum
{
	CMP_ALL = 0,
	SIMILAR_CC ,
	SAME_TYPE
};
/*
** Make sure we can call this stuff from C++.
*/
#ifdef __cplusplus
extern "C" {
#endif
void cmp_licence(sqlite3_context * ctx, int argc, sqlite3_value ** argv);
int do_cmp_licence_same_type(const char * licence_in_db, const char * licence_to_cmp,int diff_len_permit,int ignore_cc);
int do_cmp_licence_similar(const char * licence_in_db, const char * licence_to_cmp,int ignore_cc);
#ifdef __cplusplus
}  /* end of the 'extern "C"' block */
#endif

#endif  /* ifndef _SQLITE3RTREE_H_ */

