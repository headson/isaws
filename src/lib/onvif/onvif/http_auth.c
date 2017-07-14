/***************************************************************************************
 *
 *  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
 *
 *  By downloading, copying, installing or using the software you agree to this license.
 *  If you do not agree to this license, do not download, install, 
 *  copy or use the software.
 *
 *  Copyright (C) 2010-2014, Happytimesoft Corporation, all rights reserved.
 *
 *  Redistribution and use in binary forms, with or without modification, are permitted.
 *
 *  Unless required by applicable law or agreed to in writing, software distributed 
 *  under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 *  CONDITIONS OF ANY KIND, either express or implied. See the License for the specific
 *  language governing permissions and limitations under the License.
 *
****************************************************************************************/

#include "onvif/bm/sys_inc.h"
#include "onvif/onvif/http_auth.h"
#include "onvif/bm/rfc_md5.h"


void CvtHex(IN HASH Bin, OUT HASHHEX Hex)
{
	unsigned short i;
	unsigned char j;
	for (i = 0; i < HASHLEN; i++) {
		j = (Bin[i] >> 4) & 0xf;

		if (j <= 9)
			Hex[i*2] = (j + '0');
		else
			Hex[i*2] = (j + 'a' - 10);

		j = Bin[i] & 0xf;

		if (j <= 9)
			Hex[i*2+1] = (j + '0');
		else
			Hex[i*2+1] = (j + 'a' - 10);
	};
	Hex[HASHHEXLEN] = '\0';
};

BOOL HexStrToBin(IN char * str,OUT HASH Bin)
{
	int i;

	if(strlen(str) != HASHHEXLEN)
		return FALSE;
	
	for(i=0; i<HASHLEN; i++)
	{
		if(str[i*2] >= '0' && str[i*2] <= '9')
			Bin[i] = (str[i*2] - '0') << 4;
		else if(str[i*2] >= 'a' && str[i*2] <= 'z')
			Bin[i] = (str[i*2] - 'a') << 4;
		else
			return FALSE;

		if(str[i*2+1] >= '0' && str[i*2+1] <= '9')
			Bin[i] |= (str[i*2+1] - '0');
		else if(str[i*2+1] >= 'a' && str[i*2+1] <= 'z')
			Bin[i] |= str[i*2+1] - 'a';
		else
			return FALSE;
	}

	return TRUE;
}

/* calculate H(A1) as per spec */
void DigestCalcHA1(
	IN const char * pszAlg,
	IN const char * pszUserName,
	IN const char * pszRealm,
	IN const char * pszPassword,
	IN const char * pszNonce,
	IN const char * pszCNonce,
	OUT HASHHEX SessionKey
)
{
	MD5_CTX Md5Ctx;
	HASH HA1;

	MD5Init(&Md5Ctx);
	MD5Update(&Md5Ctx, (unsigned char *)pszUserName, strlen(pszUserName));
	MD5Update(&Md5Ctx, (unsigned char *)&(":"), 1);
	MD5Update(&Md5Ctx, (unsigned char *)pszRealm, strlen(pszRealm));
	MD5Update(&Md5Ctx, (unsigned char *)&(":"), 1);
	MD5Update(&Md5Ctx, (unsigned char *)pszPassword, strlen(pszPassword));
	MD5Final(HA1, &Md5Ctx);
	if (strcmp(pszAlg, "md5-sess") == 0) {
		MD5Init(&Md5Ctx);
		MD5Update(&Md5Ctx, HA1, HASHLEN);
		MD5Update(&Md5Ctx, (unsigned char *)&(":"), 1);
		MD5Update(&Md5Ctx, (unsigned char *)pszNonce, strlen(pszNonce));
		MD5Update(&Md5Ctx, (unsigned char *)&(":"), 1);
		MD5Update(&Md5Ctx, (unsigned char *)pszCNonce, strlen(pszCNonce));
		MD5Final(HA1, &Md5Ctx);
	};

	CvtHex(HA1, SessionKey);
};

/* calculate request-digest/response-digest as per HTTP Digest spec */
void DigestCalcResponseHash(
	IN HASHHEX HA1, /* H(A1) */
	IN const char * pszNonce, /* nonce from server */
	IN const char * pszNonceCount, /* 8 hex digits */
	IN const char * pszCNonce, /* client nonce */
	IN const char * pszQop, /* qop-value: "", "auth", "auth-int" */
	IN const char * pszMethod, /* method from the request */
	IN const char * pszDigestUri, /* requested URL */
	IN HASHHEX HEntity, /* H(entity body) if qop="auth-int" */
	OUT HASH  RespHash /* request-digest or response-digest */
)
{
	MD5_CTX Md5Ctx;
	HASH HA2;
	HASHHEX HA2Hex;

	// calculate H(A2)
	MD5Init(&Md5Ctx);
	MD5Update(&Md5Ctx, (unsigned char *)pszMethod, strlen(pszMethod));
	MD5Update(&Md5Ctx, (unsigned char *)&(":"), 1);
	MD5Update(&Md5Ctx, (unsigned char *)pszDigestUri, strlen(pszDigestUri));
	if (strcmp(pszQop, "auth-int") == 0) {
		MD5Update(&Md5Ctx, (unsigned char *)&(":"), 1);
		MD5Update(&Md5Ctx, (unsigned char *)(&HEntity[0]), HASHHEXLEN);
	};
	MD5Final(HA2, &Md5Ctx);
	CvtHex(HA2, HA2Hex);

	// calculate response
	MD5Init(&Md5Ctx);
	MD5Update(&Md5Ctx, (unsigned char *)(&HA1[0]), HASHHEXLEN);
	MD5Update(&Md5Ctx, (unsigned char *)&(":"), 1);
	MD5Update(&Md5Ctx, (unsigned char *)pszNonce, strlen(pszNonce));
	MD5Update(&Md5Ctx, (unsigned char *)&(":"), 1);
	if (*pszQop) {
		MD5Update(&Md5Ctx, (unsigned char *)pszNonceCount, strlen(pszNonceCount));
		MD5Update(&Md5Ctx, (unsigned char *)&(":"), 1);
		MD5Update(&Md5Ctx, (unsigned char *)pszCNonce, strlen(pszCNonce));
		MD5Update(&Md5Ctx, (unsigned char *)&(":"), 1);
		MD5Update(&Md5Ctx, (unsigned char *)pszQop, strlen(pszQop));
		MD5Update(&Md5Ctx, (unsigned char *)&(":"), 1);
	};
	MD5Update(&Md5Ctx, (unsigned char *)(&HA2Hex[0]), HASHHEXLEN);
	MD5Final(RespHash, &Md5Ctx);
};

/* calculate request-digest/response-digest as per HTTP Digest spec */
void DigestCalcResponse(
	IN HASHHEX HA1, /* H(A1) */
	IN const char * pszNonce, /* nonce from server */
	IN const char * pszNonceCount, /* 8 hex digits */
	IN const char * pszCNonce, /* client nonce */
	IN const char * pszQop, /* qop-value: "", "auth", "auth-int" */
	IN const char * pszMethod, /* method from the request */
	IN const char * pszDigestUri, /* requested URL */
	IN HASHHEX HEntity, /* H(entity body) if qop="auth-int" */
	OUT HASHHEX Response /* request-digest or response-digest */
)
{
	HASH  RespHash;
    DigestCalcResponseHash(HA1,pszNonce,pszNonceCount,pszCNonce,
		pszQop,pszMethod,pszDigestUri,HEntity,RespHash);
	CvtHex(RespHash, Response);
};






