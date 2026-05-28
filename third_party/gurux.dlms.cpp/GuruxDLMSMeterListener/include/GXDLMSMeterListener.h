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

#pragma once

#if defined(_WIN32) || defined(_WIN64)//Windows includes
#include <Winsock.h> //Add support for sockets
#endif

#include "../../development/include/GXDLMSNotify.h"

class CGXDLMSMeterListener
{
private:
#if defined(_WIN32) || defined(_WIN64)//If Windows
    SOCKET m_ServerSocket;
    HANDLE m_ReceiverThread;
#else //If Linux.
    int m_ServerSocket;
    pthread_t m_ReceiverThread;
#endif
public:
    bool m_UseLogicalNameReferencing;
    int m_ClientAddress;
    int m_ServerAddress;
    //Authentication type.
    DLMS_AUTHENTICATION m_Authentication;
    //Password if authentication is used.
    const char* m_Password;
    DLMS_INTERFACE_TYPE m_IntefaceType;
    DLMS_SECURITY m_Security;
    char* m_SystemTitle;
    char* m_AuthenticationKey;
    char* m_BlockCipherKey;
    char* m_DedicatedKey;
    GX_TRACE_LEVEL m_Trace;
    char* m_InvocationCounter;

    /////////////////////////////////////////////////////////////////////////
    //Constructor.
    /////////////////////////////////////////////////////////////////////////
    CGXDLMSMeterListener(bool UseLogicalNameReferencing = true,
        int ClientAddress = 16,
        int ServerAddress = 1,
        //Authentication type.
        DLMS_AUTHENTICATION authentication = DLMS_AUTHENTICATION_NONE,
        //Password if authentication is used.
        const char* password = NULL,
        DLMS_INTERFACE_TYPE intefaceType = DLMS_INTERFACE_TYPE_HDLC,
        DLMS_SECURITY security = DLMS_SECURITY_NONE,
        char* systemTitle = NULL,
        char* authenticationKey = NULL,
        char* blockCipherKey = NULL,
        char* dedicatedKey = NULL,
        GX_TRACE_LEVEL trace = GX_TRACE_LEVEL_INFO,
        char* invocationCounter = NULL)
    {
#if defined(_WIN32) || defined(_WIN64)//If Windows
        m_ReceiverThread = INVALID_HANDLE_VALUE;
        m_ServerSocket = INVALID_SOCKET;
#else //If Linux.
        m_ServerSocket = -1;
        m_ReceiverThread = -1;
#endif
        m_UseLogicalNameReferencing = UseLogicalNameReferencing;
        m_ClientAddress = ClientAddress;
        m_ServerAddress = ServerAddress;
        m_Authentication = authentication;
        m_Password = password;
        m_IntefaceType = intefaceType;
        m_Security = security;
        m_SystemTitle = systemTitle;
        m_AuthenticationKey = authenticationKey;
        m_BlockCipherKey = blockCipherKey;
        m_DedicatedKey = dedicatedKey;
        m_Trace = trace;
        m_InvocationCounter = invocationCounter;
    }


    /////////////////////////////////////////////////////////////////////////
    //Destructor.
    /////////////////////////////////////////////////////////////////////////
    ~CGXDLMSMeterListener(void)
    {
        StopServer();
    }

    bool IsConnected();

    int GetSocket();

    int StartServer(int port);

    int StopServer();
};
