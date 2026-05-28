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

#include "../include/GXDLMSNtpSetup.h"
#include "../include/GXDLMSClient.h"

#ifndef DLMS_IGNORE_NTP_SETUP

//Constructor.
CGXDLMSNtpSetup::CGXDLMSNtpSetup() :
    CGXDLMSNtpSetup("0.0.25.10.0.255", 0)
{
}

//SN Constructor.
CGXDLMSNtpSetup::CGXDLMSNtpSetup(std::string ln, unsigned short sn) :
    CGXDLMSObject(DLMS_OBJECT_TYPE_NTP_SETUP, ln, sn)
{

}

//LN Constructor.
CGXDLMSNtpSetup::CGXDLMSNtpSetup(std::string ln) :
    CGXDLMSNtpSetup(ln, 0)
{

}

bool CGXDLMSNtpSetup::GetActivated()
{
    return m_Activated;
}

void CGXDLMSNtpSetup::SetActivated(bool value)
{
    m_Activated = value;
}

std::string& CGXDLMSNtpSetup::GetServerAddress()
{
    return m_ServerAddress;
}

void CGXDLMSNtpSetup::SetServerAddress(std::string& value)
{
    m_ServerAddress = value;
}

uint16_t CGXDLMSNtpSetup::GetPort()
{
    return m_Port;
}

void CGXDLMSNtpSetup::SetPort(uint16_t value)
{
    m_Port = value;
}

DLMS_NTP_AUTHENTICATION_METHOD CGXDLMSNtpSetup::GetAuthentication()
{
    return m_Authentication;
}

void CGXDLMSNtpSetup::SetAuthentication(DLMS_NTP_AUTHENTICATION_METHOD value)
{
    m_Authentication = value;
}

std::map<uint32_t, CGXByteBuffer>& CGXDLMSNtpSetup::GetKeys()
{
    return m_Keys;
}

CGXByteBuffer& CGXDLMSNtpSetup::GetClientKey()
{
    return m_ClientKey;
}

void CGXDLMSNtpSetup::SetClientKey(CGXByteBuffer& value)
{
    m_ClientKey = value;
}

// Returns amount of attributes.
int CGXDLMSNtpSetup::GetAttributeCount()
{
    return 7;
}

// Returns amount of methods.
int CGXDLMSNtpSetup::GetMethodCount()
{
    return 3;
}

void CGXDLMSNtpSetup::GetValues(std::vector<std::string>& values)
{
    std::stringstream sb;
    values.clear();
    std::string ln;
    GetLogicalName(ln);
    values.push_back(ln);
    values.push_back(GXHelpers::IntToString(m_Activated));
    values.push_back(m_ServerAddress);
    values.push_back(GXHelpers::IntToString(m_Port));
    values.push_back(GXHelpers::IntToString(m_Authentication));
    bool empty = true;
    sb << '[';
    for (std::map<uint32_t, CGXByteBuffer>::iterator it = m_Keys.begin(); it != m_Keys.end(); ++it)
    {
        if (!empty)
        {
            sb << ", ";
        }
        else
        {
            empty = false;
        }
        sb << '{';
        sb << it->first;
        sb << ", ";        
        sb << it->second.ToHexString();
        sb << '}';
    }
    sb << ']';
    values.push_back(sb.str());
    values.push_back(m_ClientKey.ToString());
}

void CGXDLMSNtpSetup::GetAttributeIndexToRead(bool all, std::vector<int>& attributes)
{
    //LN is static and read only once.
    if (all || CGXDLMSObject::IsLogicalNameEmpty(m_LN))
    {
        attributes.push_back(1);
    }
    //Activated
    if (all || CanRead(2))
    {
        attributes.push_back(2);
    }
    // ServerAddress
    if (all || CanRead(3)) {
        attributes.push_back(3);
    }
    // Port
    if (all || CanRead(4)) {
        attributes.push_back(4);
    }
    // Authentication
    if (all || CanRead(5)) {
        attributes.push_back(5);
    }
    // Keys
    if (all || CanRead(6)) {
        attributes.push_back(6);
    }
    // ClientKey
    if (all || CanRead(7)) {
        attributes.push_back(7);
    }
}

