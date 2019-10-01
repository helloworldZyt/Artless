//
//  RedIPConnUtil.m
//  ButelEventConnectSDK
//
//  Created by Gai Gong on 10/12/2017.
//  Copyright © 2017 Butel. All rights reserved.
//

#include "StdAfx.h"

#include "IPConnUtil.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <windows.h>
#include <iostream>
#include <winsock.h>
#pragma comment(lib, "ws2_32")
using namespace std;
#else
#include <unistd.h>
#include <netdb.h>  
#include <net/if.h>  
#include <arpa/inet.h>  
#include <sys/ioctl.h>  
#include <sys/types.h>  
#include <sys/socket.h> 
#include <ifaddrs.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <sys/sockio.h>
#include <errno.h>
#include <net/if_dl.h>
#include <sys/utsname.h>
#endif

static bool qn_is_ipv4_addr(const char* ip)
{
    return strchr(ip, '.')!=NULL;
}

 bool iputil_ip_or_domain(const char *s)
{
	const char *pChar;
    bool rv = true;
    int tmp1, tmp2, tmp3, tmp4, i;
    while( 1 )
    {
        i = sscanf(s, "%d.%d.%d.%d", &tmp1, &tmp2, &tmp3, &tmp4);
        if( i != 4 )
        {
            rv = false;
            break;
        }
        if( (tmp1 > 255) || (tmp2 > 255) || (tmp3 > 255) || (tmp4 > 255) )
        {
            rv = false;
            break;
        }
        for( pChar = s; *pChar != 0; pChar++ )
        {
            if( (*pChar != '.')
                && ((*pChar < '0') || (*pChar > '9')) )
            {
                rv = false;
                break;
            }
        }
        break;
    }
    return rv;
}

int iputil_domain_2_ip(const char *domain, char *ip)
{  
#ifdef WIN32	
	 int WSA_return;
     WSADATA WSAData;
 
     WSA_return=WSAStartup(0x0101,&WSAData);

     /* 结构指针 */ 
     HOSTENT *host_entry;
     if(WSA_return==0)
     {
         /* 即要解析的域名或主机名 */
         host_entry=gethostbyname(domain);
         printf("%s\n", domain);
         if(host_entry!=0)
         {
              printf("解析IP地址: ");
              sprintf(ip,"%d.%d.%d.%d",
              (host_entry->h_addr_list[0][0]&0x00ff),
              (host_entry->h_addr_list[0][1]&0x00ff),
              (host_entry->h_addr_list[0][2]&0x00ff),
              (host_entry->h_addr_list[0][3]&0x00ff));
		       printf("ip = %s",ip);
		       WSACleanup();
		       return 0;
         }
	 }else{
	     WSACleanup();
	     return -1;
	 }
     
#else
    char **pptr;  
    struct hostent *hptr;  
  
    hptr = gethostbyname(domain);  
    if(NULL == hptr)  
    {  
        printf("gethostbyname error for host:%s/n", domain);  
        return -1;  
    }  
  
    for(pptr = hptr->h_addr_list ; *pptr != NULL; pptr++)  
    {  
        if (NULL != inet_ntop(hptr->h_addrtype, *pptr, ip, IP_SIZE) )  
        {  
            return 0; // 只获取第一个 ip  
        }  
    }  
	return -1;
#endif
}

