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

#ifndef GXASNSET_H
#define GXASNSET_H

#include <string>
#include "GXAsn1Base.h"

class CGXAsn1Set : public CGXAsn1Base
{
private:
    CGXAsn1Base* m_Key;
    CGXAsn1Base* m_Value;
public:
    /////////////////////////////////////////////////////////////////////////////
    // Constructor.
    /////////////////////////////////////////////////////////////////////////////
    CGXAsn1Set()
    {
        m_Value = m_Key = NULL;
    }

    /////////////////////////////////////////////////////////////////////////////
    // Constructor.
    /////////////////////////////////////////////////////////////////////////////
    CGXAsn1Set(CGXAsn1Base* key, CGXAsn1Base* value)
    {
        m_Key = key;
        m_Value = value;
    }


    /////////////////////////////////////////////////////////////////////////////
    // Destructor.
    /////////////////////////////////////////////////////////////////////////////
    ~CGXAsn1Set()
    {
        if (m_Value != NULL)
        {
            delete m_Value;
            m_Value = NULL;
        }
        if (m_Key != NULL)
        {
            delete m_Key;
            m_Key = NULL;
        }
    }

    CGXAsn1Base* GetKey()
    {
        return m_Key;
    }

    CGXAsn1Base* GetValue()
    {
        return m_Value;
    }

    std::string ToString()
    {
        std::string value;
        if (m_Key != NULL)
        {
            value += m_Key->ToString();
        }
        if (m_Value != NULL)
        {
            value += m_Value->ToString();
        }
        return value;
    }
};

#endif //GXASNSET_H
