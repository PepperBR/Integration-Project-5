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

#ifndef GXPUSHPROTECTIONPARAMETERS_H
#define GXPUSHPROTECTIONPARAMETERS_H

#include "GXIgnore.h"
#ifndef DLMS_IGNORE_PUSH_SETUP
#include "GXDLMSDataProtectionKey.h"

/**
 * Enumerated data protection types.
 */
typedef enum {
    /**
     * Authentication.
     */
    DLMS_PROTECTION_TYPE_AUTHENTICATION,
    /**
     * Encryption
     */
    DLMS_PROTECTION_TYPE_ENCRYPTION,
    /**
     * Authentication and encryption.
     */
    DLMS_PROTECTION_TYPE_AUTHENTICATION_ENCRYPTION,
    /**
     * Digital signature
     */
    DLMS_PROTECTION_TYPE_DIGITAL_SIGNATURE
}DLMS_PROTECTION_TYPE;

/**
Push protection parameters.
*/
class CGXPushProtectionParameters
{
private:
    /**
    * Protection type.
    */
    DLMS_PROTECTION_TYPE m_ProtectionType;

    /**
     * Transaction Id.
     */
    CGXByteBuffer m_TransactionId;

    /**
     * Originator system title.
     */
    CGXByteBuffer m_OriginatorSystemTitle;

    /**
     * Recipient system title.
     */
    CGXByteBuffer m_RecipientSystemTitle;

    /**
     * Other information.
     */
    CGXByteBuffer m_OtherInformation;

    /**
     * Key info.
     */
    CGXDLMSDataProtectionKey m_KeyInfo;

public:
    /**
     * Constructor.
     */
    CGXPushProtectionParameters()
    {
        m_ProtectionType = DLMS_PROTECTION_TYPE_AUTHENTICATION;
    }
    /**
     * Returns Protection type.
     */
    DLMS_PROTECTION_TYPE GetProtectionType()
    {
        return m_ProtectionType;
    }

    /**
     * value: Protection type.
     */
    void SetProtectionType(DLMS_PROTECTION_TYPE value)
    {
        m_ProtectionType = value;
    }

    /**
     * Returns Transaction Id.
     */
    CGXByteBuffer& GetTransactionId()
    {
        return m_TransactionId;
    }

    /**
     * value: Transaction Id.
     */
    void SetTransactionId(CGXByteBuffer& value)
    {
        m_TransactionId = value;
    }

    /**
     * Returns Originator system title.
     */
    CGXByteBuffer& GetOriginatorSystemTitle()
    {
        return m_OriginatorSystemTitle;
    }

    /**
     * value: Originator system title.
     */
    void SetOriginatorSystemTitle(CGXByteBuffer& value)
    {
        m_OriginatorSystemTitle = value;
    }

    /**
     * Returns Recipient system title.
     */
    CGXByteBuffer& GetRecipientSystemTitle()
    {
        return m_RecipientSystemTitle;
    }

    /**
     * value: Recipient system title.
     */
    void SetRecipientSystemTitle(CGXByteBuffer& value)
    {
        m_RecipientSystemTitle = value;
    }

    /**
     * Returns Other information.
     */
    CGXByteBuffer& GetOtherInformation()
    {
        return m_OtherInformation;
    }

    /**
     * value: Other information.
     */
    void SetOtherInformation(CGXByteBuffer& value)
    {
        m_OtherInformation = value;
    }

    /**
     * Returns Key info.
     */
    CGXDLMSDataProtectionKey& GetKeyInfo()
    {
        return m_KeyInfo;
    }
};
#endif //DLMS_IGNORE_PUSH_SETUP
#endif //GXPUSHPROTECTIONPARAMETERS_H