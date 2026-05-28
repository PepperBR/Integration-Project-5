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

#include <string.h>
#include "../include/GXBigInteger.h"
#include "../include/GXHelpers.h"

// Constructor.
CGXBigInteger::CGXBigInteger()
{
    m_Data = NULL;
    m_Capacity = 0;
    m_Count = 0;
    m_IsNegative = false;
}

CGXBigInteger::CGXBigInteger(std::string& value) : CGXBigInteger()
{
    m_IsNegative = value.rfind("-", 0) == 0;
    CGXByteBuffer bb;
    bb.SetHexString(value);
    FromByteBuffer(bb);
}

CGXBigInteger::CGXBigInteger(uint64_t value) : CGXBigInteger()
{
    Add((uint32_t)value);
    value >>= 32;
    Add((uint32_t)value);
}

CGXBigInteger::CGXBigInteger(uint32_t value) : CGXBigInteger()
{
    Add(value);
}

CGXBigInteger::CGXBigInteger(int value) : CGXBigInteger()
{
    Add(value);
}

CGXBigInteger::CGXBigInteger(const uint32_t* values, uint16_t count) : CGXBigInteger()
{
    AddRange(values, count);
}

CGXBigInteger::CGXBigInteger(const unsigned char* values, uint16_t count) : CGXBigInteger()
{
    CGXByteBuffer bb;
    bb.Set(values, count);
    FromByteBuffer(bb);
}

CGXBigInteger::CGXBigInteger(CGXByteBuffer& value) : CGXBigInteger()
{
    FromByteBuffer(value);
}

CGXBigInteger::CGXBigInteger(CGXByteArray& value) : CGXBigInteger()
{
    FromByteBuffer(value);
}

CGXBigInteger::CGXBigInteger(CGXBigInteger* value) : CGXBigInteger()
{
    AddRange(value->m_Data, value->m_Count);
    m_IsNegative = value->m_IsNegative;
}

CGXBigInteger::CGXBigInteger(const CGXBigInteger& value) : CGXBigInteger()
{
    AddRange(value.m_Data, value.m_Count);
    m_IsNegative = value.m_IsNegative;
}

CGXBigInteger& CGXBigInteger::operator=(const CGXBigInteger& value)
{
    Clear();
    AddRange(value.m_Data, value.m_Count);
    m_IsNegative = value.m_IsNegative;
    return *this;
}

CGXBigInteger::~CGXBigInteger()
{
    free(m_Data);
    m_Data = NULL;
}

int CGXBigInteger::Capacity(uint16_t value)
{
    if (!(m_Capacity > value) && value != 0)
    {
        uint32_t* tmp;
        if (m_Data == NULL)
        {
            tmp = (uint32_t*)malloc(sizeof(uint32_t) * value);
        }
        else
        {
            tmp = (uint32_t*)realloc(m_Data, sizeof(uint32_t) * value);
        }
        //If not enought memory available.
        if (tmp == NULL)
        {
            return DLMS_ERROR_CODE_OUTOFMEMORY;
        }
        m_Capacity = value;
        m_Data = tmp;
    }
    return 0;
}

int CGXBigInteger::FromByteBuffer(CGXByteArray& value)
{
    int ret = 0;
    unsigned long tmp;
    unsigned char ch;
    unsigned short ival;
    Clear();
    Capacity((uint16_t) (value.GetSize() / 4));
    unsigned long index = 0;
    for (int pos = value.GetSize() - 4; pos > -1; pos = pos - 4)
    {
        ret = value.GetUInt32(pos, &tmp);
        if (ret != 0)
        {
            break;
        }
        Add(tmp);
        index += 4;
    }
    if (ret == 0)
    {
        switch (value.GetSize() % 4)
        {
        case 1:
            ret = value.GetUInt8(index, &ch);
            if (ret != 0)
            {
                break;
            }
            Add(ch);
            break;
        case 2:
            ret = value.GetUInt16(index, &ival);
            if (ret != 0)
            {
                break;
            }
            Add(ival);
            break;
        case 3:
            ret = value.GetUInt8(index, &ch);
            if (ret != 0)
            {
                break;
            }
            tmp = ch;
            tmp <<= 16;
            ret = value.GetUInt16(index, &ival);
            if (ret != 0)
            {
                break;
            }
            tmp |= ival;
            Add(tmp);
            break;
        default:
            break;
        }
    }
    return ret;
}

