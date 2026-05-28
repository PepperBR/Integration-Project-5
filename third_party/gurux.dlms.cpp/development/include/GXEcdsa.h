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

#ifndef GXECDSA_H
#define GXECDSA_H

#include "GXPublicKey.h"
#include "GXPrivateKey.h"
#include "GXCurve.h"
#include "GXCurve.h"

/*This class implements ECDSA
asynchronous ciphering.*/
class CGXEcdsa
{
    friend class CGXPrivateKey;
    /**
    * Public key.
    */
    CGXPublicKey m_PublicKey;
    /**
    * Private key.
    */
    CGXPrivateKey m_PrivateKey;

    CGXCurve m_Curve;

    /**
     Generate random number.

     N N
     Returns Random number.

    */
    static int GetRandomNumber(CGXBigInteger& N,
        CGXByteBuffer& value);

public:
    /**
     Constructor.
    */
    CGXEcdsa(ECC scheme);

    /**
    Constructor.

    key Public key.
   */
    CGXEcdsa(CGXPublicKey& key);

    /**
     Constructor.

     key Private key.
    */
    CGXEcdsa(CGXPrivateKey& key);

    /**
     Get scheme size in bytes.

     scheme
     Returns
    */
    static int GetSchemeSize(ECC scheme);

    /**
        Sign given data using and private key.

        data: Data to sign.
        signature: Signature.
        Returns error code.
    */
    int Sign(CGXByteBuffer& data,
        CGXByteBuffer& signature);
    /**
        Generate shared secret from and private key.

        publicKey: Public key.
        secret: Generated secret.
        Returns error code.
    */
    int GenerateSecret(
        CGXPublicKey& publicKey,
        CGXByteBuffer& secret);

    /**
     Generates public and private key pair.

     Returns
    */
    static int GenerateKeyPair(
        ECC scheme,
        std::pair<CGXPublicKey, CGXPrivateKey>& kp);

    /**
     Verify that signature matches the data.

     signature Generated signature.
     data Data to valuate.
     Returns
    */
    int Verify(CGXByteBuffer& signature,
        CGXByteBuffer& data,
        bool& value);

    /**
     Check that this is correct key.
     This method can be used to verify that 
     public and private key are on the curve.

    */
    static int Validate(CGXPublicKey& publicKey);
};

#endif //GXECDSA_H
