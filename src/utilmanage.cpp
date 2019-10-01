/*****
 *
 *
************************************************************************************************/
#include "StdAfx.h"
#include "utilmanage.h"

#include <stdio.h>
#include <string>

#include "AddrChangeAwareness.h"
#include "IPConnUtil.h"

using namespace std;

typedef struct _error_man_ {
	int err_id;
	char* err_info;
}ErrMan;

ErrMan g_error_manage[] = MANERRORS;

/**
 *@ov		: char* ov[]
 */
int um_api_get_ips(void*iv, void* ov)
{
	char** pov = (char**)ov;
	AddrChangeAwareness addrman;
	AdapterInfo* infos = NULL;
	int cnt = 0;
	int ocnt = 0;
	if (NULL == ov)
	{
		return 1001;
	}
	//infos = new AdapterInfo;
	addrman.GetAdapterAddrs(infos, &cnt);

	for (int i = 0; i < cnt; i++)
	{
		string iptmp = infos[i].ip;
		if ("0.0.0.0" == iptmp || "127.0.0.1" == iptmp)
		{
			continue;
		}
		strncpy(pov[ocnt], iptmp.c_str(), iptmp.length());
		ocnt++;
	}
	return ocnt;
}
/**
 *@desc			:
 *@eid			: api id
 *@v			: param
 */
int utilman_entry(UtilApiIds eid, void* iv, void* ov)
{
	switch(eid)
	{
	case MANN_UTIL_API_GET_IPS:
		return um_api_get_ips(iv, ov);
		break;
	default:
		break;
	}
	return 0;
}