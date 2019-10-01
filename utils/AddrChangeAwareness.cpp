#include "StdAfx.h"
#include "AddrChangeAwareness.h"
#ifdef WIN32
#include <WinSock2.h>
#include <iphlpapi.h>
#include <wininet.h>
#pragma comment(lib, "Wininet.lib")
#pragma comment(lib,"Iphlpapi.lib")
#include <tchar.h>
#include <atlbase.h>
#else
#include <stddef.h>
#endif
#include <string>
#include "IPConnUtil.h"

#define CONNECT_TRACE //
#ifndef WIN32
typedef unsigned long DWORD;
typedef void VOID;
typedef void* HANDLE;
typedef unsigned short WORD;
#endif

using namespace std;

//
class AddrChangeAwareness::AddrChangeAwarenessImpl
{
public:
	AddrChangeAwarenessImpl();
	~AddrChangeAwarenessImpl();

public:
	bool BeginInBackground(AddrChangeAwarenessListener *listener);
	bool End();

public:
	bool GetAdapterAddrs(AdapterInfo *&ainfos, int *ainfoCounts);
	void FreeAdapterAddrs(AdapterInfo *ainfos);

	void LogicInThread();

protected:
#ifdef WIN32
	static DWORD WINAPI BackgroundThreadProc(VOID *tag);
#else
	static DWORD BackgroundThreadProc(VOID *tag);
#endif

protected:
	AddrChangeAwarenessListener *m_pListener;
	bool m_bStoping;
	HANDLE m_hEvent;
	HANDLE m_hThread;
};

AddrChangeAwareness::AddrChangeAwarenessImpl::AddrChangeAwarenessImpl()
{
	m_pListener = NULL;
	m_bStoping = false;
#ifdef WIN32
	m_hEvent = WSACreateEvent();
#else
	m_hEvent = NULL;
#endif
	m_hThread = NULL;
}

AddrChangeAwareness::AddrChangeAwarenessImpl::~AddrChangeAwarenessImpl()
{
	End();
#ifdef WIN32
	WSACloseEvent(m_hEvent);
#endif
	m_hEvent = NULL;
}

bool AddrChangeAwareness::AddrChangeAwarenessImpl::BeginInBackground(AddrChangeAwarenessListener *listener)
{
	if (listener == NULL)
		return false;
	End();
	m_bStoping = false;
	m_pListener = listener;

#ifdef WIN32
	m_hThread = CreateThread(NULL, 0, BackgroundThreadProc, this, 0, NULL);
#endif

	return true;
}

bool AddrChangeAwareness::AddrChangeAwarenessImpl::End()
{
	m_bStoping = true;
#ifdef WIN32
	SetEvent(m_hEvent);
#endif

	if (m_hThread != NULL)
	{
#ifdef WIN32
		WaitForSingleObject(m_hThread, INFINITE);
		CloseHandle(m_hThread);
#endif
		m_hThread = NULL;
	}

	m_pListener = NULL;
	return true;
}

bool AddrChangeAwareness::AddrChangeAwarenessImpl::GetAdapterAddrs(AdapterInfo *&ainfos, int *ainfoCounts)
{
	if (ainfos != NULL || ainfoCounts == NULL)
		return false;
#ifdef WIN32
	PMIB_IPADDRTABLE pIPAddrTable = NULL;
	ULONG lSize = sizeof(MIB_IPADDRTABLE);
	pIPAddrTable = (PMIB_IPADDRTABLE)malloc(lSize);
	if (pIPAddrTable == NULL)
		return false;

	if (GetIpAddrTable(pIPAddrTable, &lSize, FALSE) == ERROR_INSUFFICIENT_BUFFER)
	{
		pIPAddrTable = (PMIB_IPADDRTABLE)realloc(pIPAddrTable, lSize);
	}

	if (GetIpAddrTable(pIPAddrTable, &lSize, FALSE) == NO_ERROR)
	{
		ainfos = new AdapterInfo[pIPAddrTable->dwNumEntries];

		DWORD i = 0, j = 0;
		for (; i < pIPAddrTable->dwNumEntries; i++)
		{
			if ((pIPAddrTable->table[i].wType & (MIB_IPADDR_DELETED | MIB_IPADDR_TRANSIENT)) != 0)
				continue;

			ainfos[j].index = pIPAddrTable->table[i].dwIndex;
			ainfos[j].connectable = 0;
			struct in_addr addr;
			addr.s_addr = pIPAddrTable->table[i].dwAddr;
			strcpy_s(ainfos[j].ip, inet_ntoa(addr));

			j++;
		}

		*ainfoCounts = j;
	}
#endif
	return true;
}

void AddrChangeAwareness::AddrChangeAwarenessImpl::FreeAdapterAddrs(AdapterInfo *ainfos)
{
	delete[]ainfos;
}