#if 0
static bool qn_get_ipv6_sockaddr(const char* ip, sockaddr_in6 & addr)
{
    if (NULL == ip)
        return false;
    
    struct addrinfo *ailist, *aip;        
    struct addrinfo hint;        
    struct sockaddr_in6 *sinp6;  
    
    hint.ai_family = PF_UNSPEC;//AF_INET6;                   /*  hint 的限定设置  */
    hint.ai_socktype = SOCK_DGRAM;     /*   这里可是设置 socket type    比如  SOCK——DGRAM */
    hint.ai_flags = 0;//AI_PASSIVE;                    // flags 的标志很多  。常用的有AI_CANONNAME;
    hint.ai_protocol = IPPROTO_UDP;                               /*  设置协议  一般为0，默认 */         
    hint.ai_addrlen = 0;                                /*  下面不可以设置，为0，或者为NULL  */
    hint.ai_canonname = NULL;        
    hint.ai_addr = NULL;        
    hint.ai_next = NULL;
    //ilRc = getaddrinfo(hostname, port, &hint, &ailist);    /*通过主机名获得地址信息*/      
    //ilRc = getaddrinfo("fe80::500b:78f2:34dc:7442%14", port, &hint, &ailist);
    int ilRc = getaddrinfo(ip, "8080", &hint, &ailist);
    
    if (ilRc < 0)        
    {               
        //char str_error[100];                
        //strcpy(str_error, (char *)gai_strerror(errno));                
        printf("getaddrinfo [%s] failure", ip);
        return false;        
    }
    
    if(ailist == NULL)
    {
        printf("sorry not find the IP address,please try again \n");
        return false;
    }
    
    bool ret = false;
    for (aip = ailist; aip != NULL; aip = aip->ai_next)                         /* 显示获取的信息  */
    {
        //char rip[64] = {0};
        //const char* resip = inet_ntop(aip->ai_family,aip->ai_addr, rip, sizeof(rip));
        //printf("pre ip:%s\n", resip);
        
        if (aip->ai_family == AF_INET6)
        {
            aip->ai_family = AF_INET6;
            sinp6 = (struct sockaddr_in6 *)aip->ai_addr;                                  /* 为什么是for 循环 ，先向下看 */
            addr = *sinp6;
            ret = true;
            break;
        }
    } 
    
    freeaddrinfo(aip);
    
    return ret;
    
}
#endif

static string nps_req_str()
{
	string reqstr = "";
	string urlstr = "http://103.25.23.99/nps/parameter/getServiceParameters";
	string strParam = "params={\"serialNumber\":\"SNAD00000000001\",\"devModel\":\"AD_SN\",\"mac\":\"mobileMAC\"}";
	char send_str[2048] = {NULL};

	strcat(send_str,"POST ");
	strcat(send_str,urlstr.c_str());
	strcat(send_str," HTTP/1.1\r\n");
	strcat(send_str,"Accept: */*\r\n");
	strcat(send_str,"Accept-Language: zh-cn\r\n");
	strcat(send_str,"host:");
	strcat(send_str,"103.25.23.99");
	strcat(send_str,"\r\n");
	strcat(send_str,"Content-Type: application/x-www-form-urlencoded\r\n");
	strcat(send_str,"Content-Length:");
	char len[20] = {NULL};
	sprintf(len,"%d", strParam.length());
	strcat(send_str,len);
	strcat(send_str,"\r\n");
	strcat(send_str,"Connection:keep-alive\r\n\r\n");
	strcat(send_str,strParam.c_str());
	reqstr = send_str;

	return reqstr;
}

