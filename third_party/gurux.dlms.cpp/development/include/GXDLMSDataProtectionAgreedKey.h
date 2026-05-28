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

#ifndef GXDLMSDATAPROTECTIONAGREEDKEY_H
#define GXDLMSDATAPROTECTIONAGREEDKEY_H

#include "GXIgnore.h"
#ifndef DLMS_IGNORE_PUSH_SETUP

/**
    Data protection agreed key.
*/
class CGXDLMSDataProtectionAgreedKey
{
private:
    /**
     * Key parameters.
     */
    CGXByteBuffer m_Parameters;

    /**
     * Key ciphered data.
     */
    CGXByteBuffer m_Data;
public:
    CGXByteBuffer& GetParameters()
    {
        return m_Parameters;
    }

    void SetParameters(CGXByteBuffer& value)
    {
        m_Parameters = value;
    }

    /**
     * @return Key ciphered data.
     */
    CGXByteBuffer& GetData()
    {
        return m_Data;
    }

    /**
     * @param value
     *            Key ciphered data.
     */
    void SetData(CGXByteBuffer& value)
    {
        m_Data = value;
    }

    std::string ToString()
    {
        std::string str;
        str += m_Parameters.ToString();
        str += ", ";
        str += m_Data.ToString();
        return str;
    }
};
#endif //DLMS_IGNORE_PUSH_SETUP
#endif //GXDLMSDATAPROTECTIONAGREEDKEY_H