void AddrChangeAwareness::AddrChangeAwarenessImpl::LogicInThread()
{
#ifdef WIN32
	OVERLAPPED overlap;
	HANDLE hand = NULL;
	overlap.hEvent = m_hEvent;

	while (true)
	{
		NotifyAddrChange(&hand, &overlap);
		WaitForSingleObject(overlap.hEvent, INFINITE);

		if (m_bStoping)
			break;

		AdapterInfo *ainfos = NULL;
		int ainfosCount = 0;
		if (GetAdapterAddrs(ainfos, &ainfosCount))
		{
			m_pListener->OnAddrChange(ainfos, ainfosCount);
			FreeAdapterAddrs(ainfos);
		}
	}
#endif
}

#ifdef WIN32
DWORD WINAPI AddrChangeAwareness::AddrChangeAwarenessImpl::BackgroundThreadProc(VOID *tag)
#else
DWORD AddrChangeAwareness::AddrChangeAwarenessImpl::BackgroundThreadProc(VOID *tag)
#endif
{
	AddrChangeAwarenessImpl *thiz = (AddrChangeAwarenessImpl*)tag;
	thiz->LogicInThread();
	return 0;
}
//
AddrChangeAwareness::AddrChangeAwareness()
{
	m_pImpl = new AddrChangeAwarenessImpl();
	memset(m_dstaddr, 0, sizeof(m_dstaddr));
	m_dstport = 0;
}

AddrChangeAwareness::~AddrChangeAwareness()
{
	if (m_pImpl != NULL)
	{
		m_pImpl->End();
		delete m_pImpl;
		m_pImpl = NULL;
	}
}

bool AddrChangeAwareness::IsInternetConnected()
{
	DWORD dwFlags = 0;
#ifdef WIN32
	if(!InternetGetConnectedState(&dwFlags, NULL))
		return false;
#endif
	return true;
}

//////////////////////////////////////////////////////////////////////////
static
bool Static_IsWindowsVersionOrGreater(WORD wMajorVersion, WORD wMinorVersion, WORD wServicePackMajor)
{
#ifdef WIN32
	OSVERSIONINFOEXW osvi = { sizeof(osvi), 0, 0, 0, 0, { 0 }, 0, 0 };
	DWORDLONG        const dwlConditionMask = VerSetConditionMask(
		VerSetConditionMask(
		VerSetConditionMask(
		0, VER_MAJORVERSION, VER_GREATER_EQUAL),
		VER_MINORVERSION, VER_GREATER_EQUAL),
		VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);

	osvi.dwMajorVersion = wMajorVersion;
	osvi.dwMinorVersion = wMinorVersion;
	osvi.wServicePackMajor = wServicePackMajor;

	return VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR, dwlConditionMask) != FALSE;
#else
	return true;
#endif
}

#ifndef _WIN32_WINNT_VISTA
#define _WIN32_WINNT_VISTA                  0x0600
#endif

static
bool Static_IsWindowsVistaOrGreater()
{
	static bool initialized = false, retvalue = false;
#ifdef WIN32
	if (!initialized)
	{
		retvalue = Static_IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_VISTA), LOBYTE(_WIN32_WINNT_VISTA), 0);
		initialized = true;
	}
#endif

	return retvalue;
}

#ifdef WIN32
bool IsWifiNetAdapter(PIP_ADAPTER_INFO pAdapter)
{
	if (pAdapter == NULL)
		return false;
	
	bool ret = false;

	if (Static_IsWindowsVistaOrGreater())
	{
		return pAdapter->Type == IF_TYPE_IEEE80211 ? true : false;
	}

	do
	{
		TCHAR keyName[MAX_PATH] = { 0 };
		_stprintf_s(keyName, _T("SYSTEM\\CurrentControlSet\\Control\\Network\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\%S\\Connection"),
			pAdapter->AdapterName);

		CRegKey regKey;
		if (regKey.Open(HKEY_LOCAL_MACHINE, keyName, KEY_READ) != ERROR_SUCCESS)
		{
			break;
		}

		DWORD mediaSubType = 0;
		if (regKey.QueryDWORDValue(_T("MediaSubType"), mediaSubType) != ERROR_SUCCESS)
		{
			break;
		}

		ret = mediaSubType == 2 ? true : false;
	} while (0);

	return ret;
}

#endif

