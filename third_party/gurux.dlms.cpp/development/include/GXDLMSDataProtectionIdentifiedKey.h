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

#ifndef GXDLMSDATAPROTECTIONIDENTIFIEDKEY_H
#define GXDLMSDATAPROTECTIONIDENTIFIEDKEY_H

#include "GXIgnore.h"
#ifndef DLMS_IGNORE_PUSH_SETUP

/*Data protection identified key types.*/
typedef enum
{
    /**
     * Global unicast encryption key.
     */
    DLMS_DATA_PROTECTION_IDENTIFIED_KEY_TYPE_UNICAST_ENCRYPTION,
    /**
     * Global broadcast encryption key.
     */
    DLMS_DATA_PROTECTION_IDENTIFIED_KEY_TYPE_BROADCAST_ENCRYPTION
}DLMS_DATA_PROTECTION_IDENTIFIED_KEY_TYPE;

/**
    Data protection identified key.
*/
class CGXDLMSDataProtectionIdentifiedKey
{
private:
    /**
     * Data protection key type.
     */
    DLMS_DATA_PROTECTION_IDENTIFIED_KEY_TYPE m_KeyType;

public:
    /**
     * Constructor.
     */
    CGXDLMSDataProtectionIdentifiedKey()
    {
        m_KeyType = DLMS_DATA_PROTECTION_IDENTIFIED_KEY_TYPE_UNICAST_ENCRYPTION;
    }

    /**
     * @return Data protection key type.
     */
    DLMS_DATA_PROTECTION_IDENTIFIED_KEY_TYPE GetKeyType() {
        return m_KeyType;
    }

    /**
     * @param value
     *            Data protection key type.
     */
    void SetKeyType(DLMS_DATA_PROTECTION_IDENTIFIED_KEY_TYPE value) {
        m_KeyType = value;
    }

    std::string ToString()
    {
        std::string str = std::to_string(m_KeyType);
        return str;
    }
};
#endif //DLMS_IGNORE_PUSH_SETUP
#endif //GXDLMSDATAPROTECTIONIDENTIFIEDKEY_H