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

#ifndef GXDLMSDATAPROTECTIONWRAPPEDDKEY_H
#define GXDLMSDATAPROTECTIONWRAPPEDDKEY_H

#include "GXIgnore.h"
#ifndef DLMS_IGNORE_PUSH_SETUP

/**
 * Data protection wrapped key types.
 */
typedef enum {
    /**
     * Master key.
     */
    DLMS_DATA_PROTECTION_WRAPPED_KEY_TYPE_MASTER_KEY
}DLMS_DATA_PROTECTION_WRAPPED_KEY_TYPE;

/**
    Data protection wrapped key.
*/
class CGXDLMSDataProtectionWrappeddKey
{
private:
    /**
    * Data protectionKey type.
    */
    DLMS_DATA_PROTECTION_WRAPPED_KEY_TYPE m_KeyType;

    /**
     * Key ciphered data.
     */
    CGXByteBuffer m_Key;
public:
    /**
     * Constructor.
     */
    CGXDLMSDataProtectionWrappeddKey()
    {
        m_KeyType = DLMS_DATA_PROTECTION_WRAPPED_KEY_TYPE_MASTER_KEY;
    }

    /**
     * @return Data protectionKey type.
     */
    DLMS_DATA_PROTECTION_WRAPPED_KEY_TYPE GetKeyType()
    {
        return m_KeyType;
    }

    /**
     * @param value
     *            Data protectionKey type.
     */
    void SetKeyType(DLMS_DATA_PROTECTION_WRAPPED_KEY_TYPE value)
    {
        m_KeyType = value;
    }

    /**
     * @return Key ciphered data.
     */
    CGXByteBuffer& GetKey()
    {
        return m_Key;
    }

    /**
     * @param value
     *            Key ciphered data.
     */
    void SetKey(CGXByteBuffer& value)
    {
        m_Key = value;
    }

    std::string ToString()
    {
        std::string str;
        str = std::to_string(m_KeyType);
        str += ", ";
        str += m_Key.ToString();
        return str;
    }
};
#endif //DLMS_IGNORE_PUSH_SETUP
#endif //GXDLMSDATAPROTECTIONWRAPPEDDKEY_H