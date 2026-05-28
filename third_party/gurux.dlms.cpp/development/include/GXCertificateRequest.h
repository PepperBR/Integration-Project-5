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

#ifndef GXCERTIFICATEREQUEST_H
#define GXCERTIFICATEREQUEST_H

#include <vector>
#include "GXAsn1Base.h"
#include "GXDLMSVariant.h"

class CGXPkcs10;

/// <summary>
/// Certificate request.
/// </summary>
class CGXCertificateRequest
{
private:
    friend class CGXPkcs10;
    /**
     * Certificate type.
     */
    DLMS_CERTIFICATE_TYPE m_CertificateType;
    /**
     * Indicates the purpose for which the certified public key is used.
     */
    DLMS_EXTENDED_KEY_USAGE m_ExtendedKeyUsage;
    /**
     * Certificate Signing Request.
     */
    CGXPkcs10* m_Certificate;

public:
    /**
    * Constructor.
    */    
    CGXCertificateRequest()
    {
        m_Certificate = NULL;
        m_CertificateType = DLMS_CERTIFICATE_TYPE_DIGITAL_SIGNATURE;
        m_ExtendedKeyUsage = DLMS_EXTENDED_KEY_USAGE_NONE;
    }

    /**
     * Constructor.
     * 
     * certificateType: Certificate type.
     * certificate: PKCS 10 certificate.
     */
    CGXCertificateRequest(
        DLMS_CERTIFICATE_TYPE certificateType,
        CGXPkcs10* certificate)
    {
        m_Certificate = NULL;
        m_ExtendedKeyUsage = DLMS_EXTENDED_KEY_USAGE_NONE;
        m_Certificate = certificate;
        m_CertificateType = certificateType;
    }

    /**
     * Returns Certificate type.
     */
    DLMS_CERTIFICATE_TYPE GetCertificateType() 
    {
        return m_CertificateType;
    }

    /**
     * value: Certificate type.
     */
    void SetCertificateType(DLMS_CERTIFICATE_TYPE value)
    {
        m_CertificateType = value;
    }

    /**
     *
     * @return Indicates the purpose for which the certified public key is used.
     */
    DLMS_EXTENDED_KEY_USAGE GetExtendedKeyUsage()
    {
        return m_ExtendedKeyUsage;
    }

    /**
     * value: Indicates the purpose for which 
     *        the certified public key is used.
     */
    void SetExtendedKeyUsage(DLMS_EXTENDED_KEY_USAGE value)
    {
        m_ExtendedKeyUsage = value;
    }

    /**
     *
     * @return Certificate Signing Request.
     */
    CGXPkcs10* GetCertificate()
    {
        return m_Certificate;
    }

    /**
     * value: Certificate Signing Request.
     */
    void SetCertificate(CGXPkcs10* value)
    {
        m_Certificate = value;
    }
};

#endif //GXCERTIFICATEREQUEST_H
