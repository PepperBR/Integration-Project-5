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

#include "../include/GXDLMSPushListener.h"
#include "../../development/include/GXDLMSData.h"
#include "../../development/include/GXDLMSTranslator.h"
#include "../../development/include/GXDLMSClock.h"

using namespace std;

void ListenerThread(void* pVoid)
{
    CGXByteBuffer reply;
    // Client used to parse received data.
    CGXDLMSClient cl(true, -1, -1, DLMS_AUTHENTICATION_NONE, NULL, DLMS_INTERFACE_TYPE_WRAPPER);
    CGXDLMSPushListener* server = (CGXDLMSPushListener*)pVoid;

    sockaddr_in add = { 0 };
    int ret;
    char tmp[10];
    CGXByteBuffer bb;
    bb.Capacity(2048);
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
    //Add push objects if they are not included to received data.
    CGXDLMSPushSetup push;
    CGXDLMSData ldn("0.0.420.0.255");
    //Logical device name is shown as string.
    ldn.SetUIDataType(2, DLMS_DATA_TYPE_STRING);
    CGXDLMSClock clock;
    push.GetPushObjectList().push_back(std::pair<CGXDLMSObject*, CGXDLMSCaptureObject>(&ldn, CGXDLMSCaptureObject(2, 0)));
    push.GetPushObjectList().push_back(std::pair<CGXDLMSObject*, CGXDLMSCaptureObject>(&clock, CGXDLMSCaptureObject(2, 0)));

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
            while (server->IsConnected())
            {
                //If client is left wait for next client.
                if ((ret = recv(socket, (char*)
                    bb.GetData() + bb.GetSize(),
                    bb.Capacity() - bb.GetSize(), 0)) == -1)
                {
                    bb.SetSize(0);
#if defined(_WIN32) || defined(_WIN64)//If Windows
                    closesocket(socket);
                    socket = INVALID_SOCKET;
#else //If Linux
                    close(socket);
                    socket = -1;
#endif
                    break;
                }
                //If client is closed the connection.
                if (ret == 0)
                {
                    bb.SetSize(0);
#if defined(_WIN32) || defined(_WIN64)//If Windows
                    closesocket(socket);
                    socket = INVALID_SOCKET;
#else //If Linux
                    close(socket);
                    socket = -1;
#endif
                    break;
                }
                bb.SetSize(bb.GetSize() + ret);
                if ((ret = cl.GetData(bb, data, notify)) != 0 && ret != DLMS_ERROR_CODE_FALSE)
                {
                    bb.SetSize(0);
#if defined(_WIN32) || defined(_WIN64)//If Windows
                    closesocket(socket);
                    socket = INVALID_SOCKET;
#else //If Linux
                    close(socket);
                    socket = -1;
#endif
                }
                bb.SetSize(0);

                // If all data is received.
                if (notify.IsComplete())
                {
                    bb.SetSize(0);
                    if (!notify.IsMoreData())
                    {
                        std::vector<std::pair<CGXDLMSObject*, CGXDLMSCaptureObject> > result;
                        if (push.GetPushValues(&cl, notify.GetValue().Arr, result) == 0)
                        {
                            for (std::vector<std::pair<CGXDLMSObject*, CGXDLMSCaptureObject> >::iterator it = result.begin(); it != result.end(); ++it)
                            {                            
                                std::vector<std::string> values;
                                it->first->GetValues(values);
                                printf("%s\r\n", values[it->second.GetAttributeIndex() - 1].c_str());
                            }
                            for (std::vector<std::pair<CGXDLMSObject*, CGXDLMSCaptureObject> >::iterator it = result.begin(); it != result.end(); ++it)
                            {
                                delete it->first;
                            }
                            result.clear();
                        }
                        //Show data as XML.
                        string xml;
                        CGXDLMSTranslator t(DLMS_TRANSLATOR_OUTPUT_TYPE_SIMPLE_XML);
                        t.DataToXml(notify.GetData(), xml);
                        printf(xml.c_str());                       
                        printf("Server address: %d Client Address: %d\r\n", notify.GetServerAddress(), notify.GetClientAddress());
                        notify.Clear();
                        bb.SetSize(0);
                    }
                }
            }
        }
    }
    bb.Clear();
}

#if defined(_WIN32) || defined(_WIN64)//If Windows
#else //If Linux
void* UnixListenerThread(void* pVoid)
{
    ListenerThread(pVoid);
    return NULL;
}
#endif

bool CGXDLMSPushListener::IsConnected()
{
#if defined(_WIN32) || defined(_WIN64)//If Windows
    return m_ServerSocket != INVALID_SOCKET;
#else //If Linux
    return m_ServerSocket != -1;
#endif
}

int CGXDLMSPushListener::GetSocket()
{
    return m_ServerSocket;
}

int CGXDLMSPushListener::StartServer(int port)
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
#if defined(_WIN32) || defined(_WIN64)//Windows includes
    add.sin_family = AF_INET;
#else
    add.sin_family = AF_INET;
#endif
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

int CGXDLMSPushListener::StopServer()
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
        close(m_ServerSocket);
        m_ServerSocket = -1;
        void* res;
        pthread_join(m_ReceiverThread, (void**)&res);
        free(res);
#endif
    }
    return 0;
}