string iputil_test_ip_connectivity(const char* dstaddr, int dstport, const char* localip)
{
    //检查空指针
	unsigned long ul = 0, ul1 = 0;
    int TimeOut=0;
    int scfd = -1;
	int ret = 0;
	WSADATA wsaData;
    
    sockaddr_in src_addr_v4;
    sockaddr_in dst_addr_v4;
    
    string errstr = "";
	char tmpbuf[256] = {0};
    char dst_ipaddr[64];
    memset(dst_ipaddr, 0, sizeof(dst_ipaddr));
   
    do
    {
		// only ipv4 need transform domain 2 ip.
		if (!iputil_ip_or_domain(dstaddr))
		{
			iputil_domain_2_ip(dstaddr, dst_ipaddr);
		}
		else{
			memcpy(dst_ipaddr, dstaddr, strlen(dstaddr));
		}
		if (!iputil_ip_or_domain(dst_ipaddr))
		{
			
			errstr = "invalid dst ip:";
			errstr+=dst_ipaddr;
			break;
		}
#ifdef WIN32
		if(WSAStartup(MAKEWORD(2,0),&wsaData)){return 0;}
		scfd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
#else
		scfd = socket(AF_INET, SOCK_STREAM, 0);
#endif

		if(scfd==INVALID_SOCKET) {
			memset(tmpbuf, 0, 256);
			sprintf(tmpbuf,"create socket failed! errno:%d", errno);
			errstr = tmpbuf;
			break;
		}
		//设置非阻塞方式连接
		ul = 1;
		ret = ioctlsocket(scfd, FIONBIO, (unsigned long*)&ul);
		if(ret==SOCKET_ERROR)
		{
			memset(tmpbuf, 0, 256);
			sprintf(tmpbuf,"ioctlsocket FIONBIO 1 failed! errno:%d", errno);
			errstr = tmpbuf;
			break;
		}

		src_addr_v4.sin_family = AF_INET;
		src_addr_v4.sin_addr.s_addr = inet_addr(localip);
		src_addr_v4.sin_port = htons(0);

		dst_addr_v4.sin_family = AF_INET;
		dst_addr_v4.sin_addr.s_addr = inet_addr(dst_ipaddr);
		dst_addr_v4.sin_port = htons(dstport);

		ret = ::bind(scfd, (sockaddr*)&src_addr_v4, sizeof(src_addr_v4));
		if (ret < 0)
		{
			memset(tmpbuf, 0, 256);
			sprintf(tmpbuf,"bind failed ret:%d! errno:%d", ret, errno);
			errstr = tmpbuf;
			break;
		}

		ret = ::connect(scfd, (sockaddr*)&dst_addr_v4, sizeof(dst_addr_v4));
		do {
			struct timeval totv = {3,0};
			fd_set scfdset;
			FD_ZERO(&scfdset);
			FD_SET(scfd, &scfdset);
			int nr = select(scfd, NULL,&scfdset, NULL, &totv);
			if (0 < nr  && 0 == errno && FD_ISSET(scfd, &scfdset))
			{
				char rcvbuf[1024] = {0};
				struct timeval totvr = {3,0};
				fd_set scfdrcv;
				FD_ZERO(&scfdrcv);
				FD_SET(scfd, &scfdrcv);
				string httphdrstr = nps_req_str();
				nr = send(scfd, httphdrstr.c_str(), httphdrstr.length(), 0);
				if (0 > nr || 0 != errno)
				{
					memset(tmpbuf, 0, 256);
					sprintf(tmpbuf,"%s send ret:%d! errno:%d-%s", localip, nr, errno,strerror(errno));
					errstr = tmpbuf;
					break;
				}

				nr = select(scfd, &scfdrcv,NULL, NULL, &totvr);
				if (0 < nr  && 0 == errno && FD_ISSET(scfd, &scfdset))
				{
					nr = recv(scfd, rcvbuf, 256, 0);
					if (0 > nr || 0 != errno)
					{
						memset(tmpbuf, 0, 256);
						sprintf(tmpbuf,"ret:%d! errno:%d-%s msg:%s", nr, errno,strerror(errno), rcvbuf);
						errstr = tmpbuf;
						break;
					}
					else
					{
						errstr = "ok";
					}
				}
				else
				{
					memset(tmpbuf, 0, 256);
					sprintf(tmpbuf,"%s recv select ret:%d! errno:%d-%s", localip, nr, errno,strerror(errno));
					errstr = tmpbuf;
					errno = 0;
				}

			}
			else
			{
				memset(tmpbuf, 0, 256);
				sprintf(tmpbuf,"%s select ret:%d! errno:%d", localip, nr, errno);
				errstr = tmpbuf;
			}
		}while (0);

		ul1 = 0;
		ret = ioctlsocket(scfd, FIONBIO, (unsigned long*)&ul1);
// 		if (SOCKET_ERROR == ret)
// 		{
// 			memset(tmpbuf, 0, 256);
// 			sprintf(tmpbuf,"ioctlsocket FIONBIO 0 failed ret:%d! errno:%d", ret, errno);
// 			errstr = tmpbuf;
// 		}
    }while(0);
                        
    if (scfd != -1)
    {
#ifdef WIN32
		::closesocket (scfd);
#else
        close(scfd);
#endif
    }
                        
    return errstr;
}

