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

#ifndef GXPUBLICKEY_H
#define GXPUBLICKEY_H

#include "GXHelpers.h"
#include "GXAsn1Converter.h"
#include "GXByteArray.h"

class CGXPublicKey
{
private:
    friend class CGXEcdsa;
    friend class CGXPkcs10;
    /**
    * Used scheme.
    */
    ECC m_Scheme;
    /**
     Public key raw value.
    */
    CGXByteArray m_RawValue;
    /**
     SystemTitle is an extra information that can be used in debugging.
     SystemTitle is not serialized.
    */
    CGXByteArray m_SystemTitle;
public:
    /*Constructor.*/
    CGXPublicKey();

    CGXPublicKey& operator=(const CGXPublicKey& value);

    /**
    * Used scheme.
    */
    ECC GetScheme();

    /**
     Private key raw value.
    */
    CGXByteArray& GetRawValue();

    /**
     SystemTitle is an extra information that can be used in debugging.
     SystemTitle is not serialized.
    */
    CGXByteArray& GetSystemTitle();

    /**
     SystemTitle is an extra information that can be used in debugging.
     SystemTitle is not serialized.
    */
    void SetSystemTitle(CGXByteBuffer& value);

    /**
     Create the key from raw bytes.

     key Raw data
     value: Public key.
     Returns the error code.
    */
    static int FromRawBytes(
        CGXByteBuffer& key,
        CGXPublicKey& value);

    /**
     Create the key from DER.

     der: DER Base64 coded string.
     key: Public key.
     Returns the error code.
    */
    static int FromDer(
        std::string der,
        CGXPublicKey& key);

    /**
     Create the key from PEM.

     pem: PEM Base64 coded string.
     key:Public key.
     Returns the error code.
    */
    static int FromPem(
        std::string pem,
        CGXPublicKey& key);

#if defined(_WIN32) || defined(_WIN64) || defined(__linux__)
    /**
     Create the key from PEM file.

     path: Path to the PEM file.
     value: Public key.
     Returns the error code.
     */
    static int Load(std::string& path,
        CGXPublicKey& value);

    /** 
     Save Pkcs #10 Certificate Signing Request to PEM file.

     path: File path.
     */
    int Save(std::string& path);
#endif //defined(_WIN32) || defined(_WIN64) || defined(__linux__)


    /**
     Returns the key as a hex string.

     Returns
    */
    std::string ToHex();

    /**
     Get key as DER format.
    */
    int ToDer(std::string& value);

    /**
     Get key as encoded format.
    */
    int GetEncoded(CGXByteBuffer& value);

    int ToPem(std::string& value);

    /**
     X Coordinate.
    */
    CGXByteArray GetX();

    /**
     Y Coordinate.
    */
    CGXByteArray GetY();

    std::string ToString();
};

#endif //GXPUBLICKEY_H
