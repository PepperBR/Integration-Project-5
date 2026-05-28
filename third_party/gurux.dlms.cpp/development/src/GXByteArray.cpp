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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "../include/errorcodes.h"
#include "../include/GXByteArray.h"
#include "../include/GXHelpers.h"

//Constructor.
CGXByteArray::CGXByteArray()
{
    m_Capacity = 0;
    m_Data = NULL;
    m_Size = 0;
}

//Constructor.
CGXByteArray::CGXByteArray(int capacity)
{
    m_Capacity = 0;
    m_Data = NULL;
    m_Size = 0;
    Capacity(capacity);
}

//Copy constructor.
CGXByteArray::CGXByteArray(const CGXByteArray& value)
{
    m_Capacity = 0;
    m_Data = NULL;
    m_Size = 0;
    if (value.m_Size != 0)
    {
        Set(value.m_Data, value.m_Size);
    }
}

//Destructor.
CGXByteArray::~CGXByteArray()
{
    Clear();
}

unsigned long CGXByteArray::GetSize()
{
    return m_Size;
}

void CGXByteArray::SetSize(unsigned long value)
{
    assert(!(value > m_Capacity));
    m_Size = value;
}

int CGXByteArray::IncreaseSize(unsigned long size)
{
    if (size > 1)
    {
        return -1;
    }
    m_Size += size;
    return m_Size;
}

// Allocate new size for the array in bytes.
int CGXByteArray::Capacity(unsigned long capacity)
{
    m_Capacity = capacity;
    if (capacity == 0)
    {
        if (m_Data != NULL)
        {
            free(m_Data);
            m_Data = NULL;
        }
        m_Size = 0;
    }
    else
    {
        unsigned char* tmp;
        if (m_Data == NULL)
        {
            tmp = (unsigned char*)malloc(m_Capacity);
        }
        else
        {
            tmp = (unsigned char*)realloc(m_Data, m_Capacity);
        }
        //If not enought memory available.
        if (tmp == NULL)
        {
            return DLMS_ERROR_CODE_OUTOFMEMORY;
        }
        m_Data = tmp;
        if (m_Size > capacity)
        {
            m_Size = capacity;
        }
    }
    return 0;
}

unsigned long CGXByteArray::Capacity()
{
    return m_Capacity;
}

// Fill buffer it with zeros.
void CGXByteArray::Zero(unsigned long index, unsigned long count)
{
    if (index + count > m_Capacity)
    {
        CGXByteArray::Capacity(index + count);
    }
    if (m_Size < index + count)
    {
        m_Size = index + count;
    }
    memset(m_Data + index, 0, count);
}

// Push new data to the byteArray.
int CGXByteArray::SetUInt8(unsigned char item)
{
    int ret = SetUInt8(m_Size, item);
    if (ret == 0)
    {
        ++m_Size;
    }
    return ret;
}

int CGXByteArray::SetUInt8(unsigned long index, unsigned char item)
{
    if (m_Capacity == 0 || index + 1 > m_Capacity)
    {
        m_Capacity += VECTOR_CAPACITY;
        unsigned char* tmp = (unsigned char*)realloc(m_Data, m_Capacity);
        //If not enought memory available.
        if (tmp == NULL)
        {
            return DLMS_ERROR_CODE_OUTOFMEMORY;
        }
        m_Data = tmp;
    }
    m_Data[index] = item;
    return 0;
}

int CGXByteArray::SetUInt16(unsigned short item)
{
    int ret = SetUInt16(m_Size, item);
    if (ret == 0)
    {
        m_Size += 2;
    }
    return ret;
}

int CGXByteArray::SetUInt16(unsigned long index, unsigned short item)
{
    if (m_Capacity == 0 || index + 2 > m_Capacity)
    {
        m_Capacity += VECTOR_CAPACITY;
        unsigned char* tmp = (unsigned char*)realloc(m_Data, m_Capacity);
        //If not enought memory available.
        if (tmp == NULL)
        {
            return DLMS_ERROR_CODE_OUTOFMEMORY;
        }
        m_Data = tmp;
    }
    m_Data[index] = (item >> 8) & 0xFF;
    m_Data[index + 1] = item & 0xFF;
    return 0;
}

int CGXByteArray::SetUInt32(unsigned long item)
{
    int ret = CGXByteArray::SetUInt32ByIndex(m_Size, item);
    if (ret == 0)
    {
        m_Size += 4;
    }
    return ret;
}

