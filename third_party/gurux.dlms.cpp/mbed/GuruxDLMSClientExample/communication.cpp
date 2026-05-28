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

#include <mbed.h>

//---------------------------------------------------------------------------
//Gurux framework includes
//---------------------------------------------------------------------------
#include "communication.h"
#include "dlms/include/GXDLMSConverter.h"
#include "dlms/include/GXDLMSProfileGeneric.h"
#include "dlms/include/GXDLMSDemandRegister.h"
#include "dlms/include/GXDLMSDemandRegister.h"

RawSerial      pc(USBTX, USBRX);

void CGXCommunication::WriteValue(GX_TRACE_LEVEL trace, std::string line)
{
    if (trace > GX_TRACE_LEVEL_WARNING)
    {
        printf(line.c_str());
    }
}


CGXCommunication::CGXCommunication(CGXDLMSClient* pParser, GX_TRACE_LEVEL trace) :
    m_Parser(pParser), m_Trace(trace)
{
}

CGXCommunication::~CGXCommunication(void)
{
    Close();
}

//Close connection to the meter.
int CGXCommunication::Close()
{
    int ret;
    std::vector<CGXByteBuffer> data;
    CGXReplyData reply;
    if ((ret = m_Parser->ReleaseRequest(data)) != 0 ||
        (ret = ReadDataBlock(data, reply)) != 0)
    {
        //Show error but continue close.
    }
    if ((ret = m_Parser->DisconnectRequest(data)) != 0 ||
        (ret = ReadDataBlock(data, reply)) != 0)
    {
        //Show error but continue close.
    }
    return 0;
}

int CGXCommunication::Read(unsigned char eop, CGXByteBuffer& reply)
{
    bool bFound = false;
    int pos, lastReadIndex = 0;
    do
    {
        reply.SetUInt8(pc.getc());
        if (reply.GetSize() > 5)
        {
            //Some optical strobes can return extra bytes.
            for (pos = reply.GetSize() - 1; pos != lastReadIndex; --pos)
            {
                if (reply.GetData()[pos] == eop)
                {
                    bFound = true;
                    break;
                }
            }
            lastReadIndex = pos;
        }
    } while (!bFound);
    return DLMS_ERROR_CODE_OK;
}

//Initialize connection to the meter.
int CGXCommunication::InitializeConnection()
{
    if (m_Trace > GX_TRACE_LEVEL_WARNING)
    {
        printf("InitializeConnection\r\n");
    }
    std::vector<CGXByteBuffer> data;
    CGXReplyData reply;
    int ret = 0;
    //Get meter's send and receive buffers size.
    if ((ret = m_Parser->SNRMRequest(data)) != 0 ||
        (ret = ReadDataBlock(data, reply)) != 0 ||
        (ret = m_Parser->ParseUAResponse(reply.GetData())) != 0)
    {
        printf("SNRMRequest failed %d.\r\n", ret);
        return ret;
    }
    reply.Clear();
    if ((ret = m_Parser->AARQRequest(data)) != 0 ||
        (ret = ReadDataBlock(data, reply)) != 0 ||
        (ret = m_Parser->ParseAAREResponse(reply.GetData())) != 0)
    {
        if (ret == DLMS_ERROR_CODE_APPLICATION_CONTEXT_NAME_NOT_SUPPORTED)
        {
            printf("Use Logical Name referencing is wrong. Change it!\r\n");
            return ret;
        }
        printf("AARQRequest failed %d.\r\n", ret);
        return ret;
    }
    reply.Clear();
    // Get challenge Is HLS authentication is used.
    if (m_Parser->IsAuthenticationRequired())
    {
        if ((ret = m_Parser->GetApplicationAssociationRequest(data)) != 0 ||
            (ret = ReadDataBlock(data, reply)) != 0 ||
            (ret = m_Parser->ParseApplicationAssociationResponse(reply.GetData())) != 0)
        {
            return ret;
        }
    }
    return DLMS_ERROR_CODE_OK;
}

