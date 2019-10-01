#pragma once

typedef struct _AdapterInfo
{
	unsigned long index;
	int connectable;
	char ip[128];
} AdapterInfo;

#if defined(__IOS__)  || defined(_ANDROID_) 
typedef unsigned int size_t;
#elif defined(_LINUX_)
#include <stddef.h>
#endif
#include <string>
#include <map>
#include <vector>

// GetBestInterfaceEx : Find the best interface
class AddrChangeAwarenessListener
{
public:
	virtual void OnAddrChange(AdapterInfo const *addrInfos, size_t count) = 0;
};

class AddrChangeAwareness
{
public:
	AddrChangeAwareness();
	~AddrChangeAwareness();

public:
	bool IsInternetConnected();
	bool IsWifiNetAdapterByIndex(int index);
	int GetBestConnectInterface();
	int GetBestConnectInterfaceBypingHost(AdapterInfo *ainfos, int num);
	int RemoveInvalidAddrs(AdapterInfo *ainfos, int num, std::vector<std::string> curiplist);

public:
	bool BeginInBackground(AddrChangeAwarenessListener *listener);
	bool End();

	bool GetAdapterAddrs(AdapterInfo *&ainfos, int *ainfoCounts);
	void FreeAdapterAddrs(AdapterInfo *ainfos);
	void SetDstAddr(const char *dstaddr, unsigned short nport);
private:
	char m_dstaddr[128];
	unsigned short  m_dstport;
	std::map<std::string, int > m_failed_count;

protected:
	class AddrChangeAwarenessImpl;
	AddrChangeAwarenessImpl *m_pImpl;
};