int CGXByteArray::SetUInt32ByIndex(unsigned long index, unsigned long item)
{
    if (m_Capacity == 0 || index + 4 > m_Capacity)
    {
        m_Capacity += VECTOR_CAPACITY;
        unsigned char* tmp = (unsigned char*)realloc(m_Data, m_Capacity);
        //If not enought memory available.
        if (tmp == NULL)
        {
            return DLMS_ERROR_CODE_OUTOFMEMORY;
        }
        m_Data = tmp;
    }
    m_Data[index] = (item >> 24) & 0xFF;
    m_Data[index + 1] = (item >> 16) & 0xFF;
    m_Data[index + 2] = (item >> 8) & 0xFF;
    m_Data[index + 3] = item & 0xFF;
    return 0;
}

int CGXByteArray::SetUInt64(unsigned long long item)
{
    if (m_Capacity == 0 || m_Size + 8 > m_Capacity)
    {
        m_Capacity += VECTOR_CAPACITY;
        unsigned char* tmp = (unsigned char*)realloc(m_Data, m_Capacity);
        //If not enought memory available.
        if (tmp == NULL)
        {
            return DLMS_ERROR_CODE_OUTOFMEMORY;
        }
        m_Data = tmp;
    }
    m_Data[m_Size] = (unsigned char)((item >> 56) & 0xFF);
    m_Data[m_Size + 1] = (item >> 48) & 0xFF;
    m_Data[m_Size + 2] = (item >> 40) & 0xFF;
    m_Data[m_Size + 3] = (item >> 32) & 0xFF;
    m_Data[m_Size + 4] = (item >> 24) & 0xFF;
    m_Data[m_Size + 5] = (item >> 16) & 0xFF;
    m_Data[m_Size + 6] = (item >> 8) & 0xFF;
    m_Data[m_Size + 7] = item & 0xFF;
    m_Size += 8;
    return 0;
}

int CGXByteArray::SetFloat(float value)
{
    typedef union
    {
        float value;
        char b[sizeof(float)];
    } HELPER;

    HELPER tmp;
    tmp.value = value;
    if (m_Capacity == 0 || m_Size + 4 > m_Capacity)
    {
        m_Capacity += VECTOR_CAPACITY;
        unsigned char* tmp = (unsigned char*)realloc(m_Data, m_Capacity);
        //If not enought memory available.
        if (tmp == NULL)
        {
            return DLMS_ERROR_CODE_OUTOFMEMORY;
        }
        m_Data = tmp;
    }
    m_Data[m_Size] = tmp.b[3];
    m_Data[m_Size + 1] = tmp.b[2];
    m_Data[m_Size + 2] = tmp.b[1];
    m_Data[m_Size + 3] = tmp.b[0];
    m_Size += 4;
    return 0;
}

int CGXByteArray::SetDouble(double value)
{
    typedef union
    {
        double value;
        char b[sizeof(double)];
    } HELPER;

    HELPER tmp;
    tmp.value = value;
    if (m_Capacity == 0 || m_Size + 8 > m_Capacity)
    {
        m_Capacity += VECTOR_CAPACITY;
        unsigned char* tmp = (unsigned char*)realloc(m_Data, m_Capacity);
        //If not enought memory available.
        if (tmp == NULL)
        {
            return DLMS_ERROR_CODE_OUTOFMEMORY;
        }
        m_Data = tmp;
    }
    m_Data[m_Size] = tmp.b[7];
    m_Data[m_Size + 1] = tmp.b[6];
    m_Data[m_Size + 2] = tmp.b[5];
    m_Data[m_Size + 3] = tmp.b[4];
    m_Data[m_Size + 4] = tmp.b[3];
    m_Data[m_Size + 5] = tmp.b[2];
    m_Data[m_Size + 6] = tmp.b[1];
    m_Data[m_Size + 7] = tmp.b[0];
    m_Size += 8;
    return 0;
}

int CGXByteArray::SetInt8(char item)
{
    return CGXByteArray::SetUInt8((unsigned char)item);
}

int CGXByteArray::SetInt16(short item)
{
    return CGXByteArray::SetUInt16((unsigned short)item);
}

int CGXByteArray::SetInt32(long item)
{
    return CGXByteArray::SetUInt32((unsigned long)item);
}

int CGXByteArray::SetInt64(long long item)
{
    return CGXByteArray::SetUInt64((unsigned long long) item);
}