int CGXBigInteger::FromByteBuffer(CGXByteBuffer& value)
{
    int ret = 0;
    unsigned long tmp;
    unsigned char ch;
    unsigned short ival;
    Clear();
    Capacity((uint16_t)(value.GetSize() / 4));
    for (int pos = value.GetSize() - 4; pos > -1; pos = pos - 4)
    {
        ret = value.GetUInt32(pos, &tmp);
        if (ret != 0)
        {
            break;
        }
        Add(tmp);
    }
    if (ret == 0)
    {
        switch (value.GetSize() % 4)
        {
        case 1:
            ret = value.GetUInt8(&ch);
            if (ret != 0)
            {
                break;
            }
            Add(ch);
            break;
        case 2:
            ret = value.GetUInt16(&ival);
            if (ret != 0)
            {
                break;
            }
            Add(ival);
            break;
        case 3:
            ret = value.GetUInt8(&ch);
            if (ret != 0)
            {
                break;
            }
            tmp = ch;
            tmp <<= 16;
            ret = value.GetUInt16(&ival);
            if (ret != 0)
            {
                break;
            }
            tmp |= ival;
            Add(tmp);
            break;
        default:
            break;
        }
    }
    return ret;
}

void CGXBigInteger::SetIsNegative(bool value)
{
    m_IsNegative = value;
}

int CGXBigInteger::Add(const uint32_t value)
{
    return AddRange(&value, 1);
}

int CGXBigInteger::AddRange(const uint32_t* values, uint16_t count)
{
    if ((m_Count + count) > m_Capacity)
    {
        int ret = Capacity(count + m_Count);
        if (ret != 0)
        {
            return ret;
        }
    }
    memcpy(m_Data + m_Count, values, sizeof(uint32_t) * count);
    m_Count += count;
    return 0;
}

