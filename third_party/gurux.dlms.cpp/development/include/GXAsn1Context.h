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

#ifndef GXASN1CONTEXT_H
#define GXASN1CONTEXT_H

#include <vector>
#include "GXAsn1Base.h"
#include "GXDLMSVariant.h"

class CGXAsn1Context : public CGXAsn1Base
{
private:
    /* Context index.*/
    int m_Index;
    /**
     Constructed.
    */
    bool m_Constructed;
    std::vector<CGXAsn1Base*> m_Values;
public:
    /////////////////////////////////////////////////////////////////////////////
    // Constructor.
    /////////////////////////////////////////////////////////////////////////////
    CGXAsn1Context();

    /////////////////////////////////////////////////////////////////////////////
    // Destructor.
    /////////////////////////////////////////////////////////////////////////////
    ~CGXAsn1Context();

    /* Context index.*/
    int GetIndex();

    /* Context index.*/
    void SetIndex(int value);

    /**Constructed.*/
    bool GetConstructed();

    /**Constructed.*/
    void SetConstructed(bool value);

    /* Returns collection values. */
    std::vector<CGXAsn1Base*>* GetValues()
    {
        return &m_Values;
    }
};

#endif //GXASN1CONTEXT_H