int iputil_test_ip_connectivityblock(const char* dstaddr, int dstport, const char* localip)
{
	//检查空指针
	unsigned long ul = 0, ul1 = 0;
	int TimeOut=0;
	int scfd = -1;
	int ret = 0;
	WSADATA wsaData;

	sockaddr_in src_addr_v4;
	sockaddr_in dst_addr_v4;

	int err = -1;
	char dst_ipaddr[64];
	memset(dst_ipaddr, 0, sizeof(dst_ipaddr));

	do
	{
		// only ipv4 need transform domain 2 ip.
		if (!iputil_ip_or_domain(dstaddr))
		{
			iputil_domain_2_ip(dstaddr, dst_ipaddr);
		}
		else{
			memcpy(dst_ipaddr, dstaddr, strlen(dstaddr));
		}
#ifdef WIN32
		if(WSAStartup(MAKEWORD(2,0),&wsaData)){return 0;}
		scfd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
#else
		scfd = socket(AF_INET, SOCK_STREAM, 0);
#endif

		if(scfd==INVALID_SOCKET) {
			break;
		}
		TimeOut=0; //设置发送超时6秒
		if(::setsockopt(scfd,SOL_SOCKET,SO_SNDTIMEO,(char *)&TimeOut,sizeof(TimeOut))==SOCKET_ERROR){
			break;
		}
		//TimeOut=1000;//设置接收超时6秒
		if(::setsockopt(scfd,SOL_SOCKET,SO_RCVTIMEO,(char *)&TimeOut,sizeof(TimeOut))==SOCKET_ERROR){
			break;
		}
		//设置非阻塞方式连接
		//ul = 1;
		//ret = ioctlsocket(scfd, FIONBIO, (unsigned long*)&ul);
		//if(ret==SOCKET_ERROR)
		//{
		//	break;
		//}

		src_addr_v4.sin_family = AF_INET;
		src_addr_v4.sin_addr.s_addr = inet_addr(localip);
		src_addr_v4.sin_port = htons(0);

		dst_addr_v4.sin_family = AF_INET;
		dst_addr_v4.sin_addr.s_addr = inet_addr(dst_ipaddr);
		dst_addr_v4.sin_port = htons(dstport);

		err = ::bind(scfd, (sockaddr*)&src_addr_v4, sizeof(src_addr_v4));
		if (err < 0)
		{
			break;
		}

		err = ::connect(scfd, (sockaddr*)&dst_addr_v4, sizeof(dst_addr_v4));

		//ul1 = 0;
		//ret = ioctlsocket(scfd, FIONBIO, (unsigned long*)&ul1);
		//if (SOCKET_ERROR == ret)
		//{
		//	::closesocket (scfd);
		//}
	}while(0);

	if (scfd != -1)
	{
#ifdef WIN32
		::closesocket (scfd);
#else
		close(scfd);
#endif
	}

	return err;
}

/* ===== start ===== */
#pragma pack(1)
#define ICMP_ECHOREPLY 0  
#define ICMP_ECHOREQ 8
typedef struct _tagIphdr // IP数据包头部  
{  
	u_char VIHL;        // 版本号(4)+头长度(4)  
	u_char TOS;         // 服务类型(8)  
	short TotLen;       // 总长度(16)  
	short ID;           // 标识(16)  
	short FlagOff;      // 标志(3)+片偏移(13)  
	u_char TTL;         // 生存时间TTL(8)
	u_char Proto;         // 协议类型(8)
	u_short CheckSum;   // 头部校验和(16)  
	in_addr iaSrc;      // 源IP地址(32)  
	in_addr iaDst;      // 目标IP地址(32)     
} ip_hdr, *ip_pHdr;  
typedef struct tagICMPHDR   // ICMP回送请求与应带ICMP报文  
{  
	u_char Type;        // 类型(8)  
	u_char Code;        // 代码(8)  
	u_short Checksum;   // 校验和(16)  
	u_short ID;         // 标识符(16)  
	u_short Seq;        // 序号(16)  
	DWORD Data;          // 任选数据  
} icmp_hdr, *icmp_pHdr;
#pragma pack()

u_short in_chsum(u_short *buffer, int size)  
{
#if 0
	register int nLeft = len;  
	register u_short *w = addr;  
	register u_short answer;  
	register int sum = 0;  
	while (nLeft > 1)  
	{  
		sum += *w++;  
		nLeft -= 2;  
	}  
	if (nLeft == 1)  
	{  
		u_short u = 0;  
		*(u_char*)(&u) = *(u_char*)w;  
		sum += u;  
	}  
	sum = (sum >> 16) + (sum & 0xffff);  
	sum += (sum >> 16);  
	answer = ~sum;  
	return (answer);  
#else
	// 	unsigned short checksum(unsigned short *buffer, int size) 
	// 	{
	// 
	// 	}
	unsigned long cksum=0;
	while(size > 1) 
	{
		cksum += *buffer++;
		size -= sizeof(unsigned short);
	}
	if(size) 
	{
		cksum += *(unsigned short*)buffer;
	}
	cksum = (cksum >> 16) + (cksum & 0xffff);
	cksum += (cksum >>16);
	return (unsigned short)(~cksum);
#endif
}