bool AddrChangeAwareness::IsWifiNetAdapterByIndex(int index)
{
#ifdef WIN32
	PIP_ADAPTER_INFO pAdapterInfo = NULL;
	ULONG lBuffer = sizeof(pAdapterInfo);
	pAdapterInfo = (PIP_ADAPTER_INFO)malloc(lBuffer);

	ULONG ret = GetAdaptersInfo(pAdapterInfo, &lBuffer);
	if (ret == ERROR_BUFFER_OVERFLOW)
	{
		pAdapterInfo = (PIP_ADAPTER_INFO)realloc(pAdapterInfo, lBuffer);
		if (pAdapterInfo == NULL)
			return false;
	}

	bool ret_v = false;

	ret = GetAdaptersInfo(pAdapterInfo, &lBuffer);
	if (ret == NO_ERROR)
	{
		PIP_ADAPTER_INFO pAdapter = pAdapterInfo;
		while (pAdapter != NULL)
		{
			if (pAdapter->Index == index)
			{
				ret_v = IsWifiNetAdapter(pAdapter);
				break;
			}

			pAdapter = pAdapter->Next;;
		}
	}

	free(pAdapterInfo);

	return ret_v;
#else
	return true;
#endif
}

int AddrChangeAwareness::GetBestConnectInterface()
{
#ifdef WIN32
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	DWORD dwBestInterface = 0;
	if (GetBestInterfaceEx((sockaddr *)&addr, &dwBestInterface) != NO_ERROR)
		return 0;
	return dwBestInterface;
#else
	return 0;
#endif
}

int AddrChangeAwareness::GetBestConnectInterfaceBypingHost(AdapterInfo *ainfos, int num)
{
#ifdef WIN32
	DWORD dwBestInterface = 0;
	int dwDelayAva = 0, dwDelayMin = 500;
	int iFailCount = 0,i;
	string ipTemp;
	static int nshowdstaddr = 0;
	if (0 == num)
	{
		return 0;
	}
	
	if ((1 == nshowdstaddr) || (0 == nshowdstaddr%500))
	{
		CONNECT_TRACE("GetBestConnectInterfaceBypingHost dspaddr:%s dstport:%d", m_dstaddr, m_dstport);
	}
	nshowdstaddr++;
	// 没有读到，或者没有设置nps地址和端口则不做处理
	if (0 == m_dstport || 0 == strlen(m_dstaddr))
		num = 0;

	for (i = 0; i < num; i++)
	{
		ipTemp = ainfos[i].ip;
		if ("127.0.0.1" == ipTemp || "0.0.0.0" == ipTemp || 0 == ipTemp.length())
		{
			iFailCount++;
			continue;
		}
		dwDelayAva = PingHost(m_dstaddr, ipTemp.c_str());
		if (0 > dwDelayAva)
		{
			iFailCount++;
			CONNECT_TRACE("GetBestConnectInterfaceBypingHost ipTemp:%s failed", ipTemp.c_str());
		}
		else if (dwDelayAva < dwDelayMin)
		{
			dwDelayMin = dwDelayAva;
			dwBestInterface = ainfos[i].index;
			ainfos[i].connectable = 1;
			//CONNECT_TRACE("GetBestConnectInterfaceBypingHost ipTemp:%s maybe best", ipTemp.c_str());
		}
		else if (dwDelayAva < 500)
		{
			ainfos[i].connectable = 1;
		}
		else
		{
			ainfos[i].connectable = 0;
			iFailCount++;
			CONNECT_TRACE("GetBestConnectInterfaceBypingHost ipTemp:%s is nok, num:%d iFailCount %d", ipTemp.c_str(), num, iFailCount);
		}
	}
	
	if (0 == dwBestInterface || num == iFailCount)
	{
		sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr("114.112.74.13");//htonl(INADDR_ANY);
		CONNECT_TRACE("GetBestConnectInterfaceBypingHost dwBestInterface %d num:%d iFailCount:%d", dwBestInterface, num, iFailCount);
		if (GetBestInterfaceEx((sockaddr *)&addr, &dwBestInterface) != NO_ERROR)
			return 0;
		for (i = 0; i < num; i++)
		{
			if (dwBestInterface == ainfos[i].index)
			{
				ainfos[i].connectable = 1;
			}
		}
	}

	return dwBestInterface;
#else
	return 0;
#endif
}

