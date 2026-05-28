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

#ifndef GXASNHELPERS_H
#define GXASNHELPERS_H

#include "GXAsn1Base.h"
#include "GXAsn1Variant.h"
#include "GXAsn1Sequence.h"

/// <summary>
/// ASN1 helper class.
/// </summary>
class CGXAsn1Helpers
{
public:
    static CGXAsn1Base* Clone(CGXAsn1Base* value)
    {
        if (CGXAsn1Sequence* a = dynamic_cast<CGXAsn1Sequence*>(value))
        {
            CGXAsn1Sequence* tmp = new CGXAsn1Sequence();
            for (std::vector<CGXAsn1Base*>::iterator it = a->GetValues()->begin(); it != a->GetValues()->end(); ++it)
            {
                tmp->GetValues()->push_back(Clone(*it));
            }
            return tmp;
        }
        if (CGXAsn1Variant* a = dynamic_cast<CGXAsn1Variant*>(value))
        {
            CGXAsn1Variant* tmp = new CGXAsn1Variant();
            tmp->SetValue(a->GetValue());
            return tmp;
        }
        if (CGXAsn1ObjectIdentifier* a = dynamic_cast<CGXAsn1ObjectIdentifier*>(value))
        {
            return new CGXAsn1ObjectIdentifier(a->GetObjectIdentifier());
        }
        if (CGXAsn1Ia5String* a = dynamic_cast<CGXAsn1Ia5String*>(value))
        {
            return new CGXAsn1Ia5String(a->GetValue());
        }
        if (CGXAsn1Utf8String* a = dynamic_cast<CGXAsn1Utf8String*>(value))
        {
            return new CGXAsn1Utf8String(a->GetValue());
        }
        assert(0);
        return NULL;
    }
};

#endif //GXASNHELPERS_H
