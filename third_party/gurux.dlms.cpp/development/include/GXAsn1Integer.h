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

#ifndef GXASN1INTEGER_H
#define GXASN1INTEGER_H

#include "GXBytebuffer.h"
#include "GXBigInteger.h"
#include "GXHelpers.h"
#include "GXAsn1Base.h"

/**
 ASN1 bit string.
*/
class CGXAsn1Integer : public CGXAsn1Base
{
private:
    CGXByteBuffer m_Value;
public:
    /**
     Value.
    */
    CGXByteBuffer& GetValue()
    {
        return m_Value;
    }
    /**
     Value.
    */
    void SetValue(CGXByteBuffer& value)
    {
        m_Value = value;
    }

    /////////////////////////////////////////////////////////////////////////////
    // Constructor.
    /////////////////////////////////////////////////////////////////////////////
    CGXAsn1Integer()
    {

    }

    /**
     Constructor
     @param data Integer.
    */
    CGXAsn1Integer(CGXByteBuffer& value)
    {
        m_Value = value;
    }

    /**
     Constructor
      data: Data.
      index: Index.
      count: Count.
    */
    CGXAsn1Integer(CGXByteBuffer& data, int index, int count)
    {
        data.SubArray(index, count, m_Value);
    }

    /*
     Constructor
     data: Integer.
    */
    CGXAsn1Integer(std::string& data)
    {
        m_Value.SetUInt64(atoll(data.c_str()));
    }

    /**
     Constructor
     data: Integer.
    */
    CGXAsn1Integer(uint64_t data)
    {
        m_Value.SetUInt64(data);
    }

    /** @return Get integer value as int.
    */
    CGXBigInteger ToBigInteger()
    {
        return CGXBigInteger(m_Value);
    }

    /*Get integer value as byte.*/
    unsigned char ToByte()
    {
        unsigned char value;
        m_Value.GetUInt8(&value);
        return value;
    }

    /*Get integer value as short.*/
    short ToShort()
    {
        short value;
        m_Value.GetInt16(&value);
        return value;
    }

    /*Get integer value as int.*/
    long ToInt()
    {
        long value;
        m_Value.GetInt32(&value);
        return value;
    }

    /** Returns: Integer value as long.
    */
    long long ToLong()
    {
        long long value;
        m_Value.GetInt64(&value);
        return value;
    }

    std::string ToString()
    {
        std::string str;
        switch (m_Value.GetSize())
        {
        case 1:
            str = GXHelpers::IntToString(ToByte());
            break;
        case 2:
            str = GXHelpers::IntToString(ToShort());
            break;
        case 4:
            str = GXHelpers::IntToString(ToInt());
            break;
        case 8:
            str = std::to_string(ToLong());
            break;
        default:
            str = "Invalid value.";
            break;
        }
        return str;
    }
};

#endif //GXASN1INTEGER_H
