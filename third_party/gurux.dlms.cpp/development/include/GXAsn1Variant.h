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

#ifndef GXASN1VARIANT_H
#define GXASN1VARIANT_H

#include "GXDLMSVariant.h"
#include "GXAsn1Base.h"

/**
 ASN1 variant.
*/
class CGXAsn1Variant : public CGXAsn1Base
{
private:
    CGXDLMSVariant m_Value;
public:
    /////////////////////////////////////////////////////////////////////////////
    // Constructor.
    /////////////////////////////////////////////////////////////////////////////
    CGXAsn1Variant()
    {

    }

    CGXDLMSVariant& GetValue()
    {
        return m_Value;
    }

    void SetValue(CGXDLMSVariant& value)
    {
        m_Value = value;
    }

    CGXAsn1Variant(float value);
    CGXAsn1Variant(double value);

    CGXAsn1Variant(unsigned long long value);
    CGXAsn1Variant(long long value);
    CGXAsn1Variant(bool value);
    CGXAsn1Variant(char value);
    CGXAsn1Variant(short value);
    CGXAsn1Variant(int value);
    CGXAsn1Variant(long value);
    CGXAsn1Variant(struct tm value);
    CGXAsn1Variant(unsigned char* value, int count);
    CGXAsn1Variant(CGXDate& value);
    CGXAsn1Variant(CGXTime& value);
    CGXAsn1Variant(CGXDateTime& value);
    CGXAsn1Variant(CGXAsn1Variant* value);
    CGXAsn1Variant(unsigned char value);
    CGXAsn1Variant(unsigned short value);
    CGXAsn1Variant(unsigned long value);
    CGXAsn1Variant(std::string value);
    CGXAsn1Variant(std::wstring value);
    CGXAsn1Variant(CGXByteBuffer& value);
    CGXAsn1Variant(CGXByteArray& value);
    CGXAsn1Variant(const char* value);
    CGXAsn1Variant(unsigned int value);
    //Destructor.
    ~CGXAsn1Variant();


    CGXAsn1Variant& operator=(const CGXAsn1Variant& value);

    CGXAsn1Variant& operator=(std::string value);
    CGXAsn1Variant& operator=(const char* value);
    CGXAsn1Variant& operator=(CGXByteBuffer& value);
    CGXAsn1Variant& operator=(CGXByteArray& value);
    CGXAsn1Variant& operator=(float value);
    CGXAsn1Variant& operator=(double value);
    CGXAsn1Variant& operator=(unsigned long long value);
    CGXAsn1Variant& operator=(long long value);
    CGXAsn1Variant& operator=(const bool value);
    CGXAsn1Variant& operator=(char value);
    CGXAsn1Variant& operator=(short value);
    CGXAsn1Variant& operator=(int value);
    CGXAsn1Variant& operator=(long value);
    CGXAsn1Variant& operator=(unsigned char value);
    CGXAsn1Variant& operator=(unsigned short value);
    CGXAsn1Variant& operator=(unsigned int value);
    CGXAsn1Variant& operator=(unsigned long value);
    CGXAsn1Variant& operator=(struct tm value);
    CGXAsn1Variant& operator=(CGXDate& value);
    CGXAsn1Variant& operator=(CGXTime& value);
    CGXAsn1Variant& operator=(CGXDateTime& value);
    bool Equals(CGXAsn1Variant& item);
    std::string ToString();
};

#endif //GXASN1VARIANT_H
