//
//  RedIPConnUtil.h
//  ButelEventConnectSDK
//
//  Created by Gai Gong on 10/12/2017.
//  Copyright © 2017 Butel. All rights reserved.
//
#ifndef	__IP_CONNECT_UTIL_H_
#define __IP_CONNECT_UTIL_H_

#include <string>
// #ifdef __cplusplus
// extern "C" {
/**
 * 判断指定本地出口IP能访问目标地址（含域名方式、指定IP方式）
 * 成功返回 0, 失败返回小于0的数
 */
std::string iputil_test_ip_connectivity(const char* dstaddr, int dstport, const char* localip);

bool iputil_ip_or_domain(const char *s);

int iputil_domain_2_ip(const char *domain, char *ip);

int PingHost(const char* DescIp,const char* LocalIp);
// }		// extern "C"
// #endif	// __cplusplus
#endif	// __IP_CONNECT_UTIL_H_