int gettimeofday1 (struct timeval *tv, void* tz)
{
	union
	{
		__int64 ns100; /*time since 1 Jan 1601 in 100ns units */
		FILETIME fileTime;
	} now;
	GetSystemTimeAsFileTime (&now.fileTime);
	tv->tv_usec = (long) ((now.ns100 / 10LL) % 1000000LL);
	tv->tv_sec = (long) ((now.ns100 - 116444736000000000LL) / 10000000LL);
	return (0);
}
int AddrChangeAwareness::RemoveInvalidAddrs(AdapterInfo *ainfos, int num, std::vector<std::string> curiplist)
{
#ifdef	WIN32
	DWORD dwBestInterface = 0;
	string ipTemp;
	int i = 0, j = 0;
	//int nRet = 0;
	int nValid = 0;
	int nInUse = 0;
	struct timeval t1v;
	struct timeval t2v;
	map<string, int>::iterator it;
	string errstr = "";
	string msgstr = "";
	static int is_first = 0;
	if (0 >= num)
		return 0;
	if (3 > is_first)
	{
		CONNECT_TRACE("RemoveInvalidAddrs dstip:%s dstport:%d", m_dstaddr, m_dstport);
		is_first += 1;
	}
	 // 没有读到，或者没有设置nps地址和端口则不做处理
	if (0 == m_dstport || 0 == strlen(m_dstaddr))
		return 0;

	for (i = 0; i < num; i++)
	{
		ipTemp = ainfos[i].ip;
		if ("127.0.0.1" == ipTemp || "0.0.0.0" == ipTemp || 0 == ipTemp.length())
		{
			continue;
		}
		
		gettimeofday1(&t1v, NULL);
		errstr = iputil_test_ip_connectivity(m_dstaddr, m_dstport, ipTemp.c_str());
		gettimeofday1(&t2v, NULL);
		//CONNECT_TRACE("RemoveInvalidAddrs nRet:%s local ip %s connect time %lu(s):%lu(us)", \
			errstr.c_str(), ipTemp.c_str(),(t2v.tv_sec-t1v.tv_sec), ((t2v.tv_sec-t1v.tv_sec)*1000*1000+t2v.tv_usec)-t1v.tv_usec);
		for(it = m_failed_count.begin(); it != m_failed_count.end(); it++)
		{
			if (it->first == ipTemp)
			{
				break;
			}
		}
		
		ainfos[i].connectable = 1;
		if (errstr != "ok")
		{
			CONNECT_TRACE("RemoveInvalidAddrs nRet:%s local ip %s connect time %lu(s):%lu(us)", \
				errstr.c_str(), ipTemp.c_str(),(t2v.tv_sec-t1v.tv_sec), ((t2v.tv_sec-t1v.tv_sec)*1000*1000+t2v.tv_usec)-t1v.tv_usec);
			//ainfos[i].connectable = 0;
			for (j = 0,nInUse=0; j < curiplist.size(); j++)
			{
				if (ipTemp == curiplist.at(j))
				{
					nInUse = 1;
					break;
				}
			}
			if (nInUse)
			{
				if (it == m_failed_count.end())
				{
					m_failed_count[ipTemp] = 1;
					CONNECT_TRACE("RemoveInvalidAddrs ip:%s first timeout.", ipTemp.c_str());
				}
				else
				{
					it->second++;
					CONNECT_TRACE("RemoveInvalidAddrs ip:%s failcnt:%d", ipTemp.c_str(), it->second);
					if ( ((0 == j) && it->second >= 3) ||
						(it->second >= 6))
					{
						ainfos[i].connectable = 0;
						m_failed_count.erase(it);
					}
				}
			}
			else
			{
				ainfos[i].connectable = 0;
			}
			continue;
		}
		if (it != m_failed_count.end())
		{
			m_failed_count.erase(it);
		}
		nValid++;
		if (0 == curiplist.size())
			break;
	}
	if (0 == nValid)
	{
		sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr("114.112.74.13");//htonl(INADDR_ANY);

		if (GetBestInterfaceEx((sockaddr *)&addr, &dwBestInterface) != NO_ERROR)
			return 0;
		for (i = 0; i < num; i++)
		{
			string tempip = ainfos[i].ip;

			if ("127.0.0.0" == tempip ||
				"0.0.0.0" == tempip ||
				tempip.empty())
				continue;
			if (dwBestInterface == ainfos[i].index)
			{
				ainfos[i].connectable = 1;
			}
		}
	}
	return nValid;
#else
	return 0;
#endif
}
bool AddrChangeAwareness::BeginInBackground(AddrChangeAwarenessListener *listener)
{
	return m_pImpl->BeginInBackground(listener);
}

bool AddrChangeAwareness::End()
{
	return m_pImpl->End();
}

bool AddrChangeAwareness::GetAdapterAddrs(AdapterInfo *&ainfos, int *ainfoCounts)
{
	return m_pImpl->GetAdapterAddrs(ainfos, ainfoCounts);
}
void AddrChangeAwareness::FreeAdapterAddrs(AdapterInfo *ainfos)
{
	return m_pImpl->FreeAdapterAddrs(ainfos);
}

void AddrChangeAwareness::SetDstAddr(const char *dstaddr, unsigned short nport)
{
	if (dstaddr && 0 < nport && 65535 > nport)
	{
		memcpy(m_dstaddr, dstaddr, strlen(dstaddr));
		m_dstport = nport;
	}
	else
	{
		memset(m_dstaddr, 0, 128);
		m_dstport = 0;
	}

	return;
}