// Read DLMS Data frame from the device.
int CGXCommunication::ReadDLMSPacket(CGXByteBuffer& data, CGXReplyData& reply)
{
    int ret, pos;
    CGXByteBuffer bb;
    std::string tmp;
    if (data.GetSize() == 0)
    {
        return DLMS_ERROR_CODE_OK;
    }
    Now(tmp);
    tmp = "TX: " + tmp;
    tmp += "\t" + data.ToHexString();
    if (m_Trace > GX_TRACE_LEVEL_INFO)
    {
        printf("%s\r\n", tmp.c_str());
    }
    GXHelpers::Write("trace.txt", tmp + "\r\n");
    int len = data.GetSize();
    //Send data.
    for (pos = 0; pos != data.GetSize(); ++pos)
    {
        pc.putc(data.GetData()[pos]);
    }
    // Loop until whole DLMS packet is received.
    tmp = "";
    do
    {
        if (Read(0x7E, bb) != 0)
        {
            return DLMS_ERROR_CODE_SEND_FAILED;
        }
        if (tmp.size() == 0)
        {
            Now(tmp);
            tmp = "RX: " + tmp + "\t";
        }
        else
        {
            tmp += " ";
        }
        tmp += bb.ToHexString();
    } while ((ret = m_Parser->GetData(bb, reply)) == DLMS_ERROR_CODE_FALSE);
    tmp += "\r\n";
    if (m_Trace > GX_TRACE_LEVEL_INFO)
    {
        printf("%s", tmp.c_str());
    }
    GXHelpers::Write("trace.txt", tmp);
    if (ret == DLMS_ERROR_CODE_REJECTED)
    {
        ret = ReadDLMSPacket(data, reply);
    }
    return ret;
}

int CGXCommunication::ReadDataBlock(CGXByteBuffer& data, CGXReplyData& reply)
{
    //If ther is no data to send.
    if (data.GetSize() == 0)
    {
        return DLMS_ERROR_CODE_OK;
    }
    int ret;
    CGXByteBuffer bb;
    //Send data.
    if ((ret = ReadDLMSPacket(data, reply)) != DLMS_ERROR_CODE_OK)
    {
        return ret;
    }
    while (reply.IsMoreData())
    {
        bb.Clear();
        if ((ret = m_Parser->ReceiverReady(reply.GetMoreData(), bb)) != 0)
        {
            return ret;
        }
        if ((ret = ReadDLMSPacket(bb, reply)) != DLMS_ERROR_CODE_OK)
        {
            return ret;
        }
    }
    return DLMS_ERROR_CODE_OK;
}

int CGXCommunication::ReadDataBlock(std::vector<CGXByteBuffer>& data, CGXReplyData& reply)
{
    //If ther is no data to send.
    if (data.size() == 0)
    {
        return DLMS_ERROR_CODE_OK;
    }
    int ret;
    CGXByteBuffer bb;
    //Send data.
    for (std::vector<CGXByteBuffer>::iterator it = data.begin(); it != data.end(); ++it)
    {
        //Send data.
        if ((ret = ReadDLMSPacket(*it, reply)) != DLMS_ERROR_CODE_OK)
        {
            return ret;
        }
        while (reply.IsMoreData())
        {
            bb.Clear();
            if ((ret = m_Parser->ReceiverReady(reply.GetMoreData(), bb)) != 0)
            {
                return ret;
            }
            if ((ret = ReadDLMSPacket(bb, reply)) != DLMS_ERROR_CODE_OK)
            {
                return ret;
            }
        }
    }
    return DLMS_ERROR_CODE_OK;
}

