﻿#include "stdafx.h"
#include "XGetOpt.h"
#ifdef WIN32
#define ENET_SUPPORT
#include "enet/enet.h"
#pragma comment( linker, "/subsystem:\"console\" /entry:\"mainCRTStartup\"")
#endif
typedef enum
{
    TCP_SERVER = 0,
    TCP_CLIENT,
    UDP_SERVER,
    UDP_CLIENT,
    ENET_SERVER,
    ENET_CLIENT,
    QUIC_SERVER,
    QUIC_CLIENT,
    ROLE_TYPE_MAX
} ROLE_TYPE;

const CHAR* roleTypeName[] = {
    ("tcpserver"),
    ("tcpclient"),
    ("udpserver"),
    ("udpclient"),
    ("enetserver"),
    ("enetclient"),
    ("quicserver"),
    ("quicclient")
};

extern void TCPServerTest(int port);
extern void TCPClientTest(char* address, int port);

extern void UDPServerTest(int port);
extern void UDPClientTest(char* address, int port);

#ifdef ENET_SUPPORT
extern void ENetServerTest(int port);
extern void ENetClientTest(char* address, int port);
#endif

extern void QUICServerTest(int port);
extern void QUICClientTest(char* address, int port);

HANDLE    g_StopEvent  = NULL;
DWORD     g_RecvBytes  = 0;

CRITICAL_SECTION g_RecvByetLock;

DWORD WINAPI TimerProc(void* pParam)
{
    while (TRUE)
    {
        EnterCriticalSection(&g_RecvByetLock);

        DBG_INFO(_T("Transfor Speed %d KBps\r\n"), g_RecvBytes / 1024);

        g_RecvBytes = 0;

        LeaveCriticalSection(&g_RecvByetLock);

        DWORD Ret = WaitForSingleObject(g_StopEvent, 1000);
        if (Ret != WAIT_TIMEOUT)
        {
            break;
        }
    }

    return 0;
}

int main(int argc, CHAR* argv[])
{
	int c;
    
    char ipAddr[64] = { 0 };
    int port = 0;
    ROLE_TYPE roleType = ROLE_TYPE_MAX;

	BOOL isServer = FALSE;

    InitializeCriticalSection(&g_RecvByetLock);

    g_StopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

#ifdef ENET_SUPPORT
    enet_initialize();
#endif

	while ((c = getopt(argc, argv, "t:a:p:")) != -1)
    {
        switch (c)
        {
            case 't':
            {
#ifdef UNICODE
                DBG_INFO(_T("name: %S\n"), optarg);
#else
                DBG_INFO(_T("name: %s\n"), optarg);
#endif

                for (int i = 0; i < ROLE_TYPE_MAX; i++)
                {
                    if (strcmp(roleTypeName[i], optarg) == 0)
                    {
                        roleType = (ROLE_TYPE)i;
                        break;
                    }
                }

                if (roleType == ROLE_TYPE_MAX)
                {
                    DBG_ERROR(_T("ERROR: unknow role %s\n"), optarg);
                    return -1;
                }

                if (roleType == TCP_SERVER || roleType == UDP_SERVER || roleType == ENET_SERVER || roleType == QUIC_SERVER)
                {
                    isServer = TRUE;
                }

                break;
            }
            case 'p':
            {
                TCHAR* end = NULL;
                int tmpPort = atoi(optarg);

                DBG_INFO(_T("port: %d\n"), tmpPort);
                port = tmpPort;
                break;
            }
            case 'a':
            {
                if (!isServer)
                {
#ifdef UNICODE
                    DBG_INFO(_T("addr: %S\n"), optarg);
#else
                    DBG_INFO(_T("addr: %s\n"), optarg);
#endif
                    strcpy(ipAddr, optarg);
                }
                break;
            }
            default:
            {
                DBG_ERROR(_T("WARNING: no handler for option %c\n"), c);
                return -1;
            }
        }
    }

    if (port == 0)
    {
        DBG_ERROR(_T("ERROR: unknow port\n"));
        return -1;
    }

    if (!isServer && strlen(ipAddr) == 0)
    {
        DBG_ERROR(_T("ERROR: unknow addr\n"));
        return -1;
    }

    if (isServer || roleType == QUIC_CLIENT)
    {
        CreateThread(NULL, 0, TimerProc, NULL, 0, NULL);
    }

    switch (roleType)
    {
        case TCP_SERVER:
            TCPServerTest(port);
            break;
        case TCP_CLIENT:
            TCPClientTest(ipAddr, port);
            break;
        case UDP_SERVER:
            UDPServerTest(port);
            break;
        case UDP_CLIENT:
            UDPClientTest(ipAddr, port);
            break;
#ifdef ENET_SUPPORT            
        case ENET_SERVER:
            ENetServerTest(port);
            break;
        case ENET_CLIENT:
            ENetClientTest(ipAddr, port);
            break;
#endif            
        case QUIC_SERVER:
            QUICServerTest(port);
            break;
        case QUIC_CLIENT:
            QUICClientTest(ipAddr, port);
            break;
        default:
            DBG_ERROR(_T("ERROR: unknow type\n"));
            break;
    }
#ifdef ENET_SUPPORT
    enet_deinitialize();
#endif    

    DeleteCriticalSection(&g_RecvByetLock);
}


