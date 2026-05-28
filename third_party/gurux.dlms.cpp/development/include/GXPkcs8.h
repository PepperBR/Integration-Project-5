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

#ifndef CGXPKCS8_H
#define CGXPKCS8_H

#include <vector>
#include "GXDLMSVariant.h"
#include "GXAsn1Sequence.h"
#include "GXPublicKey.h"
#include "GXPrivateKey.h"
#include "GXAsn1Converter.h"
#include "GXAsn1Context.h"
#include "GXAsn1BitString.h"

/**
 Pkcs8 certification request.
 Private key is saved using this format.
 https://tools.ietf.org/html/rfc5208
*/

#include "GXHelpers.h"

class CGXPkcs8
{
private:
    /**
    * Loaded PKCS #8 certificate request as a raw data.
    */
    CGXByteBuffer m_RawData;

    /**
    * Algorithm.
    */
    DLMS_X9_OBJECT_IDENTIFIER m_Algorithm;

    /**
    * Private key version.
    */
    DLMS_CERTIFICATE_VERSION m_Version;
    /**
    * Private key.
    */
    CGXPrivateKey m_PrivateKey;

    /**
    Public key.
    */
    CGXPublicKey m_PublicKey;

    /**
    * Description is extra metadata that is saved to PEM file.
    */
    std::string m_Description;

private:
    int Init(CGXByteBuffer& data);
public:
    /////////////////////////////////////////////////////////////////////////////
    // Constructor.
    /////////////////////////////////////////////////////////////////////////////
    CGXPkcs8();

    /**
     Returns default file path.
     scheme: Used scheme.
     certificateType: Certificate type.
     systemTitle: System title.
     Returns File path.
    */
    static int GetFilePath(ECC scheme,
        DLMS_CERTIFICATE_TYPE certificateType,
        CGXByteBuffer& systemTitle, 
        std::string& path);

    /**
     Description is extra metadata that is saved to PEM file.
    */
    std::string GetDescription();

    /**
     Description is extra metadata that is saved to PEM file.
    */
    void SetDescription(std::string& value);

    /**
     Private key version.
    */
    DLMS_CERTIFICATE_VERSION GetVersion();

    /**
     Private key version.
    */
    void SetVersion(DLMS_CERTIFICATE_VERSION value);

    /**
     Private key.
    */
    CGXPrivateKey& GetPrivateKey();

    /**
     Private key.
    */
    void SetPrivateKey(CGXPrivateKey& value);

    /**
     Returns default file path.

     scheme Used scheme.
     certificateType Certificate type.
     Returns
    */
    int GetFilePath(ECC scheme,
        DLMS_CERTIFICATE_TYPE certificateType,
        std::string& path,
        CGXByteBuffer& systemTitle);  

    int GetEncoded(CGXByteBuffer& encoded);

    /**
     Constructor.

     key Private key.
    */
    CGXPkcs8(CGXPrivateKey& key);

    /**
     Constructor.
     pair Private key pair.
    */
    CGXPkcs8(std::pair<CGXPublicKey, CGXPrivateKey>& pair);

    /**
     Create PKCS #8 from PEM string.

     data PEM string.
    */
    static int FromPem(
        std::string data,
        CGXPkcs8& cert);
    /**
     Create PKCS 8 from hex string.

     data Hex string.
     Returns PKCS 8
    */
    static int FromHexString(
        std::string& data,
        CGXPkcs8& cert);

    /**
    * Create Pkcs8 from byte array.
    *
    *data: Byte array.
    */
    static int FromByteArray(CGXByteBuffer& data,
        CGXPkcs8& cert);

    /**
    * Create Pkcs8 from byte array.
    *
    *data: Byte array.
    */
    static int FromByteArray(
        const unsigned char* data,
        uint16_t length,
        CGXPkcs8& cert);

    /**
     Create PKCS #8 from DER Base64 encoded string.

     der Base64 DER string.
     Returns
    */
    static int FromDer(std::string& der,
        CGXPkcs8& cert);

    std::string ToString();

#if defined(_WIN32) || defined(_WIN64) || defined(__linux__)

    /** Load private key from the PEM file.
     path: File path.
     value: Created GXPkcs8 object.
     Returns error code.
     */
    static int Load(
        std::string& path, 
        CGXPkcs8& value);

    /**
     Save private key to PEM file.

     path: File path.
     Returns error code.
     */
    int Save(std::string& path);
#endif //defined(_WIN32) || defined(_WIN64) || defined(__linux__)

    /**
     Private key in PEM format.

     Returns Private key as in PEM string.
    */
    int ToPem(std::string& pem);

    /**
     Private key in DER format.

     Returns Private key as in DER string.
    */
    int ToDer(std::string& value);

    //Is the content of the objects equal.
    bool Equals(CGXPkcs8& cert);
};

#endif //CGXPKCS8_H
