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

#ifndef GXASN1BITSTRING_H
#define GXASN1BITSTRING_H

#include "GXHelpers.h"
#include "GXBytebuffer.h"
#include "GXByteArray.h"
#include "GXAsn1Base.h"

class CGXAsn1BitString : public CGXAsn1Base
{
private:
    /**
    * Number of extra bits at the end of the string.
    */
    unsigned char m_PadBits;

    /**
     * Bit string.
     */
    CGXByteBuffer m_Value;

    /**
     * Append zeroes to the buffer.
     *
     * count: Amount of zero.
     */
    static void AppendZeros(std::string& sb, int count)
    {
        for (int pos = 0; pos != count; ++pos)
        {
            sb += '0';
        }
    }

public:

    /**
     * Constructor.
     */
    CGXAsn1BitString()
    {
        m_PadBits = 0;
    }

    /**
     * Constructor
     *
     * bitString
     *            Bit string.
     */
    CGXAsn1BitString(std::string& bitString)
    {
        m_PadBits = 8 - (bitString.length() % 8);
        if (m_PadBits == 8)
        {
            m_PadBits = 0;
        }
        std::string sb = bitString;
        AppendZeros(sb, m_PadBits);
        CGXDLMSVariant value = sb;
        m_Value.Capacity((unsigned long)(sb.length() / 8));
        GXHelpers::SetBitString(m_Value, value, false);
    }

    /**
     * Constructor
     *
     * str
     *            Bit string.
     * padCount
     *            Number of extra bits at the end of the string.
     */
    CGXAsn1BitString(CGXByteBuffer& str, int padCount)
    {
        m_Value = str;
        m_PadBits = padCount;
    }

    /**
     * Constructor
     *
     * str
     *            Bit string.
     * padCount
     *            Number of extra bits at the end of the string.
     */
    CGXAsn1BitString(CGXByteArray& str, int padCount)
    {
        str.ToByteBuffer(m_Value);
        m_PadBits = padCount;
    }
    
    /**
     * Constructor
     *
     * str
     *            Bit string.
     */
    CGXAsn1BitString(CGXByteBuffer& str) 
    {
        str.GetUInt8(&m_PadBits);
        str.SubArray(1, str.Available(), m_Value);
    }

    /**
     * Returns Bit string.
     */
    CGXByteBuffer& GetValue()
    {
        return m_Value;
    }

    /**
     * Returns Number of extra bits at the end of the string.
     */
    int GetPadBits()
    {
        return m_PadBits;
    }

    /**
     * Returns Number of extra bits at the end of the string.
     */
    int Length()
    {
        return (8 * m_Value.GetSize()) - m_PadBits;
    }

    std::string ToString()
    {
        if (m_Value.GetSize() == 0)
        {
            return "";
        }
        std::string str;
        str += std::to_string((8 * m_Value.GetSize()) - m_PadBits);
        str += " bit ";
        str += AsString();
        return str;
    }

    std::string AsString()
    {
        if (m_Value.GetSize() == 0)
        {
            return "";
        }
        CGXByteBuffer bb;
        for (unsigned long pos = 0; pos != m_Value.GetSize(); ++pos)
        {
            GXHelpers::ToBitString(bb, m_Value.GetData()[pos], 8);
        }
        bb.SetSize(bb.GetSize() - m_PadBits);
        return bb.ToString();
    }

    /**
     * Returns bit-string value as a integer.
     **/
    int ToInteger()
    {
        int ret = 0;
        int bytePos = 0;
        for (unsigned long pos = 0; pos != m_Value.GetSize(); ++pos)
        {
            ret |= (int)(GXHelpers::SwapBits(m_Value.GetData()[pos]) << bytePos);
            bytePos += 8;
        }
        return ret;
    }
};

#endif //GXASN1BITSTRING_H
