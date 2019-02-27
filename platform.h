#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include <iostream>  

#if defined(WIN32) || defined(WIN64)
#include <Winsock2.h>  
#include <winsock.h>  
#include <ws2tcpip.h>
#include <windows.h>  
#include <io.h>
#include <process.h>
#pragma comment(lib,"ws2_32.lib")  
#else
#include <errno.h>  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>
#include <unistd.h>
#endif


#if _MSC_VER
#define snprintf _snprintf
typedef long long ssize_t;
#endif


#endif