int CGXByteArray::Set(const void* pSource, unsigned long count)
{
    if (pSource != NULL && count != 0)
    {
        if (m_Size + count > m_Capacity)
        {
            //First time data is reserved only for the added data.
            if (m_Capacity == 0)
            {
                m_Capacity = count;
            }
            else
            {
                m_Capacity += count + VECTOR_CAPACITY;
            }
            unsigned char* tmp = (unsigned char*)realloc(m_Data, m_Capacity);
            //If not enought memory available.
            if (tmp == NULL)
            {
                return DLMS_ERROR_CODE_OUTOFMEMORY;
            }
            m_Data = tmp;
        }
        memcpy(m_Data + m_Size, pSource, count);
        m_Size += count;
    }
    return 0;
}

int CGXByteArray::Set(
    CGXByteBuffer* data, 
    unsigned long index, 
    unsigned long count)
{
    if (data != NULL)
    {
        if (count == (unsigned long)-1)
        {
            count = data->m_Size - index;
        }
        int ret = CGXByteArray::Set(data->m_Data + index, count);
        if (ret == 0)
        {
            data->m_Position += count;
        }
    }
    return 0;
}

int CGXByteArray::AddString(const char* value)
{
    if (value != NULL)
    {
        unsigned long len = (unsigned long)strlen(value);
        return CGXByteArray::Set(value, len);
    }
    return 0;
}

int CGXByteArray::AddString(const std::string& value)
{
    return CGXByteArray::Set(value.c_str(), (unsigned long)value.length());
}

int CGXByteArray::AddString(const std::wstring& value)
{
    return CGXByteArray::Set(value.c_str(), (unsigned long)value.length());
}

int CGXByteArray::AttachString(char* value)
{
    unsigned long len = (unsigned long)strlen(value);
    int ret = CGXByteArray::Set(value, len);
    if (ret == 0)
    {
        free(value);
    }
    return ret;
}

void CGXByteArray::Clear()
{
    Capacity(0);
}

int CGXByteArray::GetUInt8(unsigned long index, unsigned char* value)
{
    if (index >= m_Size)
    {
        return DLMS_ERROR_CODE_OUTOFMEMORY;
    }
    *value = m_Data[index];
    return 0;
}

int CGXByteArray::GetUInt24(unsigned long index, unsigned int* value)
{
    if (index + 3 > m_Size)
    {
        return DLMS_ERROR_CODE_OUTOFMEMORY;
    }
    *value = m_Data[index] << 16 |
        m_Data[index + 1] << 8 |
        m_Data[index + 2];
    return 0;
}

int CGXByteArray::GetUInt32(unsigned long index, unsigned long* value)
{
    if (index + 4 > m_Size)
    {
        return DLMS_ERROR_CODE_OUTOFMEMORY;
    }
    *value = m_Data[index] << 24 |
        m_Data[index + 1] << 16 |
        m_Data[index + 2] << 8 |
        m_Data[index + 3];
    return 0;
}

int CGXByteArray::GetUInt64(unsigned long index, unsigned long long* value)
{
    if (index + 8 > m_Size)
    {
        return DLMS_ERROR_CODE_OUTOFMEMORY;
    }
    *value = (unsigned long long)m_Data[index] << 56 |
        (unsigned long long) m_Data[index + 1] << 48 |
        (unsigned long long) m_Data[index + 2] << 40 |
        (unsigned long long) m_Data[index + 3] << 32 |
        (unsigned long long) m_Data[index + 4] << 24 |
        (unsigned long long) m_Data[index + 5] << 16 |
        (unsigned long long) m_Data[index + 6] << 8 |
        (unsigned long long) m_Data[index + 7];
    return 0;
}

int CGXByteArray::GetUInt128(unsigned long index, unsigned char* value)
{
    int ret = CGXByteArray::GetUInt32(index, (unsigned long*)value);
    if (ret == 0)
    {
        ret = CGXByteArray::GetUInt32(index + 4, (unsigned long*)value + 1);
        if (ret == 0)
        {
            ret = CGXByteArray::GetUInt32(index + 8, (unsigned long*)value + 2);
            if (ret == 0)
            {
                ret = CGXByteArray::GetUInt32(index + 12, (unsigned long*)value + 3);
            }
        }
    }
    return ret;
}

unsigned char* CGXByteArray::GetData()
{
    return m_Data;
}

int CGXByteArray::GetUInt16(unsigned long index, unsigned short* value)
{
    if (index + 2 > m_Size)
    {
        return DLMS_ERROR_CODE_OUTOFMEMORY;
    }
    *value = m_Data[index] << 8 |
        m_Data[index + 1];
    return 0;
}

std::string CGXByteArray::ToString()
{
    std::string str;
    if (m_Size != 0)
    {
        str.append(reinterpret_cast<char const*>(m_Data), m_Size);
    }
    return str;
}

