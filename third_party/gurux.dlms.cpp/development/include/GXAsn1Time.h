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

#ifndef GXAsn1Time_H
#define GXAsn1Time_H

#include "GXAsn1Base.h"
#include "GXDateTime.h"

/// <summary>
/// ASN1 time value base class.
/// </summary>
class CGXAsn1Time : public CGXAsn1Base
{
private:
    CGXDateTime m_Time;
public:
    /////////////////////////////////////////////////////////////////////////////
    // Constructor.
    /////////////////////////////////////////////////////////////////////////////
    CGXAsn1Time()
    {

    }

    /////////////////////////////////////////////////////////////////////////////
    // Constructor.
    // value: date time.
    /////////////////////////////////////////////////////////////////////////////
    CGXAsn1Time(CGXDateTime& value)
    {
        m_Time = value;
    }

    CGXDateTime& GetValue()
    {
        return m_Time;
    }

    void SetValue(CGXDateTime& value)
    {
        m_Time = value;
    }

    std::string ToString()
    {
        return m_Time.ToString();
    }
};

#endif //GXAsn1Time_H
