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

#ifndef CGXPKCS10_H
#define CGXPKCS10_H

#include "GXHelpers.h"
#include "GXByteArray.h"
#include "GXAsn1Base.h"
#include "GXPrivateKey.h"
#include "GXPublicKey.h"
#include "GXx509Certificate.h"
#include "GXCertificateRequest.h"

class CGXPkcs10
{
private:
    /**
    * Loaded PKCS #10 certificate as a raw data.
    */
    CGXByteArray m_RawData;
    /**
     * Certificate version.
     */
    DLMS_CERTIFICATE_VERSION m_Version;

    /**
     * Subject.
     */
    std::string m_Subject;

    /**
     * Collection of attributes providing additional information about the
     * subject of the certificate.
     */
    std::vector< std::pair<DLMS_PKCS_OBJECT_IDENTIFIER, std::vector<CGXAsn1Base*> > > m_Attributes;

    /**
     * Algorithm.
     */
    DLMS_X9_OBJECT_IDENTIFIER m_Algorithm;

    /**
     * Subject key.
     */
    CGXPublicKey m_PublicKey;

    /**
     * Signature algorithm.
     */
    DLMS_HASH_ALGORITHM m_SignatureAlgorithm;

    CGXByteArray m_Signature;

private:
    int GetData(CGXAsn1Sequence* value);

    int Init(CGXByteBuffer& data);
public:
    /**
     * Constructor.
     */
    CGXPkcs10();

    /**
     * Constructor.
     *
     * data: Encoded bytes.
     * cert: Pkcs10 certificate.
     */
    static int FromHexString(std::string& data,
        CGXPkcs10& cert);

    /**
    * Create Pkcs10 from byte array.
    *
    *data: Byte buffer.
    *length: The length of the byte buffer.
    */
    static int FromByteArray(const unsigned char* data,
        uint16_t length,
        CGXPkcs10& cert);

    /**
    * Create Pkcs10 from byte array.
    *
    *data: Byte array.
    */
    static int FromByteArray(CGXByteBuffer& data,
        CGXPkcs10& cert);

    /**
     * Create GXPkcs10 certificate request from PEM string.
     *
     * data: PEM string.
     * cert: GXPkcs10 certificate request.
     * Returns Pkcs10 certificate.
     */
    static int FromPem(std::string data,
        CGXPkcs10& cert);

    /**
     * Create x509Certificate from DER Base64 encoded string.
     *
     * data
     *            Base64 DER string.
     * Returns x509 certificate.
     */
    static int FromDer(std::string& data,
        CGXPkcs10& cert);

    /**
     * Constructor.
     *
     * data: Encoded bytes.
     * cert: Pkcs10 certificate.
     */
    static int FromHexString(CGXByteBuffer& data,
        CGXPkcs10& cert);

    /**
     * Returns Certificate version.
     */
    DLMS_CERTIFICATE_VERSION GetVersion();

    /**
     * value: Certificate version.
     */
    void SetVersion(DLMS_CERTIFICATE_VERSION value);

    /**
     * Returns Subject.
     */
    std::string& GetSubject();

    /**
     * value
     *            Subject.
     */
    void SetSubject(std::string& value);

    /**
     * Returns Subject key info.
     */
    CGXPublicKey& GetPublicKey();

    /**
     * value: Subject key info.
     */
    void SetPublicKey(CGXPublicKey& value);

    /**
     * Returns Algorithm
     */
    DLMS_X9_OBJECT_IDENTIFIER GetAlgorithm();

    /**
     * value: Algorithm
     */
    void SetAlgorithm(DLMS_X9_OBJECT_IDENTIFIER value);

    /**
     * Returns signature algorithm.
     */
    DLMS_HASH_ALGORITHM GetSignatureAlgorithm();

    /**
     * value: Signature algorithm.
     */
    void SetSignatureAlgorithm(DLMS_HASH_ALGORITHM value);

    /**
     * Returns Signature.
     */
    CGXByteArray& GetSignature();

    /**
     * value
     *            Signature.
     */
    void SetSignature(CGXByteBuffer& value);

    std::string ToString();

    /**
     * Returns PKCS #10 certificate as a byte array.
     */
    int GetEncoded(CGXByteBuffer& value);

    /**
     * Sign
     *
     * kp: Private key.
     * hashAlgorithm: Used algorithm for signing.
     */
    int Sign(CGXPrivateKey& kp,
        DLMS_HASH_ALGORITHM hashAlgorithm);

    /**
     * Create Certificate Signing Request.
     *
     * kp: Key pair.
     * subject: Subject.
     * Returns created GXPkcs10.
     */
    static int CreateCertificateSigningRequest(
        std::pair<CGXPublicKey, CGXPrivateKey>& kp,
        std::string& subject,
        CGXPkcs10& pkc10);

    /**
     * Ask Gurux certificate server to generate the new certificate.
     *
     * certifications
     *            std::vector of certification requests.
     * Returns Generated certificate(s).
     */
    static int GetCertificate(
        std::vector<CGXCertificateRequest>& certifications,
        std::vector <CGXx509Certificate>& certificates);

#if defined(_WIN32) || defined(_WIN64) || defined(__linux__)
    /**
    * Load Certificate Signing Request from the PEM file.
    *
    * path
    *            File path.
    * cert: Created GXPkcs10 object.
    */
    static int Load(std::string& path,
        CGXPkcs10& cert);

    /**
    * Load Certificate Signing Request from the PEM file.
    *
    * path
    *            File path.
    * cert: Created GXPkcs10 object.
    */
    static int Load(const char* path,
        CGXPkcs10& cert);

    /**
     * Save Certificate Signing Request to PEM file.
     *
     * path: File path.
     */
    int Save(std::string& path);

    /**
     * Save Certificate Signing Request to PEM file.
     *
     * path: File path.
     */
    int Save(const char* path);
#endif //defined(_WIN32) || defined(_WIN64) || defined(__linux__)

    /**
     * Returns Certificate Signing Request in PEM format.
     */
    int ToPem(std::string& value);

    /**
     * Returns Certificate Signing Request in DER format.
     */
    int ToDer(std::string& value);

    //Is the content of the objects equal.
    bool Equals(CGXPkcs10& cert);
};

#endif //CGXPKCS10_H