std::string CGXByteArray::ToHexString()
{
    return GXHelpers::BytesToHex(m_Data, m_Size);
}

std::string CGXByteArray::ToHexString(bool addSpaces)
{
    return GXHelpers::BytesToHex(m_Data, m_Size, addSpaces);
}

std::string CGXByteArray::ToHexString(unsigned long position, unsigned long count, bool addSpaces)
{
    if (m_Data == NULL || count == 0)
    {
        return "";
    }
    return GXHelpers::BytesToHex(m_Data + position, count, addSpaces);
}

void CGXByteArray::AddIntAsString(int value)
{
    std::string str = GXHelpers::IntToString(value);
    CGXByteArray::AddString(str.c_str());
}

void CGXByteArray::AddDoubleAsString(double value)
{
    char buff[20];
    //Show as integer value if there is no fractal part.
    if (value - (long)value == 0)
    {
        CGXByteArray::AddIntAsString((int)value);
    }
    else
    {
#if _MSC_VER > 1000
        sprintf_s(buff, 20, "%lf", value);
#else
        sprintf(buff, "%lf", value);
#endif
        CGXByteArray::AddString(buff);
    }
}

int CGXByteArray::SubArray(
    unsigned long index,
    int count,
    CGXByteArray& bb)
{
    bb.Clear();
    bb.Set(m_Data + index, count);
    return 0;
}

int CGXByteArray::SubArray(
    unsigned long index,
    int count,
    CGXByteBuffer& bb)
{
    bb.Clear();
    bb.Set(m_Data + index, count);
    return 0;
}

bool CGXByteArray::Compare(unsigned char* buff, unsigned long length)
{
    if (buff == NULL || length == 0)
    {
        return m_Size == 0;
    }
    if (m_Size < length)
    {
        return false;
    }
    return memcmp(m_Data, buff, length) == 0;
}

void CGXByteArray::ToArray(unsigned char*& value, unsigned long& count)
{
    if (value != NULL)
    {
        free(value);
    }
    count = m_Size;
    value = (unsigned char*)malloc(count);
    memcpy(value, m_Data, count);
}

int CGXByteArray::ToByteBuffer(CGXByteBuffer& value)
{
    value.Capacity(GetSize());
    value.SetSize(0);
    if (GetSize() != 0)
    {
        return value.Set(m_Data, m_Size);
    }
    return 0;
}

CGXByteArray& CGXByteArray::operator=(
    CGXByteArray& value)
{
    Capacity(value.GetSize());
    m_Size = 0;
    if (value.GetSize() != 0)
    {
        Set(value.m_Data, value.m_Size);
    }
    return *this;
}

CGXByteArray& CGXByteArray::operator=(
    CGXByteBuffer& value)
{
    Capacity(value.GetSize());
    m_Size = 0;
    if (value.GetSize() != 0)
    {
        Set(value.m_Data, value.m_Size);
    }
    return *this;
}

void CGXByteArray::SetHexString(std::string& value)
{
    CGXByteBuffer tmp;
    GXHelpers::HexToBytes(value, tmp);
    Set(&tmp);
}

void CGXByteArray::SetHexString2(std::string value)
{
    CGXByteBuffer tmp;
    GXHelpers::HexToBytes(value, tmp);
    Set(&tmp);
}

void CGXByteArray::SetHexString(char* value)
{
    CGXByteBuffer tmp;
    std::string str = value;
    GXHelpers::HexToBytes(str, tmp);
    Set(&tmp);
}

bool CGXByteArray::IsAsciiString(unsigned char* value, unsigned long length)
{
    if (value != NULL)
    {
        for (unsigned long pos = 0; pos != length; ++pos)
        {
            unsigned char it = value[pos];
            if ((it < 32 || it > 127) && it != '\r' && it != '\n' && it != 0)
            {
                return false;
            }
        }
    }
    return true;
}

bool CGXByteArray::IsAsciiString()
{
    return IsAsciiString(m_Data, m_Size);
}