int CGXDLMSNtpSetup::GetDataType(int index, DLMS_DATA_TYPE& type)
{
    switch (index)
    {
    case 1:
        type = DLMS_DATA_TYPE_OCTET_STRING;
        break;
    case 2:
        type = DLMS_DATA_TYPE_BOOLEAN;
        break;
    case 3:
        type = DLMS_DATA_TYPE_OCTET_STRING;
        break;
    case 4:
        type = DLMS_DATA_TYPE_UINT16;
        break;
    case 5:
        type = DLMS_DATA_TYPE_ENUM;
        break;
    case 6:
        type = DLMS_DATA_TYPE_ARRAY;
        break;
    case 7:
        type = DLMS_DATA_TYPE_OCTET_STRING;
        break;
    default:
        return DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    return DLMS_ERROR_CODE_OK;
}

// Returns value of given attribute.
int CGXDLMSNtpSetup::GetValue(CGXDLMSSettings& settings, CGXDLMSValueEventArg& e)
{
    int ret = DLMS_ERROR_CODE_OK;
    switch (e.GetIndex())
    {
    case 1:
    {
        CGXDLMSVariant tmp;
        if ((ret = GetLogicalName(this, tmp)) == 0)
        {
            e.SetValue(tmp);
        }
        break;
    }
    case 2:
        e.SetValue(m_Activated);
        break;
    case 3:
        e.SetValue(m_ServerAddress);
        break;
    case 4:
        e.SetValue(m_Port);
        break;
    case 5:
        e.SetValue(m_Authentication);
        break;
    case 6:
    {
        CGXByteBuffer data;
        data.SetUInt8(DLMS_DATA_TYPE_ARRAY);
        GXHelpers::SetObjectCount((unsigned long)m_Keys.size(), data);
        for (std::map<uint32_t, CGXByteBuffer>::iterator it = m_Keys.begin(); it != m_Keys.end(); ++it)
        {
            if ((ret = data.SetUInt8(DLMS_DATA_TYPE_STRUCTURE)) != 0 ||
                (ret = data.SetUInt8(2)) != 0 ||
                (ret = data.SetUInt8(DLMS_DATA_TYPE_UINT32)) != 0 ||
                (ret = data.SetUInt32(it->first)) != 0 ||
                (ret = data.SetUInt8(DLMS_DATA_TYPE_OCTET_STRING)) != 0 ||
                (ret = GXHelpers::SetObjectCount(it->second.GetSize(), data)) != 0 ||
                (ret = data.Set(it->second.GetData(), it->second.GetSize())) != 0)
            {
                break;
            }
        }
        e.SetValue(data);
        break;
    }
    case 7:
        e.SetValue(m_ClientKey);
        break;
    default:
        return DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    return ret;
}

// Set value of given attribute.
int CGXDLMSNtpSetup::SetValue(CGXDLMSSettings& settings, CGXDLMSValueEventArg& e)
{
    int ret = DLMS_ERROR_CODE_OK;
    switch (e.GetIndex())
    {
    case 1:
        ret = SetLogicalName(this, e.GetValue());
        break;
    case 2:
        m_Activated = e.GetValue().boolVal;
        break;
    case 3:
        if (e.GetValue().vt == DLMS_DATA_TYPE_STRING)
        {
            m_ServerAddress = e.GetValue().ToString();
        }
        else
        {
            CGXDLMSVariant tmp;
            if ((ret = CGXDLMSClient::ChangeType(e.GetValue(), DLMS_DATA_TYPE_OCTET_STRING, tmp)) != 0)
            {
                return ret;
            }
            m_ServerAddress = tmp.ToString();
        }
        break;
    case 4:
        m_Port = e.GetValue().ToInteger();
        break;
    case 5:
        m_Authentication = (DLMS_NTP_AUTHENTICATION_METHOD)e.GetValue().ToInteger();
        break;
    case 6:
    {
        m_Keys.clear();
        for (std::vector<CGXDLMSVariant>::iterator it = e.GetValue().Arr.begin(); it != e.GetValue().Arr.end(); ++it)
        {
            CGXByteBuffer tmp;
            if ((ret = tmp.Set(it->Arr.at(1).byteArr, it->Arr.at(1).size)) != 0)
            {
                break;
            }
            m_Keys[it->Arr.at(0).ulVal] = tmp;
        }
        break;
    }
    case 7:
        e.GetValue().GetBytes(m_ClientKey);
        break;
    default:
        ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    return ret;
}

int CGXDLMSNtpSetup::Synchronize(CGXDLMSClient* client, std::vector<CGXByteBuffer>& reply)
{
    CGXDLMSVariant data((char)0);
    return client->Method(this, 1, data, DLMS_DATA_TYPE_ARRAY, reply);
}

int CGXDLMSNtpSetup::AddAuthenticationKey(CGXDLMSClient* client, uint32_t id, CGXByteBuffer& key, std::vector<CGXByteBuffer>& reply)
{
    int ret;
    CGXByteBuffer bb;
    if ((ret = bb.SetUInt8(DLMS_DATA_TYPE_STRUCTURE)) == 0 ||
        (ret = bb.SetUInt8(2)) == 0 ||
        (ret = bb.SetUInt8(DLMS_DATA_TYPE_UINT32)) == 0 ||
        (ret = bb.SetUInt32(id)) == 0 ||
        (ret = bb.SetUInt8(DLMS_DATA_TYPE_OCTET_STRING)) == 0 ||
        (ret = GXHelpers::SetObjectCount(key.GetSize(), bb)) == 0 ||
        (ret = bb.Set(key.GetData(), key.GetSize())) == 0)
    {
        CGXDLMSVariant tmp = bb;
        ret = client->Method(this, 2, tmp, DLMS_DATA_TYPE_ARRAY, reply);
    }
    return ret;
}

int CGXDLMSNtpSetup::DeleteAuthenticationKey(CGXDLMSClient* client, uint32_t id, std::vector<CGXByteBuffer>& reply)
{
    CGXDLMSVariant data = id;
    return client->Method(this, 3, data, DLMS_DATA_TYPE_ARRAY, reply);
}

int CGXDLMSNtpSetup::Invoke(CGXDLMSSettings& settings, CGXDLMSValueEventArg& e)
{
    if (e.GetIndex() == 1)
    {
        //Server must handle this. Do nothing...
    }
    else if (e.GetIndex() == 2)
    {
        CGXByteBuffer bb;
        if (e.GetParameters().vt != DLMS_DATA_TYPE_STRUCTURE)
        {
            return DLMS_ERROR_CODE_INCONSISTENT_CLASS_OR_OBJECT;
        }
        bb.Set(e.GetParameters().Arr[1].byteArr, e.GetParameters().Arr[1].GetSize());
        m_Keys[e.GetParameters().Arr[0].ulVal] = bb;
    }
    else if (e.GetIndex() == 3)
    {
        m_Keys.erase(e.GetParameters().ulVal);
    }
    else
    {
        e.SetError(DLMS_ERROR_CODE_READ_WRITE_DENIED);
    }
    return DLMS_ERROR_CODE_OK;
}
#endif //DLMS_IGNORE_NTP_SETUP