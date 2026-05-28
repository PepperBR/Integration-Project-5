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

#include "../include/GXAsn1Context.h"

CGXAsn1Context::CGXAsn1Context()
{
    m_Index = 0;
    m_Constructed = true;
}

/////////////////////////////////////////////////////////////////////////////
// Destructor.
/////////////////////////////////////////////////////////////////////////////
CGXAsn1Context::~CGXAsn1Context()
{
    for (std::vector<CGXAsn1Base*>::iterator it = m_Values.begin(); it != m_Values.end(); ++it)
    {
        delete (*it);
    }
    m_Values.clear();
}
/* Context index.*/
int CGXAsn1Context::GetIndex()
{
	return m_Index;
}

/* Context index.*/
void CGXAsn1Context::SetIndex(int value)
{
	m_Index = value;
}

/**
 Constructed.
*/
bool CGXAsn1Context::GetConstructed()
{
	return m_Constructed;
}

/**
 Constructed.
*/
void CGXAsn1Context::SetConstructed(bool value)
{
	m_Constructed = value;
}