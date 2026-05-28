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

#ifndef GXASN1IA5STRING_H
#define GXASN1IA5STRING_H

#include <string>
#include "GXAsn1Base.h"

class CGXAsn1Ia5String : public CGXAsn1Base
{
private:
    std::string m_Ia5String;
public:
    /////////////////////////////////////////////////////////////////////////////
    // Constructor.
    /////////////////////////////////////////////////////////////////////////////
    CGXAsn1Ia5String()
    {

    }

    /////////////////////////////////////////////////////////////////////////////
    // Constructor.
    // value: IA5 string.
    /////////////////////////////////////////////////////////////////////////////
    CGXAsn1Ia5String(std::string& value)
    {
        m_Ia5String = value;
    }

    std::string& GetValue()
    {
        return m_Ia5String;
    }

    void SetValue(std::string& value)
    {
        m_Ia5String = value;
    }

    std::string ToString()
    {
        return m_Ia5String;
    }
};

#endif //GXASN1IA5STRING_H
