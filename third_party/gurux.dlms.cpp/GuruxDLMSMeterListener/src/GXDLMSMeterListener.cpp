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
#include <winsock2.h>//Add support for sockets
#include <time.h>
#include <process.h>//Add support for threads
#else //Linux includes.
#include <stdio.h>
#include <pthread.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/socket.h> //Add support for sockets
#include <unistd.h> //Add support for sockets
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>
#endif

#include "../include/communication.h"
#include "../include/GXDLMSMeterListener.h"
#include "../../development/include/GXDLMSData.h"
#include "../../development/include/GXDLMSTranslator.h"

using namespace std;

void ListenerThread(void* pVoid)
{
    CGXByteBuffer reply;
    CGXDLMSMeterListener* server = (CGXDLMSMeterListener*)pVoid;
    sockaddr_in add = { 0 };
    int ret = 0;
    char tmp[10];
#if defined(_WIN32) || defined(_WIN64)//If Windows
    int len;
    int AddrLen = sizeof(add);
#else //If Linux
    socklen_t len;
    socklen_t AddrLen = sizeof(add);
#endif
    struct sockaddr_in client;
    memset(&client, 0, sizeof(client));
    //Get buffer data
    basic_string<char> senderInfo;

    /**
    * Received data. This is used if GBT is used and data is received on
    * several data blocks.
    */
    CGXReplyData data;
    CGXReplyData notify;
    while (server->IsConnected())
    {
        len = sizeof(client);
        senderInfo.clear();
        int socket = accept(server->GetSocket(), (struct sockaddr*)&client, &len);
        if (server->IsConnected())
        {
            if ((ret = getpeername(socket, (sockaddr*)&add, &AddrLen)) == -1)
            {
#if defined(_WIN32) || defined(_WIN64)//If Windows
                closesocket(socket);
                socket = INVALID_SOCKET;
#else //If Linux
                close(socket);
                socket = -1;
#endif
                continue;
                //Notify error.
            }
            senderInfo = inet_ntoa(add.sin_addr);
            senderInfo.append(":");
#if _MSC_VER > 1000
            _ltoa_s(add.sin_port, tmp, 10, 10);
#else
            sprintf(tmp, "%d", add.sin_port);
#endif
            senderInfo.append(tmp);
            //Make the connection to the meter and read all data.
            CGXDLMSSecureClient cl(server->m_UseLogicalNameReferencing,
                server->m_ClientAddress,
                server->m_ServerAddress,
                server->m_Authentication,
                server->m_Password,
                server->m_IntefaceType);

            cl.GetCiphering()->SetSecurity(server->m_Security);
            CGXByteBuffer bb;
            if (server->m_SystemTitle != NULL)
            {
                bb.Clear();
                bb.SetHexString(server->m_SystemTitle);
                cl.GetCiphering()->SetSystemTitle(bb);
            }
            if (server->m_AuthenticationKey != NULL)
            {
                bb.Clear();
                bb.SetHexString(server->m_AuthenticationKey);
                cl.GetCiphering()->SetAuthenticationKey(bb);
            }
            if (server->m_BlockCipherKey != NULL)
            {
                bb.Clear();
                bb.SetHexString(server->m_BlockCipherKey);
                cl.GetCiphering()->SetBlockCipherKey(bb);
            }
            if (server->m_DedicatedKey != NULL)
            {
                bb.Clear();
                bb.SetHexString(server->m_DedicatedKey);
                cl.GetCiphering()->SetDedicatedKey(bb);
            }
            CGXCommunication comm(&cl, 5000, GX_TRACE_LEVEL_VERBOSE, NULL);
            comm.SetSocket(socket);
            ret = comm.ReadAll(NULL);
        }
        //Close socket.
#if defined(_WIN32) || defined(_WIN64)//If Windows
        closesocket(socket);
        socket = INVALID_SOCKET;
#else //If Linux
        close(socket);
        socket = -1;
#endif
        if (ret != 0)
        {
            char buff[200];
#if _MSC_VER > 1000
            sprintf_s(buff, 100, "%s\n", CGXDLMSConverter::GetErrorMessage(ret));
#else
            sprintf(buff, "%s\n", CGXDLMSConverter::GetErrorMessage(ret));
#endif
        }
    }
}

#if defined(_WIN32) || defined(_WIN64)//If Windows
#else //If Linux
void* UnixListenerThread(void* pVoid)
{
    ListenerThread(pVoid);
    return NULL;
}
#endif

bool CGXDLMSMeterListener::IsConnected()
{
#if defined(_WIN32) || defined(_WIN64)//If Windows
    return m_ServerSocket != INVALID_SOCKET;
#else //If Linux
    return m_ServerSocket != -1;
#endif
}

int CGXDLMSMeterListener::GetSocket()
{
    return m_ServerSocket;
}

int CGXDLMSMeterListener::StartServer(int port)
{
    int ret;
    if ((ret = StopServer()) != 0)
    {
        return ret;
    }
    m_ServerSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (!IsConnected())
    {
        //socket creation.
        return -1;
    }
    int fFlag = 1;
    if (setsockopt(m_ServerSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&fFlag, sizeof(fFlag)) == -1)
    {
        //setsockopt.
        return -1;
    }
    sockaddr_in add = { 0 };
    add.sin_port = htons(port);
    add.sin_addr.s_addr = htonl(INADDR_ANY);
    add.sin_family = AF_INET;
    if ((ret = ::bind(m_ServerSocket, (sockaddr*)&add, sizeof(add))) == -1)
    {
        //bind;
        return -1;
    }
    if ((ret = listen(m_ServerSocket, 1)) == -1)
    {
        //socket listen failed.
        return -1;
    }
#if defined(_WIN32) || defined(_WIN64)//Windows includes
    m_ReceiverThread = (HANDLE)_beginthread(ListenerThread, 0, (LPVOID)this);
#else
    ret = pthread_create(&m_ReceiverThread, NULL, UnixListenerThread, (void*)this);
#endif
    return ret;
}

int CGXDLMSMeterListener::StopServer()
{
    if (IsConnected())
    {
#if defined(_WIN32) || defined(_WIN64)//Windows includes
        closesocket(m_ServerSocket);
        m_ServerSocket = INVALID_SOCKET;
        if (m_ReceiverThread != INVALID_HANDLE_VALUE)
        {
            int ret = ::WaitForSingleObject(m_ReceiverThread, 5000);
            m_ReceiverThread = INVALID_HANDLE_VALUE;
        }
#else
        shutdown(m_ServerSocket, SHUT_RDWR);
        close(m_ServerSocket);
        m_ServerSocket = -1;
        void* res;
        pthread_join(m_ReceiverThread, (void**)&res);
        free(res);
#endif
    }
    return 0;
}