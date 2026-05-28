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


#ifndef GXCOMMUNICATION_H
#define GXCOMMUNICATION_H

#include <stdio.h>
#include "dlms/include/GXDLMSSecureClient.h"

class CGXCommunication
{
    GX_TRACE_LEVEL m_Trace;
    CGXDLMSClient* m_Parser;
    int Read(unsigned char eop, CGXByteBuffer& reply);
public:
    void WriteValue(GX_TRACE_LEVEL trace, std::string line);
public:

    CGXCommunication(CGXDLMSClient* pCosem, GX_TRACE_LEVEL trace);
    ~CGXCommunication(void);

    //Get current time as a string.
    static inline void Now(std::string& str)
    {
        time_t tm1 = time(NULL);
        struct tm dt;
        char tmp[10];
        int ret;
        dt = *localtime(&tm1);
        ret = sprintf(tmp, "%.2d:%.2d:%.2d", dt.tm_hour, dt.tm_min, dt.tm_sec);
        str.append(tmp, ret);
    }


    int Close();

    int ReadDLMSPacket(CGXByteBuffer& data, CGXReplyData& reply);
    int ReadDataBlock(CGXByteBuffer& data, CGXReplyData& reply);
    int ReadDataBlock(std::vector<CGXByteBuffer>& data, CGXReplyData& reply);

    int InitializeConnection();
    int GetAssociationView();

    //Read selected object.
    int Read(CGXDLMSObject* pObject, int attributeIndex, std::string& value);

    int ReadList(
        std::vector<std::pair<CGXDLMSObject*, unsigned char> >& list);

    //Write selected object.
    int Write(
        CGXDLMSObject* pObject,
        int attributeIndex,
        CGXDLMSVariant& value);

    //Write selected object.
    int Write(
        CGXDLMSObject* pObject,
        int attributeIndex);

    //Call action of selected object.
    int Method(
        CGXDLMSObject* pObject,
        int ActionIndex,
        CGXDLMSVariant& value);


    int ReadRowsByRange(
        CGXDLMSProfileGeneric* pObject,
        CGXDateTime& start,
        CGXDateTime& end,
        CGXDLMSVariant& rows);

    int ReadRowsByRange(
        CGXDLMSProfileGeneric* pObject,
        struct tm* start,
        struct tm* end,
        CGXDLMSVariant& rows);

    int ReadRowsByEntry(
        CGXDLMSProfileGeneric* pObject,
        unsigned int Index,
        unsigned int Count,
        CGXDLMSVariant& rows);

    int ReadScalerAndUnits();
    int GetProfileGenericColumns();

    int GetReadOut();
    int GetProfileGenerics();

    /*
    * Read all objects from the meter. This is only example. Usually there is
    * no need to read all data from the meter.
    */
    int ReadAll();
};
#endif //GXCOMMUNICATION_H