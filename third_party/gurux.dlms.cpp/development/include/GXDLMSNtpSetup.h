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

#ifndef GXDLMSNTPSETUP_H
#define GXDLMSNTPSETUP_H

#include "GXIgnore.h"
#include "GXDLMSObject.h"
#ifndef DLMS_IGNORE_NTP_SETUP

/**
Online help:
http://www.gurux.fi/Gurux.DLMS.Objects.GXDLMSNtpSetup
*/
class CGXDLMSNtpSetup : public CGXDLMSObject
{
    /**
    * Is NTP time synchronisation active.
    */
    bool m_Activated;
    /**
     * NTP server address.
     */
    std::string m_ServerAddress;
    /**
     * UDP port related to this protocol.
     */
    uint16_t m_Port;
    /**
     * Authentication method.
     */
    DLMS_NTP_AUTHENTICATION_METHOD m_Authentication;
    /**
     * Symmetric keys for authentication.
     */
    std::map<uint32_t, CGXByteBuffer> m_Keys;
    /**
     * Client key (NTP server public key).
     */
    CGXByteBuffer m_ClientKey;

    int Invoke(CGXDLMSSettings& settings, CGXDLMSValueEventArg& e);

public:
    //Constructor.
    CGXDLMSNtpSetup();

    //SN Constructor.
    CGXDLMSNtpSetup(std::string ln, unsigned short sn);

    //LN Constructor.
    CGXDLMSNtpSetup(std::string ln);

    /**
     * @return Is NTP time synchronisation active.
     */
    bool GetActivated();

    /**
     * @param value
     *            Is NTP time synchronisation active.
     */
    void SetActivated(bool value);

    /**
     * @return NTP server address.
     */
    std::string& GetServerAddress();

    /**
     * @param value
     *            NTP server address.
     */
    void SetServerAddress(std::string& value);

    /**
     * @return UDP port related to this protocol.
     */
    uint16_t GetPort();

    /**
     * @param value
     *            UDP port related to this protocol.
     */
    void SetPort(uint16_t value);

    /**
     * @return Authentication method.
     */
    DLMS_NTP_AUTHENTICATION_METHOD GetAuthentication();

    /**
     * @param value
     *            Authentication method.
     */
    void SetAuthentication(DLMS_NTP_AUTHENTICATION_METHOD value);

    /**
     * @return Symmetric keys for authentication.
     */
    std::map<uint32_t, CGXByteBuffer>& GetKeys();

    /**
     * @return Client key (NTP server public key).
     */
    CGXByteBuffer& GetClientKey();

    /**
     * @param value
     *            Client key (NTP server public key).
     */
    void SetClientKey(CGXByteBuffer& value);

    // Returns amount of attributes.
    int GetAttributeCount();

    // Returns amount of methods.
    int GetMethodCount();

    //Get attribute values of object.
    void GetValues(std::vector<std::string>& values);

    /////////////////////////////////////////////////////////////////////////
    // Returns collection of attributes to read.
    //
    // If attribute is static and already read or device is returned
    // HW error it is not returned.
    //
    // all: All items are returned even if they are read already.
    // attributes: Collection of attributes to read.
    void GetAttributeIndexToRead(bool all, std::vector<int>& attributes);

    int GetDataType(int index, DLMS_DATA_TYPE& type);

    // Returns value of given attribute.
    int GetValue(CGXDLMSSettings& settings, CGXDLMSValueEventArg& e);

    // Set value of given attribute.
    int SetValue(CGXDLMSSettings& settings, CGXDLMSValueEventArg& e);

    //Synchronizes the time of the DLMS server with the NTP server.
    int Synchronize(CGXDLMSClient* client, std::vector<CGXByteBuffer>& reply);

    int AddAuthenticationKey(CGXDLMSClient* client, uint32_t id, CGXByteBuffer& key, std::vector<CGXByteBuffer>& reply);

    //Remove symmetric authentication key.
    int DeleteAuthenticationKey(CGXDLMSClient* client, uint32_t id, std::vector<CGXByteBuffer>& reply);

};
#endif //DLMS_IGNORE_NTP_SETUP
#endif //GXDLMSNTPSETUP_H