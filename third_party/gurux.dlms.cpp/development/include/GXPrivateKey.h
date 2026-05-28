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

#ifndef GXPRIVATEKEY_H
#define GXPRIVATEKEY_H

#include "GXHelpers.h"
#include "GXAsn1Converter.h"
#include "GXPublicKey.h"

class CGXPrivateKey
{
private:
    friend class CGXEcdsa;
    friend class CGXPkcs8;
    friend class CGXPkcs10;    
    /**
     Used scheme.
    */
    ECC m_Scheme;
    /**
     Private key raw value.
    */
    CGXByteArray m_RawValue;
    CGXPublicKey m_PublicKey;
    /**
     SystemTitle is an extra information that can be used in debugging.
     SystemTitle is not serialized.
    */
    CGXByteArray m_SystemTitle;
private:
    static int UpdateSchema(
        CGXAsn1Sequence* seq,
        CGXPrivateKey& key);

    static int UpdatePublicKey(
        CGXAsn1Sequence* seq,
        CGXPrivateKey& key);

public:
    // Constructor.
    CGXPrivateKey();

    CGXPrivateKey& operator=(const CGXPrivateKey& value);

    /**
     Used scheme.
    */
    ECC GetScheme();

    /**
     Private key raw value.
    */
    CGXByteArray& GetRawValue();

    CGXByteArray& GetSystemTitle();

    void SetSystemTitle(CGXByteBuffer& value);

    /**
     Create the private key from raw bytes.
     value: Raw data
     key: Private key.
    */
    static int FromRawBytes(CGXByteBuffer& value,
        CGXPrivateKey& key);

    /**
     Create the private key from DER.
     der: DER Base64 coded string.
    */
    static int FromDer(std::string der,
        CGXPrivateKey& key);

    /**
     Create the private key from PEM.

     pem PEM in Base64 coded string.
     ReturnsPrivate key.
    */
    static int FromPem(std::string pem,
        CGXPrivateKey& value);

#if defined(_WIN32) || defined(_WIN64) || defined(__linux__)
    /**
     Create the private key from PEM file.

     path: Path to the PEM file.
     value: Private key.
     Returns error code.
    */
    static int Load(std::string& path,
        CGXPrivateKey& value);

    /**
     Save private key to PEM file.
     path File path.
     Returns error code.
    */
    int Save(std::string& path);
#endif //defined(_WIN32) || defined(_WIN64) || defined(__linux__)

    int ToDer(std::string& value);

    int ToPem(std::string& value);

    /**
     Get public key from private key.

     Returns Public key.
    */
    int GetPublicKey(CGXPublicKey& value);

    /**
     Returns the private key as a hex string.

     Returns the private key as a hex string.
    */
    std::string ToHex();

    /**
     Returns the private key as a hex string.

     addSpace Is space added between the bytes.
     Returns the private key as a hex string.
    */
    std::string ToHex(bool addSpace);

    std::string ToString();
};

#endif //GXPRIVATEKEY_H