int CGXByteArray::GetString(unsigned long index,
    unsigned long count,
    std::string& value)
{
    if (index + count > m_Size)
    {
        return DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    if (count != 0)
    {
        value.append(reinterpret_cast<char const*>(m_Data + index), count);
    }
    return 0;
}

int CGXByteArray::GetStringUnicode(
    unsigned long index,
    unsigned long count,
    std::string& value)
{
    if (index + count > m_Size)
    {
        return DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    if (m_Size != 0)
    {
        value.append(reinterpret_cast<char const*>(m_Data + index), m_Size);
    }
    return 0;
}

int CGXByteArray::GetStringUnicode(
    unsigned long index,
    unsigned long count,
    std::wstring& value)
{
    if (index + count > m_Size)
    {
        return DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    if (m_Size != 0)
    {
        value.append(reinterpret_cast<wchar_t const*>(m_Data + index), m_Size);
    }
    return 0;
}
/**
* Get index of given char.
*
* @param ch
* @return
*/
static int GetIndex(char ch)
{
    if (ch == '+')
    {
        return 62;
    }
    if (ch == '/')
    {
        return 63;
    }
    if (ch == '=')
    {
        return 64;
    }
    if (ch < ':')
    {
        return (52 + (ch - '0'));
    }
    if (ch < '[')
    {
        return (ch - 'A');
    }
    if (ch < '{')
    {
        return (26 + (ch - 'a'));
    }
    return -1;
}

/**
   * Convert Base64 string to byte array.
   *
   * @param input
   *            Base64 string.
   * @return Converted byte array.
   */
int CGXByteArray::FromBase64(std::string input)
{
    GXHelpers::Replace(input, "\r\n", "");
    GXHelpers::Replace(input, "\n", "");
    if (input.length() % 4 != 0)
    {
        return DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    size_t len = (input.length() * 3) / 4;
    size_t pos = input.find('=', 0);
    if (pos > 0)
    {
        len -= input.length() - pos;
    }
    std::string inChars;
    int b[4];
    for (pos = 0; pos != input.length(); pos += 4)
    {
        inChars = input.substr(pos, pos + 4);
        b[0] = GetIndex(inChars[0]);
        b[1] = GetIndex(inChars[1]);
        b[2] = GetIndex(inChars[2]);
        b[3] = GetIndex(inChars[3]);
        SetUInt8((b[0] << 2) | (b[1] >> 4));
        if (b[2] < 64)
        {
            SetUInt8((b[1] << 4) | (b[2] >> 2));
            if (b[3] < 64)
            {
                SetUInt8((b[2] << 6) | b[3]);
            }
        }
    }
    return 0;
}

const char BASE_64_ARRAY[] = { 'A', 'B', 'C', 'D', 'E', 'F',
    'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S',
    'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's',
    't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5',
    '6', '7', '8', '9', '+', '/', '=' };

int CGXByteArray::ToBase64(std::string& value)
{
    unsigned long b, pos;
    for (pos = 0; pos < m_Size; pos += 3)
    {
        b = (m_Data[pos] & 0xFC) >> 2;
        value += BASE_64_ARRAY[b];
        b = (m_Data[pos] & 0x03) << 4;
        if (pos + 1 < m_Size)
        {
            b |= (m_Data[pos + 1] & 0xF0) >> 4;
            value += BASE_64_ARRAY[b];
            b = (m_Data[pos + 1] & 0x0F) << 2;
            if (pos + 2 < m_Size)
            {
                b |= (m_Data[pos + 2] & 0xC0) >> 6;
                value += BASE_64_ARRAY[b];
                b = m_Data[pos + 2] & 0x3F;
                value += BASE_64_ARRAY[b];
            }
            else {
                value += BASE_64_ARRAY[b];
                value += '=';
            }
        }
        else {
            value += BASE_64_ARRAY[b];
            value += "==";
        }
    }
    return 0;
}

CGXByteArray& CGXByteArray::operator=(
    const CGXByteArray& value)
{
    Clear();
    Capacity(value.m_Capacity);
    if (value.m_Size != 0)
    {
        memcpy(m_Data, value.m_Data, value.m_Size);
        m_Size = value.m_Size;
    }
    return *this;
}

CGXByteArray& CGXByteArray::operator=(
    const CGXByteBuffer& value)
{
    Clear();
    Capacity(value.m_Capacity);
    if (value.m_Size != 0)
    {
        memcpy(m_Data, value.m_Data, value.m_Size);
        m_Size = value.m_Size;
    }
    return *this;
}

void CGXByteArray::Reverse(
    unsigned long index,
    unsigned long count)
{
    unsigned long pos;
    unsigned long endPos;
    if (index == 0)
    {
        endPos = m_Size - 1;
    }
    else
    {
        endPos = index + count - 1;
    }
    unsigned char tmp;
    count /= 2;
    for (pos = 0; pos != count; ++pos)
    {
        tmp = m_Data[index + pos];
        m_Data[index + pos] = m_Data[endPos];
        m_Data[endPos] = tmp;
        --endPos;
    }
}