int CGXBigInteger::AddValue(uint32_t* list,
    uint16_t length,
    uint32_t value,
    uint16_t index)
{
    uint64_t tmp;
    if (index < length)
    {
        tmp = list[index];
        tmp += value;
        list[index] = (uint32_t)tmp;
        uint32_t reminer = (uint32_t)(tmp >> 32);
        if (reminer > 0)
        {
            AddValue(list, length, reminer, index + 1);
        }
    }
    else
    {
        return DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    return 0;
}

uint16_t CGXBigInteger::GetCount()
{
    return m_Count;
}

void CGXBigInteger::SetCount(uint16_t value)
{
    m_Count = value;
}

bool CGXBigInteger::IsNegative()
{
    return m_IsNegative;
}

bool CGXBigInteger::IsZero()
{
    return m_Count == 0 ||
        (m_Count == 1 && m_Data[0] == 0);
}

bool CGXBigInteger::IsEven()
{
    return m_Count != 0 &&
        m_Data[0] % 2 == 0;
}

bool CGXBigInteger::IsOne()
{
    return m_Count == 1 && m_Data[0] == 1;
}

int CGXBigInteger::ToArray(CGXByteBuffer& data,
    bool removeLeadingZeroes)
{
    int ret = 0;
    uint32_t pos;
    uint32_t value;
    uint32_t zeroIndex = -1;
    for (pos = 0; pos != m_Count; ++pos)
    {
        value = m_Data[pos];
        if (value == 0)
        {
            zeroIndex = pos;
        }
        else
        {
            zeroIndex = 0xFFFFFFFF;
        }
        data.SetUInt8(value);
        data.SetUInt8(value >> 8);
        data.SetUInt8(value >> 16);
        data.SetUInt8(value >> 24);
    }
    //Remove leading zeroes.
    if (removeLeadingZeroes && zeroIndex != 0xFFFFFFFF)
    {
        data.SetSize(zeroIndex * 4);
    }
    data.Reverse(0, data.GetSize());
    return ret;
}

int CGXBigInteger::ToArray(CGXByteBuffer& data)
{
    return ToArray(data, true);
}

int CGXBigInteger::ToArray(uint32_t start,
    uint32_t size,
    CGXByteBuffer& data)
{
    uint32_t pos;
    for (pos = start; pos != size; ++pos)
    {
        data.SetUInt32(m_Data[pos]);
    }
    return 0;
}

void CGXBigInteger::Or(CGXBigInteger& value)
{
    uint32_t pos;
    if (m_Count < value.m_Count)
    {
        m_Count = value.m_Count;
    }
    for (pos = 0; pos < value.m_Count; ++pos)
    {
        m_Data[pos] |= value.m_Data[pos];
    }
}

int CGXBigInteger::Add(CGXBigInteger& value)
{
    if (value.IsNegative())
    {
        value.SetIsNegative(false);
        Sub(value);
        value.SetIsNegative(true);
    }
    else
    {
        while (m_Count < value.m_Count)
        {
            Add((uint32_t)0);
        }
        uint64_t overflow = 0;
        for (uint32_t pos = 0; pos != m_Count; ++pos)
        {
            uint64_t tmp = m_Data[pos];
            if (pos < value.m_Count)
            {
                tmp += value.m_Data[pos];
            }
            tmp += overflow;
            m_Data[pos] = (uint32_t)tmp;
            overflow = tmp >> 32;
        }
        if (overflow != 0)
        {
            Add((uint32_t)overflow);
        }
    }
    return 0;
}

void CGXBigInteger::Sub(CGXBigInteger& value)
{
    int c = Compare(value);
    if (c == 0)
    {
        Clear();
    }
    else if (value.IsNegative() || c == -1)
    {
        if (!value.IsNegative() && !IsNegative())
        {
            //If biger value is decreased from smaller value.
            CGXBigInteger tmp(value);
            tmp.Sub(*this);
            Clear();
            AddRange(tmp.m_Data, tmp.m_Count);
            m_Count = tmp.m_Count;
            SetIsNegative(true);
        }
        else
        {
            //If IsNegative value is decreased from the value.
            bool ret = value.IsNegative();
            value.SetIsNegative(false);
            Add(value);
            value.SetIsNegative(ret);
            SetIsNegative(!ret);
        }
    }
    else
    {
        if (!value.IsZero())
        {
            if (IsZero())
            {
                SetIsNegative(true);
                Clear();
                AddRange(value.m_Data, value.m_Count);
                m_Count = value.m_Count;
            }
            else
            {
                while (m_Count < value.m_Count)
                {
                    Add((uint32_t)0);
                }
                unsigned char borrow = 0;
                uint64_t tmp;
                uint32_t pos;
                for (pos = 0; pos != value.m_Count; ++pos)
                {
                    tmp = m_Data[pos];
                    tmp += 0x100000000;
                    tmp -= value.m_Data[pos];
                    tmp -= borrow;
                    m_Data[pos] = (uint32_t)tmp;
                    borrow = (unsigned char)((tmp < 0x100000000) ? 1 : 0);
                }
                if (borrow != 0)
                {
                    for (; pos != m_Count; ++pos)
                    {
                        tmp = m_Data[pos];
                        tmp += 0x100000000;
                        tmp -= borrow;
                        m_Data[pos] = (uint32_t)tmp;
                        borrow = (unsigned char)((tmp < 0x100000000) ? 1 : 0);
                        if (borrow == 0)
                        {
                            break;
                        }
                    }
                }
                //Remove empty last item(s).
                while (m_Count != 1 && m_Data[m_Count - 1] == 0)
                {
                    --m_Count;
                }
            }
        }
    }
}

void CGXBigInteger::Multiply(int value)
{
    if (value == 0 || IsZero())
    {
        m_Count = 0;
    }
    else if (value != 1)
    {
        uint16_t lenght = 1 + 1 + m_Count;
        Capacity(lenght);
        uint32_t* ret = new uint32_t[lenght];
        memset(ret, 0, lenght * sizeof(uint32_t));
        uint32_t overflow = 0;
        uint32_t index = 0;
        for (uint32_t j = 0; j != m_Count; ++j)
        {
            uint64_t result = value;
            result *= m_Data[j];
            result += overflow;
            overflow = (uint32_t)(result >> 32);
            index = j;
            AddValue(ret, lenght, (uint32_t)result, index);
        }
        if (overflow > 0)
        {
            AddValue(ret, lenght, overflow, 1 + index);
        }
        index = lenght - 1;
        while (index != 0 && ret[index] == 0)
        {
            --index;
        }
        ++index;
        memcpy(m_Data, ret, index * sizeof(uint32_t));
        delete[] ret;
        m_Count = index;
    }
    if (value < 0 && !IsNegative())
    {
        SetIsNegative(true);
    }
    else if (value < 0 && IsNegative())
    {
        //If both values are negative.
        SetIsNegative(false);
    }
}

void CGXBigInteger::Multiply(CGXBigInteger& value)
{
    if (value.IsZero() || IsZero())
    {
        m_Count = 0;
    }
    else if (!value.IsOne())
    {
        uint16_t lenght = 1 + value.m_Count + m_Count;
        Capacity(lenght);
        uint32_t* ret = new uint32_t[lenght];
        memset(ret, 0, lenght * sizeof(uint32_t));
        uint32_t overflow = 0;
        uint32_t index = 0;
        for (uint32_t i = 0; i != value.m_Count; ++i)
        {
            overflow = 0;
            for (uint32_t j = 0; j != m_Count; ++j)
            {
                uint64_t result = value.m_Data[i];
                result *= m_Data[j];
                result += overflow;
                overflow = (uint32_t)(result >> 32);
                index = i + j;
                AddValue(ret, lenght, (uint32_t)result, index);
            }
            if (overflow > 0)
            {
                AddValue(ret, lenght, overflow, 1 + index);
            }
        }
        index = lenght - 1;
        while (index != 0 && ret[index] == 0)
        {
            --index;
        }
        ++index;
        memcpy(m_Data, ret, index * sizeof(uint32_t));
        delete[] ret;
        m_Count = index;
    }
    if (value.IsNegative() != IsNegative())
    {
        if (!IsNegative())
        {
            SetIsNegative(true);
        }
    }
    else if (IsNegative())
    {
        //If both values are IsNegative.
        SetIsNegative(false);
    }
}

int CGXBigInteger::Compare(CGXBigInteger& value)
{
    int ret = 0;
    if (IsNegative() != value.IsNegative())
    {
        //If other value is IsNegative.
        if (IsNegative())
        {
            ret = -1;
        }
        else
        {
            ret = 1;
        }
    }
    else if (IsZero() && value.IsZero())
    {
        ret = 0;
    }
    else
    {
        int cntA = (int)m_Count;
        cntA -= 1;
        //Skip zero values.
        while (cntA != -1 && m_Data[cntA] == 0)
        {
            --cntA;
        }
        int cntB = (int)value.m_Count;
        cntB -= 1;
        //Skip zero values.
        while (cntB != -1 && value.m_Data[cntB] == 0)
        {
            --cntB;
        }
        if (cntA > cntB)
        {
            ret = 1;
        }
        else if (cntA < cntB)
        {
            ret = -1;
        }
        else
        {
            do
            {
                if (m_Data[cntA] > value.m_Data[cntA])
                {
                    ret = 1;
                    break;
                }
                else if (m_Data[cntA] < value.m_Data[cntA])
                {
                    ret = -1;
                    break;
                }
                cntA -= 1;
            } while (cntA != -1);
        }
    }
    return ret;
}

int CGXBigInteger::Compare(uint32_t value)
{
    if (m_Count == 0)
    {
        return -1;
    }
    if (m_Data[0] == value)
    {
        return 0;
    }
    return m_Data[0] < value ? -1 : 1;
}

void CGXBigInteger::Lshift(uint32_t amount)
{
    while (amount > 8 * sizeof(uint32_t))
    {
        Lshift(8 * sizeof(uint32_t));
        amount -= 8 * sizeof(uint32_t);
    }
    if (amount != 0)
    {
        uint32_t overflow = 0;
        for (uint32_t pos = 0; pos != m_Count; ++pos)
        {
            uint64_t tmp = m_Data[pos];
            tmp <<= amount;
            tmp |= overflow;
            m_Data[pos] = (uint32_t)tmp;
            overflow = (uint32_t)(tmp >> 32);
        }
        if (overflow != 0)
        {
            Add(overflow);
        }
    }
}

void CGXBigInteger::Rshift(uint32_t amount)
{
    int pos;
    while (amount > 8 * sizeof(uint32_t))
    {
        Rshift(8 * sizeof(uint32_t));
        //--m_Count;
        amount -= 8 * sizeof(uint32_t);
    }
    if (amount != 0)
    {
        uint64_t overflow = 0;
        uint32_t mask = (uint32_t)0xFFFFFFFF;
        mask = mask >> (32 - amount);
        uint32_t cnt = m_Count - 1;
        for (pos = cnt; pos != -1; --pos)
        {
            uint64_t tmp = m_Data[pos];
            m_Data[pos] = (uint32_t)((tmp >> amount) | overflow);
            overflow = (tmp & mask) << (32 - amount);
        }
        //Remove last item if it's empty.
        while (m_Count != 1 && m_Data[cnt] == 0)
        {
            --m_Count;
            --cnt;
        }
    }
}

void CGXBigInteger::Clear()
{
    m_Count = 0;
    SetIsNegative(false);
}

void CGXBigInteger::Pow(uint32_t exponent)
{
    if (exponent != 1)
    {
        uint32_t pos = 1;
        CGXBigInteger tmp(this);
        while (pos <= exponent / 2)
        {
            Multiply(*this);
            pos <<= 1;
        }
        while (pos < exponent)
        {
            Multiply(tmp);
            ++pos;
        }
    }
}

void CGXBigInteger::Div(CGXBigInteger* value)
{
    CGXBigInteger current(1);
    CGXBigInteger denom(value);
    CGXBigInteger tmp(this);
    bool neq = IsNegative();
    SetIsNegative(false);
    // while denom < this.
    while (denom.Compare(*this) == -1)
    {
        current.Lshift(1);
        denom.Lshift(1);
    }
    //If overflow.
    if (denom.Compare(*this) == 1)
    {
        if (current.IsOne())
        {
            Clear();
            return;
        }
        Clear();
        current.Rshift(1);
        denom.Rshift(1);
        while (!current.IsZero())
        {
            uint32_t r = tmp.Compare(denom);
            if (r == 1)
            {
                tmp.Sub(denom);
                Add(current);
            }
            else if (r == 0)
            {
                Add(current);
                break;
            }
            current.Rshift(1);
            denom.Rshift(1);
        }
        memcpy(current.m_Data, m_Data, sizeof(uint32_t) * m_Count);
    }
    SetIsNegative(neq);
    memcpy(m_Data, current.m_Data, sizeof(uint32_t) * m_Count);
}

void CGXBigInteger::Mod(CGXBigInteger& mod)
{
    CGXBigInteger current(1);
    CGXBigInteger denom(mod);
    bool neq = IsNegative();
    SetIsNegative(false);
    // while denom < this.
    while (denom.Compare(*this) == -1)
    {
        int dwords = GetCount() - denom.GetCount();
        if (dwords > 1)
        {
            current.Lshift(8 * dwords * sizeof(uint32_t));
            denom.Lshift(8 * dwords * sizeof(uint32_t));
        }
        else
        {
            current.Lshift(1);
            denom.Lshift(1);
        }
    }
    //If overflow.
    if (denom.Compare(*this) == 1)
    {
        if (current.IsOne())
        {
            if (neq)
            {
                Sub(mod);
                SetIsNegative(false);
            }
            return;
        }
        current.Rshift(1);
        denom.Rshift(1);
        while (!current.IsZero())
        {
            uint32_t r = Compare(denom);
            if (r == 1)
            {
                Sub(denom);
            }
            else if (r == 0)
            {
                break;
            }
            int dwords = GetCount() - denom.GetCount();
            if (dwords > 1)
            {
                current.Rshift(8 * dwords * sizeof(uint32_t));
                denom.Rshift(8 * dwords * sizeof(uint32_t));
            }
            else
            {
                current.Rshift(1);
                denom.Rshift(1);
            }
        }
    }
    else
    {
        Clear();
    }
    if (neq)
    {
        Sub(mod);
        SetIsNegative(false);
    }
}

void CGXBigInteger::Inv(CGXBigInteger& value)
{
    if (!IsZero())
    {
        CGXBigInteger lm(1);
        CGXBigInteger hm(0);
        CGXBigInteger low(this);
        low.Mod(value);
        CGXBigInteger high = value;
        while (!(low.IsZero() || low.IsOne()))
        {
            CGXBigInteger r(high);
            r.Div(&low);
            CGXBigInteger tmp(lm);
            tmp.Multiply(r);
            CGXBigInteger nm(hm);
            nm.Sub(tmp);
            tmp = low;
            tmp.Multiply(r);
            high.Sub(tmp);
            tmp = low;
            low = high;
            high = tmp;
            hm = lm;
            lm = nm;
        }
        memcpy(m_Data, lm.m_Data, sizeof(uint32_t) * lm.m_Count);
        m_Count = lm.m_Count;
        SetIsNegative(lm.IsNegative());
        Mod(value);
    }
}

std::string CGXBigInteger::ToString()
{
    std::string str;
    if (IsZero())
    {
        str = "0x00";
    }
    else
    {
        int pos;
        uint32_t cnt = 0;
        CGXByteBuffer bb;
        for (pos = (int)(m_Count - 1); pos != -1; --pos)
        {
            bb.SetUInt32(m_Data[pos]);
        }
        for (pos = 0; pos != (int)bb.GetSize(); ++pos)
        {
            if (bb.GetData()[pos] != 0)
            {
                cnt = bb.GetSize() - pos;
                break;
            }
        }
        if (IsNegative())
        {
            str = "-";
        }
        str += "0x";
        str += bb.ToHexString(pos, cnt, false);
    }
    return str;
}

uint16_t CGXBigInteger::GetUsedBits()
{
    uint16_t count = (uint16_t)(32 * m_Count);
    if (m_Count != 0)
    {
        uint32_t tmp = m_Data[m_Count - 1];
        uint32_t mask = 0x80000000;
        while ((tmp & mask) == 0)
        {
            mask >>= 1;
            --count;
        }
    }
    return count;
}

bool CGXBigInteger::IsBitSet(uint16_t index)
{
    uint16_t pos = (uint16_t)(index / 32);
    if (index > 32)
    {
        index -= (uint16_t)(32 * index);
    }
    return (m_Data[pos] & (1 << index)) != 0;
}