//Get Association view.
int CGXCommunication::GetAssociationView()
{
    if (m_Trace > GX_TRACE_LEVEL_WARNING)
    {
        printf("GetAssociationView\r\n");
    }
    int ret;
    std::vector<CGXByteBuffer> data;
    CGXReplyData reply;
    if ((ret = m_Parser->GetObjectsRequest(data)) != 0 ||
        (ret = ReadDataBlock(data, reply)) != 0 ||
        (ret = m_Parser->ParseObjects(reply.GetData(), true)) != 0)
    {
        printf("GetObjects failed %d.\r\n", ret);
        return ret;
    }
    return DLMS_ERROR_CODE_OK;
}

//Read selected object.
int CGXCommunication::Read(CGXDLMSObject* pObject, int attributeIndex, std::string& value)
{
    value.clear();
    int ret;
    std::vector<CGXByteBuffer> data;
    CGXReplyData reply;
    //Read data from the meter.
    if ((ret = m_Parser->Read(pObject, attributeIndex, data)) != 0 ||
        (ret = ReadDataBlock(data, reply)) != 0 ||
        (ret = m_Parser->UpdateValue(*pObject, attributeIndex, reply.GetValue())) != 0)
    {
        return ret;
    }
    //Update data type.
    DLMS_DATA_TYPE type;
    if ((ret = pObject->GetDataType(attributeIndex, type)) != 0)
    {
        return ret;
    }
    if (type == DLMS_DATA_TYPE_NONE)
    {
        type = reply.GetValue().vt;
        if ((ret = pObject->SetDataType(attributeIndex, type)) != 0)
        {
            return ret;
        }
    }
    //Get read value as string.
    //Note! This is for example. It's faster if you handle read COSEM object directly.
    std::vector<std::string> values;
    pObject->GetValues(values);
    value = values[attributeIndex - 1];
    return DLMS_ERROR_CODE_OK;
}

