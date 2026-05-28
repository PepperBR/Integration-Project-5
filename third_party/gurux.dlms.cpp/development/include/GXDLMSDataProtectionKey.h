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

#ifndef GXDLMSDATAPROTECTIONKEY_H
#define GXDLMSDATAPROTECTIONKEY_H

#include "GXIgnore.h"
#ifndef DLMS_IGNORE_PUSH_SETUP
#include "GXDLMSDataProtectionIdentifiedKey.h"
#include "GXDLMSDataProtectionWrappeddKey.h"
#include "GXDLMSDataProtectionAgreedKey.h"

// Enumerates data protection key types.
typedef enum
{
    /**
     Identified key.
    */
    DLMS_DATA_PROTECTION_KEY_TYPE_IDENTIFIED,
    /**
     Wrapped key.
    */
    DLMS_DATA_PROTECTION_KEY_TYPE_WRAPPED,
    /**
     Agreed.
    */
    DLMS_DATA_PROTECTION_KEY_TYPE_AGREED
}DLMS_DATA_PROTECTION_KEY_TYPE;

/**
    Data protection Key.
*/
class CGXDLMSDataProtectionKey
{
private:
    /**
      * Data protectionKey type.
      */
    DLMS_DATA_PROTECTION_KEY_TYPE m_DataProtectionKeyType;

    /**
     * Identified key parameters.
     */
    CGXDLMSDataProtectionIdentifiedKey m_IdentifiedKey;

    /**
     * Wrapped key parameters.
     */
    CGXDLMSDataProtectionWrappeddKey m_WrappedKey;
    /**
     * Agreed key parameters.
     */
    CGXDLMSDataProtectionAgreedKey m_AgreedKey;

public:
    /**
     * Constructor.
     */
    CGXDLMSDataProtectionKey()
    {
        m_DataProtectionKeyType = DLMS_DATA_PROTECTION_KEY_TYPE_IDENTIFIED;
    }
    /**
    * Returns Data protectionKey type.
    */
    DLMS_DATA_PROTECTION_KEY_TYPE GetDataProtectionKeyType() {
        return m_DataProtectionKeyType;
    }

    /**
    * value : Data protectionKey type.
    */
    void SetDataProtectionKeyType(DLMS_DATA_PROTECTION_KEY_TYPE value)
    {
        m_DataProtectionKeyType = value;
    }

    /**
     * Returns Identified key parameters.
     */
    CGXDLMSDataProtectionIdentifiedKey& GetIdentifiedKey()
    {
        return m_IdentifiedKey;
    }

    /**
      * value : Wrapped key parameters.
      */
    CGXDLMSDataProtectionWrappeddKey& GetWrappedKey()
    {
        return m_WrappedKey;
    }

    /**
      * value : Agreed key parameters.
      */
    CGXDLMSDataProtectionAgreedKey& GetAgreedKey()
    {
        return m_AgreedKey;
    }

    std::string ToString()
    {
        std::string str;
        str = std::to_string(m_DataProtectionKeyType);
        str += ", ";
        str += m_IdentifiedKey.ToString();
        str += ", ";
        str += m_WrappedKey.ToString();
        str += ", ";
        str += m_AgreedKey.ToString();
        return str;
    }
};
#endif //DLMS_IGNORE_PUSH_SETUP
#endif //GXDLMSDATAPROTECTIONKEY_H