int PingHost(const char* DescIp,const char* LocalIp)
{
	//socket初始化
	DWORD dwDelayAv = 0;
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 0), &wsaData);

	//域名转IP地址
	HOSTENT *pHost = gethostbyname(DescIp);
	if (pHost == NULL)
	{
		/*CDN_ERROR("MyPing gethostbyname Fail");*/
		return -1;
	}

	unsigned long nAddress = ((long**)pHost->h_addr_list)[0][0];
	sockaddr_in addrSend;
	addrSend.sin_family = AF_INET;
	addrSend.sin_port = htons(0);
	addrSend.sin_addr.s_addr = nAddress;

	sockaddr_in addrLocal;
	addrLocal.sin_family = AF_INET;
	addrLocal.sin_port = 0;//htons(8887);
	addrLocal.sin_addr.s_addr =inet_addr(LocalIp) ;

	//创建原始套接字
	SOCKET sRaw = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sRaw == INVALID_SOCKET)
	{
		/*CDN_ERROR("MyPing Creat socket fail");*/
		return -2;
	}
	int ret = bind(sRaw,(const sockaddr*)&addrLocal,sizeof(addrLocal));
	if(ret!=0)
	{
		/*CDN_ERROR("MyPing bind fail Ip:%s,ret:%d",LocalIp,ret);*/
		return -3;
	}

	char szSend[32] = {0};
	int errnum=0;
	int slerrno = 0;
	int dterrno = 0;

	for (int i = 0; i < 10; i++)
	{
		icmp_hdr *pICMP = (icmp_hdr *)szSend;
		pICMP->Code = 0;
		pICMP->Checksum = 0;
		pICMP->Data = ::GetTickCount();
		pICMP->ID = (unsigned short)GetCurrentProcessId();
		pICMP->Seq = i;
		pICMP->Type = ICMP_ECHOREQ;

		pICMP->Checksum = in_chsum((unsigned short *)pICMP, sizeof(icmp_hdr));

		//发送封包
		sendto(sRaw, szSend, sizeof(szSend), 0, (sockaddr *)&addrSend, sizeof(addrSend));

		//通过选择模型，设置等待时间
		timeval tv = {0, 500000};
		DWORD dwNewTick = 0;
RECVTIMEOUT_PK_RETRY:
		fd_set fd;
		FD_ZERO(&fd);
		FD_SET(sRaw, &fd);

		int nResult = select(0, &fd, NULL, NULL, &tv);
		if (nResult == 0)
		{
			errnum++;
			continue;
		}
		else if (0 > nResult)
		{
			slerrno++;
		}

		dwNewTick = ::GetTickCount();

		//接收封包
		char szRecv[MAXBYTE];
		sockaddr_in addrRecv;
		int nLen = sizeof(addrRecv);

		recvfrom(sRaw, szRecv, sizeof(szRecv), 0, (sockaddr *)&addrRecv, &nLen);

		//检验校验和
		ip_hdr * pIPRecv = (ip_hdr *)szRecv;
		icmp_hdr *pICMPRecv = (icmp_hdr *)(pIPRecv + 1);
		// 		char *toCheck = (char *)(pIPRecv + 1);
		// 		icmp_hdr *pICMPRecv = (icmp_hdr *)&toCheck[1];
		if ((i != pICMPRecv->Seq))
		{
			goto RECVTIMEOUT_PK_RETRY;
		}

		//校验和为0，表示封包正确
		if (!in_chsum((unsigned short *)pICMPRecv, sizeof(icmp_hdr)))
			//if (i == pICMPRecv->Seq)
		{
			//计算时间间隔
			DWORD dwTime = dwNewTick - pICMPRecv->Data;
			if (500 < dwTime || 0 > dwTime)
			{
				//CONNECT_TRACE("PingHost dwTime %d pICMPRecv->Data %d i %d pICMPRecv->Seq %d", 
				//dwTime, pICMPRecv->Data, i, pICMPRecv->Seq);
				dwDelayAv += 500;
				dterrno++;
			}
			else
			{
				dwDelayAv += dwTime;
			}

		}
		else
		{
			dwDelayAv += 500;
			dterrno++;
		}
	}
	if(errnum!=0)
	{
		dwDelayAv += errnum * 500;
	}
	closesocket(sRaw);
	//socket释放资源
	WSACleanup( );
	if (5 < (errnum + slerrno + dterrno))
	{
		//CONNECT_TRACE("PingHost errnum %d slerrno %d dterrno %d dwDelayAv %d", errnum, slerrno, dterrno, dwDelayAv);
	}

	return dwDelayAv/10;//返回十次的平均值
}
/* ===== end ===== */