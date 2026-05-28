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
#include "../include/GXAsn1Variant.h"
#include "../include/GXByteArray.h"

CGXAsn1Variant::~CGXAsn1Variant()
{
}

CGXAsn1Variant::CGXAsn1Variant(CGXAsn1Variant* value)
{
    m_Value = value->m_Value;
}

CGXAsn1Variant::CGXAsn1Variant(float value)
{
    m_Value = value;
}

CGXAsn1Variant::CGXAsn1Variant(double value)
{
    m_Value = value;
}


CGXAsn1Variant::CGXAsn1Variant(unsigned long long value)
{
    m_Value = value;
}

CGXAsn1Variant::CGXAsn1Variant(long long value)
{
    m_Value = value;
}

CGXAsn1Variant::CGXAsn1Variant(bool value)
{
    m_Value = value;
}

CGXAsn1Variant::CGXAsn1Variant(char value)
{
    m_Value = value;
}

CGXAsn1Variant::CGXAsn1Variant(short value)
{
    m_Value = value;
}

CGXAsn1Variant::CGXAsn1Variant(int value)
{
    m_Value = value;
}

CGXAsn1Variant::CGXAsn1Variant(long value)
{
    m_Value = value;
}

CGXAsn1Variant::CGXAsn1Variant(struct tm value)
{
    m_Value = value;
}

CGXAsn1Variant::CGXAsn1Variant(
    CGXByteBuffer& value)
{
    m_Value = value;
}

CGXAsn1Variant::CGXAsn1Variant(
    CGXByteArray& value)
{
    m_Value = value;
}

CGXAsn1Variant::CGXAsn1Variant(
    CGXDate& value)
{
    m_Value = value;
}

CGXAsn1Variant::CGXAsn1Variant(CGXTime& value)
{
    m_Value = value;
}

CGXAsn1Variant::CGXAsn1Variant(CGXDateTime& value)
{
    m_Value = value;
}

CGXAsn1Variant::CGXAsn1Variant(unsigned char value)
{
    m_Value = value;
}

CGXAsn1Variant::CGXAsn1Variant(unsigned short value)
{
    m_Value = value;
}

CGXAsn1Variant::CGXAsn1Variant(unsigned int value)
{
    m_Value = value;
}

CGXAsn1Variant::CGXAsn1Variant(unsigned long value)
{
    m_Value = value;
}

CGXAsn1Variant::CGXAsn1Variant(std::string value)
{
    m_Value = value;
}

CGXAsn1Variant::CGXAsn1Variant(std::wstring value)
{
    m_Value = value;
}

CGXAsn1Variant::CGXAsn1Variant(
    unsigned char* value, 
    int count)
{
    m_Value.Add(value, count);
}

CGXAsn1Variant& CGXAsn1Variant::operator=(
    const CGXAsn1Variant& value)
{
    m_Value.Clear();
    m_Value = value.m_Value;
    return *this;
}

CGXAsn1Variant& CGXAsn1Variant::operator=(
    CGXByteBuffer& value)
{
    m_Value.Clear();
    m_Value = value;
    return *this;
}

CGXAsn1Variant::CGXAsn1Variant(
    const char* value)
{
    m_Value = value;
}

CGXAsn1Variant& CGXAsn1Variant::operator=(
    std::string value)
{
    m_Value.Clear();
    m_Value = value;
    return *this;
}

CGXAsn1Variant& CGXAsn1Variant::operator=(
    const char* value)
{
    m_Value.Clear();
    m_Value = value;
    return *this;
}

CGXAsn1Variant& CGXAsn1Variant::operator=(
    float value)
{
    m_Value.Clear();
    m_Value = value;
    return *this;
}

CGXAsn1Variant& CGXAsn1Variant::operator=(
    double value)
{
    m_Value.Clear();
    m_Value = value;
    return *this;
}

CGXAsn1Variant& CGXAsn1Variant::operator=(
    unsigned long long value)
{
    m_Value.Clear();
    m_Value = value;
    return *this;
}

CGXAsn1Variant& CGXAsn1Variant::operator=(
    long long value)
{
    m_Value.Clear();
    m_Value = value;
    return *this;
}

CGXAsn1Variant& CGXAsn1Variant::operator=(
    const bool value)
{
    m_Value.Clear();
    m_Value = value;
    return *this;
}

CGXAsn1Variant& CGXAsn1Variant::operator=(
    char value)
{
    m_Value.Clear();
    m_Value = value;
    return *this;
}

CGXAsn1Variant& CGXAsn1Variant::operator=(
    short value)
{
    m_Value.Clear();
    m_Value = value;
    return *this;
}

CGXAsn1Variant& CGXAsn1Variant::operator=(
    int value)
{
    m_Value.Clear();
    m_Value = value;
    return *this;
}

CGXAsn1Variant& CGXAsn1Variant::operator=(
    long value)
{
    m_Value.Clear();
    m_Value = value;
    return *this;
}

CGXAsn1Variant& CGXAsn1Variant::operator=(
    unsigned char value)
{
    m_Value.Clear();
    m_Value = value;
    return *this;
}

CGXAsn1Variant& CGXAsn1Variant::operator=(
    unsigned short value)
{
    m_Value.Clear();
    m_Value = value;
    return *this;
}

CGXAsn1Variant& CGXAsn1Variant::operator=(
    unsigned long value)
{
    m_Value.Clear();
    m_Value = value;
    return *this;
}

CGXAsn1Variant& CGXAsn1Variant::operator=(
    struct tm value)
{
    m_Value.Clear();
    m_Value = value;
    return *this;
}

CGXAsn1Variant& CGXAsn1Variant::operator=(
    CGXDate& value)
{
    m_Value.Clear();
    m_Value = value;
    return *this;
}

CGXAsn1Variant& CGXAsn1Variant::operator=(
    CGXTime& value)
{
    m_Value.Clear();
    m_Value = value;
    return *this;
}

CGXAsn1Variant& CGXAsn1Variant::operator=(
    CGXDateTime& value)
{
    m_Value.Clear();
    m_Value = value;
    return *this;
}

CGXAsn1Variant& CGXAsn1Variant::operator=(
    unsigned int value)
{
    m_Value.Clear();
    m_Value = value;
    return *this;
}

bool CGXAsn1Variant::Equals(
    CGXAsn1Variant& item)
{
    return m_Value.Equals(item.m_Value);
}

std::string CGXAsn1Variant::ToString()
{
    return m_Value.ToString();
}