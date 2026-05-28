//
// --------------------------------------------------------------------------
//  Gurux Ltd
//
//
//
// Filename:        $HeadURL$
//
// Version:         $Revision$,
//                  $Date$
//                  $Author$
//
// Copyright (c) Gurux Ltd
//
//---------------------------------------------------------------------------
//
//  DESCRIPTION
//
// This file is a part of Gurux Device Framework.
//
// Gurux Device Framework is Open Source software; you can redistribute it
// and/or modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; version 2 of the License.
// Gurux Device Framework is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// More information of Gurux products: http://www.gurux.org
//
// This code is licensed under the GNU General Public License v2.
// Full text may be retrieved at http://www.gnu.org/licenses/gpl-2.0.txt
//---------------------------------------------------------------------------

#if defined(_WIN32) || defined(_WIN64)//Windows includes
#include <tchar.h>
#include <conio.h>
#include <Winsock.h> //Add support for sockets	
#include <time.h>
#include <process.h>//Add support for threads
#else //Linux includes.
#define closesocket close
#include <unistd.h>
#include <pthread.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/socket.h> //Add support for sockets
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>
#endif
#include "../include/GXDLMSPushListener.h"
#include "../../development/include/GXDLMSData.h"
#include "../../development/include/GXDLMSClock.h"

/**
* Close socket.
*/
int Close(int& s)
{
    if (s != -1)
    {
        closesocket(s);
        s = -1;
    }
    return 0;
}

/**
* Connect to Push listener.
*/
int Connect(const char* address, int port, int& s)
{
    //create socket.
    s = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (s == -1)
    {
        assert(0);
        return DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    sockaddr_in add;
    add.sin_port = htons(port);
    add.sin_family = AF_INET;
    add.sin_addr.s_addr = inet_addr(address);
    //If address is give as name
    if(add.sin_addr.s_addr == INADDR_NONE)
    {
        hostent *Hostent = gethostbyname(address);
        if (Hostent == NULL)
        {
#if defined(_WIN32) || defined(_WIN64)//If Windows
            int err = WSAGetLastError();
#else
            int err = errno;
#endif
            Close(s);
            return err;
        };
        add.sin_addr = *(in_addr*)(void*)Hostent->h_addr_list[0];
    };

    //Connect to the meter.
    int ret = connect(s, (sockaddr*)&add, sizeof(sockaddr_in));
    if (ret == -1)
    {
        return DLMS_ERROR_CODE_INVALID_PARAMETER;
    };
    return DLMS_ERROR_CODE_OK;
}
//This example sends Logical device name and current time i push message.
int Test()
{
    int socket = -1;
    int ret;
    int port = 4059;
    CGXDLMSNotify cl(true, 1, 1, DLMS_INTERFACE_TYPE_WRAPPER);
    CGXDLMSPushSetup p;
    CGXDLMSClock clock;
    CGXDLMSData ldn("0.0.420.0.255");
    //Add logical name as octet-string.
    CGXDLMSVariant value;
    value.Add("GRX00000", 8);
    ldn.SetValue(value);
    p.GetPushObjectList().push_back(std::pair<CGXDLMSObject*, CGXDLMSCaptureObject>(&ldn, CGXDLMSCaptureObject(2, 0)));
    p.GetPushObjectList().push_back(std::pair<CGXDLMSObject*, CGXDLMSCaptureObject>(&clock, CGXDLMSCaptureObject(2, 0)));

    ///////////////////////////////////////////////////////////////////////
    //Create Gurux DLMS server component for Short Name and start listen events.
    CGXDLMSPushListener pushListener(true, DLMS_INTERFACE_TYPE_WRAPPER);
    pushListener.StartServer(port);
    printf("Listening DLMS Push IEC 62056-47 messages on port %d.\r\n", port);
    printf("Press X to close and Enter to send a Push message.\r\n");
    int key = 0;
    while ((key = getchar()) != 'X' && key != 'x')
    {
        if (key == 10)
        {
            printf("Sending Push message.\r\n");
            if ((ret = Connect("localhost", port, socket)) != 0)
            {
                break;
            }
            CGXDateTime now = CGXDateTime::Now();
            clock.SetTime(now);
            std::vector<CGXByteBuffer> reply;
            if ((ret = cl.GeneratePushSetupMessages(NULL, &p, reply)) != 0)
            {
                break;
            }
            for (std::vector<CGXByteBuffer>::iterator it = reply.begin(); it != reply.end(); ++it)
            {
                if ((ret = send(socket, (const char*)it->GetData(), it->GetSize(), 0)) == -1)
                {
                    break;
                }
            }
            Close(socket);
        }
    }
    pushListener.StopServer();
    return 0;
}

#if defined(_WIN32) || defined(_WIN64)//Windows includes
int _tmain(int argc, _TCHAR* argv[])
#else
int main( int argc, char* argv[] )
#endif
{
#if defined(_WIN32) || defined(_WIN64)//Windows includes
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        // Tell the user that we could not find a usable WinSock DLL.
        return 1;
    }
#endif
    Test();
#if defined(_WIN32) || defined(_WIN64)//Windows
    WSACleanup();
#if _MSC_VER > 1400
    _CrtDumpMemoryLeaks();
#endif
#endif

    return 0;
}