int CGXCommunication::ReadList(
    std::vector<std::pair<CGXDLMSObject*, unsigned char> >& list)
{
    int ret;
    CGXReplyData reply;
    std::vector<CGXByteBuffer> data;
    //Get values from the meter.
    if ((ret = m_Parser->ReadList(list, data)) != 0)
    {
        return ret;
    }

    std::vector<CGXDLMSVariant> values;
    for (std::vector<CGXByteBuffer>::iterator it = data.begin(); it != data.end(); ++it)
    {
        if ((ret = ReadDataBlock(*it, reply)) != 0)
        {
            return ret;
        }
        if (list.size() != 1 && reply.GetValue().vt == DLMS_DATA_TYPE_ARRAY)
        {
            values.insert(values.end(), reply.GetValue().Arr.begin(), reply.GetValue().Arr.end());
        }
        else if (reply.GetValue().vt != DLMS_DATA_TYPE_NONE)
        {
            // Value is null if data is send multiple frames.
            values.push_back(reply.GetValue());
        }
        reply.Clear();
    }

    if (values.size() != list.size())
    {
        //Invalid reply. Read items count do not match.
        return DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    return m_Parser->UpdateValues(list, values);
}

//Write selected object.
int CGXCommunication::Write(CGXDLMSObject* pObject, int attributeIndex, CGXDLMSVariant& value)
{
    int ret;
    std::vector<CGXByteBuffer> data;
    CGXReplyData reply;
    //Get meter's send and receive buffers size.
    if ((ret = m_Parser->Write(pObject, attributeIndex, value, data)) != 0 ||
        (ret = ReadDataBlock(data, reply)) != 0)
    {
        return ret;
    }
    return DLMS_ERROR_CODE_OK;
}

//Write selected object.
int CGXCommunication::Write(CGXDLMSObject* pObject, int attributeIndex)
{
    int ret;
    std::vector<CGXByteBuffer> data;
    CGXReplyData reply;
    //Get meter's send and receive buffers size.
    if ((ret = m_Parser->Write(pObject, attributeIndex, data)) != 0 ||
        (ret = ReadDataBlock(data, reply)) != 0)
    {
        return ret;
    }
    return DLMS_ERROR_CODE_OK;
}

int CGXCommunication::Method(CGXDLMSObject* pObject, int attributeIndex, CGXDLMSVariant& value)
{
    int ret;
    std::vector<CGXByteBuffer> data;
    CGXReplyData reply;
    //Get meter's send and receive buffers size.
    if ((ret = m_Parser->Method(pObject, attributeIndex, value, data)) != 0 ||
        (ret = ReadDataBlock(data, reply)) != 0)
    {
        return ret;
    }
    return DLMS_ERROR_CODE_OK;
}

int CGXCommunication::ReadRowsByRange(
    CGXDLMSProfileGeneric* pObject,
    CGXDateTime& start,
    CGXDateTime& end,
    CGXDLMSVariant& rows)
{
    return ReadRowsByRange(pObject, &start.GetValue(), &end.GetValue(), rows);
}

int CGXCommunication::ReadRowsByRange(
    CGXDLMSProfileGeneric* pObject,
    struct tm* start,
    struct tm* end,
    CGXDLMSVariant& rows)
{
    rows.Clear();
    int ret;
    std::vector<CGXByteBuffer> data;
    CGXReplyData reply;
    if ((ret = m_Parser->ReadRowsByRange(pObject, start, end, data)) != 0 ||
        (ret = ReadDataBlock(data, reply)) != 0 ||
        (ret = m_Parser->UpdateValue(*pObject, 2, reply.GetValue())) != 0)
    {
        return ret;
    }
    //Get rows value as string.
    //Note! This is for example. It's faster if you handle read COSEM object directly.
    std::vector<std::string> values;
    pObject->GetValues(values);
    rows = values[2 - 1];
    return DLMS_ERROR_CODE_OK;
}

int CGXCommunication::ReadRowsByEntry(
    CGXDLMSProfileGeneric* pObject,
    unsigned int index,
    unsigned int count,
    CGXDLMSVariant& rows)
{
    rows.Clear();
    int ret;
    std::vector<CGXByteBuffer> data;
    CGXReplyData reply;
    if ((ret = m_Parser->ReadRowsByEntry(pObject, index, count, data)) != 0 ||
        (ret = ReadDataBlock(data, reply)) != 0 ||
        (ret = m_Parser->UpdateValue(*pObject, 2, reply.GetValue())) != 0)
    {
        return ret;
    }
    //Get rows value as string.
    //Note! This is for example. It's faster if you handle read COSEM object directly.
    std::vector<std::string> values;
    pObject->GetValues(values);
    rows = values[2 - 1];
    return DLMS_ERROR_CODE_OK;
}

int CGXCommunication::ReadScalerAndUnits()
{
    int ret = 0;
    std::string str;
    std::string ln;
    std::vector<std::pair<CGXDLMSObject*, unsigned char> > list;
    if ((m_Parser->GetNegotiatedConformance() & DLMS_CONFORMANCE_MULTIPLE_REFERENCES) != 0)
    {
        // Read scalers and units from the device.
        for (std::vector<CGXDLMSObject*>::iterator it = m_Parser->GetObjects().begin(); it != m_Parser->GetObjects().end(); ++it)
        {
            if ((*it)->GetObjectType() == DLMS_OBJECT_TYPE_REGISTER ||
                (*it)->GetObjectType() == DLMS_OBJECT_TYPE_EXTENDED_REGISTER)
            {
                list.push_back(std::make_pair(*it, 3));
            }
            else if ((*it)->GetObjectType() == DLMS_OBJECT_TYPE_DEMAND_REGISTER)
            {
                list.push_back(std::make_pair(*it, 4));
            }
        }
        if ((ret = ReadList(list)) != 0)
        {
            printf("Err! Failed to read register: %s", CGXDLMSConverter::GetErrorMessage(ret));
            m_Parser->SetNegotiatedConformance((DLMS_CONFORMANCE)(m_Parser->GetNegotiatedConformance() & ~DLMS_CONFORMANCE_MULTIPLE_REFERENCES));
        }
    }
    if ((m_Parser->GetNegotiatedConformance() & DLMS_CONFORMANCE_MULTIPLE_REFERENCES) == 0)
    {
        //If readlist is not supported read one value at the time.
        for (std::vector<CGXDLMSObject*>::iterator it = m_Parser->GetObjects().begin(); it != m_Parser->GetObjects().end(); ++it)
        {
            if ((*it)->GetObjectType() == DLMS_OBJECT_TYPE_REGISTER ||
                (*it)->GetObjectType() == DLMS_OBJECT_TYPE_EXTENDED_REGISTER)
            {
                (*it)->GetLogicalName(ln);
                WriteValue(m_Trace, ln.c_str());
                if ((ret = Read(*it, 3, str)) != 0)
                {
                    printf("Err! Failed to read register: %s %s", ln.c_str(), CGXDLMSConverter::GetErrorMessage(ret));
                    //Continue reading.
                    continue;
                }
            }
            else if ((*it)->GetObjectType() == DLMS_OBJECT_TYPE_DEMAND_REGISTER)
            {
                (*it)->GetLogicalName(ln);
                WriteValue(m_Trace, ln.c_str());
                if ((ret = Read(*it, 4, str)) != 0)
                {
                    printf("Err! Failed to read register: %s %s", ln.c_str(), CGXDLMSConverter::GetErrorMessage(ret));
                    //Continue reading.
                    continue;
                }
            }
        }
    }
    return ret;
}

int CGXCommunication::GetProfileGenericColumns()
{
    int ret = 0;
    std::string ln;
    std::string value;
    //Read columns.
    CGXDLMSObjectCollection profileGenerics;
    m_Parser->GetObjects().GetObjects(DLMS_OBJECT_TYPE_PROFILE_GENERIC, profileGenerics);
    for (std::vector<CGXDLMSObject*>::iterator it = profileGenerics.begin(); it != profileGenerics.end(); ++it)
    {
        //Read Profile Generic columns first.
        CGXDLMSProfileGeneric* pg = (CGXDLMSProfileGeneric*)*it;
        if ((ret = Read(pg, 3, value)) != 0)
        {
            printf("Err! Failed to read columns: %s", CGXDLMSConverter::GetErrorMessage(ret));
            //Continue reading.
            continue;
        }

        //Update columns scalers.
        DLMS_OBJECT_TYPE ot;
        CGXDLMSObject* obj;
        for (std::vector<std::pair<CGXDLMSObject*, CGXDLMSCaptureObject*> >::iterator it2 = pg->GetCaptureObjects().begin(); it2 != pg->GetCaptureObjects().end(); ++it2)
        {
            ot = it2->first->GetObjectType();
            if (ot == DLMS_OBJECT_TYPE_REGISTER ||
                ot == DLMS_OBJECT_TYPE_EXTENDED_REGISTER ||
                ot == DLMS_OBJECT_TYPE_DEMAND_REGISTER)
            {
                it2->first->GetLogicalName(ln);
                obj = m_Parser->GetObjects().FindByLN(ot, ln);
                if (obj != NULL)
                {
                    if (ot == DLMS_OBJECT_TYPE_REGISTER || ot == DLMS_OBJECT_TYPE_EXTENDED_REGISTER)
                    {
                        ((CGXDLMSRegister*)it2->first)->SetScaler(((CGXDLMSRegister*)obj)->GetScaler());
                        ((CGXDLMSRegister*)it2->first)->SetUnit(((CGXDLMSRegister*)obj)->GetUnit());
                    }
                    else if (ot == DLMS_OBJECT_TYPE_DEMAND_REGISTER)
                    {
                        ((CGXDLMSDemandRegister*)it2->first)->SetScaler(((CGXDLMSDemandRegister*)obj)->GetScaler());
                        ((CGXDLMSDemandRegister*)it2->first)->SetUnit(((CGXDLMSDemandRegister*)obj)->GetUnit());
                    }
                }
            }
        }
        WriteValue(m_Trace, "Profile Generic " + (*it)->GetName().ToString() + " Columns:\r\n");
        std::string str;
        for (std::vector<std::pair<CGXDLMSObject*, CGXDLMSCaptureObject*> >::iterator it2 = pg->GetCaptureObjects().begin(); it2 != pg->GetCaptureObjects().end(); ++it2)
        {
            if (str.size() != 0)
            {
                str.append(" | ");
            }
            str.append((*it2).first->GetName().ToString());
            str.append(" ");
            str.append((*it2).first->GetDescription());
        }
        str.append("\r\n");
        WriteValue(m_Trace, str);
    }
    return ret;
}

int CGXCommunication::GetReadOut()
{
    int ret = 0;
    char buff[200];
    std::string value;
    for (std::vector<CGXDLMSObject*>::iterator it = m_Parser->GetObjects().begin(); it != m_Parser->GetObjects().end(); ++it)
    {
        // Profile generics are read later because they are special cases.
        // (There might be so lots of data and we so not want waste time to read all the data.)
        if ((*it)->GetObjectType() == DLMS_OBJECT_TYPE_PROFILE_GENERIC)
        {
            continue;
        }

#if _MSC_VER > 1000
        sprintf_s(buff, 200, "-------- Reading %s %s %s\r\n", CGXDLMSClient::ObjectTypeToString((*it)->GetObjectType()).c_str(), (*it)->GetName().ToString().c_str(), (*it)->GetDescription().c_str());
#else
        sprintf(buff, "-------- Reading %s %s %s\r\n", CGXDLMSClient::ObjectTypeToString((*it)->GetObjectType()).c_str(), (*it)->GetName().ToString().c_str(), (*it)->GetDescription().c_str());
#endif

        WriteValue(m_Trace, buff);
        std::vector<int> attributes;
        (*it)->GetAttributeIndexToRead(attributes);
        for (std::vector<int>::iterator pos = attributes.begin(); pos != attributes.end(); ++pos)
        {
            value.clear();
            if ((ret = Read(*it, *pos, value)) != DLMS_ERROR_CODE_OK)
            {
#if _MSC_VER > 1000
                sprintf_s(buff, 100, "Error! Index: %d %s\r\n", *pos, CGXDLMSConverter::GetErrorMessage(ret));
#else
                sprintf(buff, "Error! Index: %d read failed: %s\r\n", *pos, CGXDLMSConverter::GetErrorMessage(ret));
#endif
                WriteValue(GX_TRACE_LEVEL_ERROR, buff);
                //Continue reading.
            }
            else
            {
#if _MSC_VER > 1000
                sprintf_s(buff, 100, "Index: %d Value: ", *pos);
#else
                sprintf(buff, "Index: %d Value: ", *pos);
#endif
                WriteValue(m_Trace, buff);
                WriteValue(m_Trace, value.c_str());
                WriteValue(m_Trace, "\r\n");
                }
            }
        }
    return ret;
}

int CGXCommunication::GetProfileGenerics()
{
    char buff[200];
    int ret = 0;
    std::string str;
    std::string value;
    //Find profile generics and read them.
    CGXDLMSObjectCollection pgs;
    m_Parser->GetObjects().GetObjects(DLMS_OBJECT_TYPE_PROFILE_GENERIC, pgs);
    for (std::vector<CGXDLMSObject*>::iterator it = pgs.begin(); it != pgs.end(); ++it)
    {
#if _MSC_VER > 1000
        sprintf_s(buff, 200, "-------- Reading %s %s %s\r\n", CGXDLMSClient::ObjectTypeToString((*it)->GetObjectType()).c_str(), (*it)->GetName().ToString().c_str(), (*it)->GetDescription().c_str());
#else
        sprintf(buff, "-------- Reading %s %s %s\r\n", CGXDLMSClient::ObjectTypeToString((*it)->GetObjectType()).c_str(), (*it)->GetName().ToString().c_str(), (*it)->GetDescription().c_str());
#endif

        WriteValue(m_Trace, buff);

        if ((ret = Read(*it, 7, value)) != DLMS_ERROR_CODE_OK)
        {
#if _MSC_VER > 1000
            sprintf_s(buff, 100, "Error! Index: %d: %s\r\n", 7, CGXDLMSConverter::GetErrorMessage(ret));
#else
            sprintf(buff, "Error! Index: %d: %s\r\n", 7, CGXDLMSConverter::GetErrorMessage(ret));
#endif
            WriteValue(GX_TRACE_LEVEL_ERROR, buff);
            //Continue reading.
        }

        std::string entriesInUse = value;
        if ((ret = Read(*it, 8, value)) != DLMS_ERROR_CODE_OK)
        {
#if _MSC_VER > 1000
            sprintf_s(buff, 100, "Error! Index: %d: %s\r\n", 8, CGXDLMSConverter::GetErrorMessage(ret));
#else
            sprintf(buff, "Error! Index: %d: %s\r\n", 8, CGXDLMSConverter::GetErrorMessage(ret));
#endif
            WriteValue(GX_TRACE_LEVEL_ERROR, buff);
            //Continue reading.
        }
        std::string entries = value;
        str = "Entries: ";
        str += entriesInUse;
        str += "/";
        str += entries;
        str += "\r\n";
        WriteValue(m_Trace, str);
        //If there are no columns or rows.
        if (((CGXDLMSProfileGeneric*)*it)->GetEntriesInUse() == 0 || ((CGXDLMSProfileGeneric*)*it)->GetCaptureObjects().size() == 0)
        {
            continue;
        }
        //All meters are not supporting parameterized read.
        CGXDLMSVariant rows;
        if ((m_Parser->GetNegotiatedConformance() & (DLMS_CONFORMANCE_PARAMETERIZED_ACCESS | DLMS_CONFORMANCE_SELECTIVE_ACCESS)) != 0)
        {
            //Read first row from Profile Generic.
            if ((ret = ReadRowsByEntry((CGXDLMSProfileGeneric*)*it, 1, 1, rows)) != 0)
            {
                str = "Error! Failed to read first row:";
                str += CGXDLMSConverter::GetErrorMessage(ret);
                str += "\r\n";
                WriteValue(GX_TRACE_LEVEL_ERROR, str);
                //Continue reading.
            }
            else
            {
                //////////////////////////////////////////////////////////////////////////////
                //Show rows.
                WriteValue(m_Trace, rows.ToString());
            }
        }

        //All meters are not supporting parameterized read.
        if ((m_Parser->GetNegotiatedConformance() & (DLMS_CONFORMANCE_PARAMETERIZED_ACCESS | DLMS_CONFORMANCE_SELECTIVE_ACCESS)) != 0)
        {
            CGXDateTime start = CGXDateTime::Now();
            start.ResetTime();
            CGXDateTime end = CGXDateTime::Now();
            if ((ret = ReadRowsByRange((CGXDLMSProfileGeneric*)(*it), start, end, rows)) != 0)
            {
                str = "Error! Failed to read last day:";
                str += CGXDLMSConverter::GetErrorMessage(ret);
                str += "\r\n";
                WriteValue(GX_TRACE_LEVEL_ERROR, str);
                //Continue reading.
            }
            else
            {
                //////////////////////////////////////////////////////////////////////////////
                //Show rows.
                WriteValue(m_Trace, rows.ToString());
            }
        }
    }
    return ret;
}

int CGXCommunication::ReadAll()
{
    int ret;
    if ((ret = InitializeConnection()) != 0 ||
        (ret = GetAssociationView()) != 0 ||
        // Read Scalers and units from the register objects.
        (ret = ReadScalerAndUnits()) != 0 ||
        // Read Profile Generic columns.
        (ret = GetProfileGenericColumns()) != 0 ||
        // Read all attributes from all objects.
        (ret = GetReadOut()) != 0 ||
        // Read historical data.
        (ret = GetProfileGenerics()) != 0)
    {
    }
    Close();
    return ret;
}