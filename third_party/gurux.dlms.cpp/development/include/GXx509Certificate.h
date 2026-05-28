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
// as published by the Free Software Foundation; m_Version 2 of the License.
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

#ifndef GXX509CERTIFICATE_H
#define GXX509CERTIFICATE_H

#include <vector>
#include "enums.h"
#include "GXAsn1Base.h"
#include "GXDLMSVariant.h"
#include "GXPublicKey.h"
#include "GXAsn1Converter.h"

class CGXx509Certificate
{
private:
    friend class CGXPkcs10;
    friend class CGXAPDU;
    /**
    * Loaded x509 Certificate certificate as a raw data.
    */
    CGXByteBuffer m_RawData;

    /**
     * This extension identifies the key being certified.
     */
    CGXByteBuffer m_SubjectKeyIdentifier;
    /**
     * May be used either as a certificate or CRL extension. It identifies the
     * key to be used to verify the m_Signature on this certificate or CRL.
     * It enables distinct keys used by the same CA to be distinguished.
     */
    CGXByteBuffer m_AuthorityKeyIdentifier;

    /**
     * Authority certification serial number.
     */
    CGXByteBuffer m_AuthorityCertificationSerialNumber;

    /**
     * Indicates if the m_Subject may act as a CA.
     */
    bool m_BasicConstraints;

    /**
     * Signature algorithm.
     */
    DLMS_HASH_ALGORITHM m_SignatureAlgorithm;
    /**
     * Signature m_Parameters.
     */
    CGXAsn1Base m_SignatureParameters;

    /**
     * Public key.
     */
    CGXPublicKey m_PublicKey;

    /**
     * Algorithm.
     */
    DLMS_HASH_ALGORITHM m_PublicKeyAlgorithm;

    /**
     * Parameters.
     */
    CGXAsn1Base m_Parameters;

    /**
     * Signature.
     */
    CGXByteBuffer m_Signature;

    /**
     * Subject. Example: "CN=4142434445464748, O=Gurux, L=Tampere, C=FI".
     */
    std::string m_Subject;

    /**
     * Issuer. Example: "CN=Gurux O=Gurux, L=Tampere, C=FI".
     */
    std::string m_Issuer;

    /**
     * Raw Issuer in ASN1 format.
     */
    CGXByteBuffer m_IssuerRaw;

    /**
     * Authority Cert Issuer. Example: "CN=Test O=Gurux, L=Tampere, C=FI".
     */
    std::string m_AuthorityCertIssuer;

    /**
     * Serial number.
     */
    CGXBigInteger m_SerialNumber;

    /**
     * Version.
     */
    DLMS_CERTIFICATE_VERSION m_Version;
    /**
     * Validity from.
     */
    struct tm m_ValidFrom;
    /**
     * Validity to.
     */
    struct tm m_ValidTo;

    /**
     * Indicates the purpose for which the certified key is used.
     */
    DLMS_KEY_USAGE m_KeyUsage;

    /**
     * Indicates that a certificate can be used as an TLS server or client
     * certificate.
     */
    DLMS_EXTENDED_KEY_USAGE m_ExtendedKeyUsage;

    int Init(CGXByteBuffer& data);
private:
    //Internal methods.
    int UpdateVersion(CGXAsn1Context* tmp);
    int UpdateSerialNumber(CGXAsn1Sequence* reqInfo);
    int UpdateSignatureAlgorithm(CGXAsn1Sequence* reqInfo);
    int UpdateIssuer(CGXAsn1Sequence* reqInfo);
    int UpdateValidity(CGXAsn1Sequence* reqInfo);
    int UpdateStandardExtensions(
        CGXAsn1Sequence* reqInfo,
        bool& basicConstraintsExists);
    int UpdateSubjectKeyIdentifier(CGXAsn1Base* value);
    int UpdateKeyUsage(CGXAsn1Sequence* s, CGXAsn1Base* value);
    int UpdateExtendedKeyUsage(CGXAsn1Base* value);
    int UpdateBasicConstraints(CGXAsn1Base* value);
    int UpdateAuthorityKeyIdentifier(CGXAsn1Base* value);
public:
    /////////////////////////////////////////////////////////////////////////////
    // Constructor.
    /////////////////////////////////////////////////////////////////////////////   
    CGXx509Certificate();

    /**
     * cert: Certificate.
     * path: File path.
     * Returns error code.
     */
    static int GetFilePath(
        CGXx509Certificate& cert,
        std::string& path);

    /**
     * ecc: ECC scheme.
     * usage: Certificate key usage.
     * systemTitle: System title.
     * path: Path.
     * Returns error code.
     */
    static int GetFilePath(
        ECC ecc,
        DLMS_KEY_USAGE usage,
        std::string& systemTitle,
        std::string& path);

    /**
     * Create x509Certificate from the hex string.
     *
     * data: x509Certificate as a hex string.
     * cart: x509 certificate
     * Returns error code.
     */
    static int FromHexString(
        std::string& data,
        CGXx509Certificate& cert);

    /**
     * Create x509Certificate from PEM string.
     *
     * data: PEM string.
     * cert: x509 certificate.
     * Returns error code.
     */
    static int FromPem(std::string data,
        CGXx509Certificate& cert);

    /**
     * Create x509Certificate from DER Base64 encoded string.
     *
     * data: Base64 DER string.
     * cert: x509 certificate.
     * Returns error code.
     */
    static int FromDer(std::string& data,
        CGXx509Certificate& cert);

    /**
    Create x509Certificate from byte array.

    data: Byte array.
    */
    static int FromByteArray(CGXByteBuffer& data,
        CGXx509Certificate& cert);

    /**
    * Create x509Certificate from byte array.
    *
    *data: Byte buffer.
    *length: The length of the byte buffer.
    */
    static int FromByteArray(const unsigned char* data,
        uint16_t length,
        CGXx509Certificate& cert);

    static int GetAlgorithm(std::string& algorithm,
        std::string& value);

    /**
     * Returns Subject.
     */
    std::string& GetSubject();

    /**
     * value: Subject.
     */
    void SetSubject(std::string& value);

    /**
     * Returns Issuer.
     */
    std::string& GetIssuer();

    /**
     * value: Issuer.
     */
    void SetIssuer(std::string& value);

    /**
     * Returns Serial number.
     */
    CGXBigInteger& GetSerialNumber();

    /**
     * value: Serial number.
     */
    void SetSerialNumber(CGXBigInteger& value);

    /**
     * Returns Version number.
     */
    DLMS_CERTIFICATE_VERSION GetVersion();

    /**
     * value: Version number.
     */
    void SetVersion(DLMS_CERTIFICATE_VERSION value);

    /**
     * Returns Validity from.
     */
    struct tm& GetValidFrom();

    /**
     * value: Validity from.
     */
    void SetValidFrom(struct tm& value);

    /**
     * Returns Validity to.
     */
    struct tm& GetValidTo();

    /**
     * value: Validity to.
     */
    void SetValidTo(struct tm& value);

    /**
     * Returns Signature algorithm
     */
    DLMS_HASH_ALGORITHM GetSignatureAlgorithm();

    /**
     * value: Signature algorithm.
     */
    void SetSignatureAlgorithm(DLMS_HASH_ALGORITHM value);

    /**
     * Returns Parameters.
     */
    CGXAsn1Base& GetParameters();

    /**
     * Returns Public key.
     */
    CGXPublicKey& GetPublicKey();

    /**
     * value: Public key.
     */
    void SetPublicKey(CGXPublicKey& value);

    /**
     * Returns Signature.
     */
    CGXByteBuffer& GetSignature();

    /**
     * value: Signature.
     */
    void SetSignature(CGXByteBuffer& value);

    /**
     * Returns Encoded x509 certificate.
     */
    int GetEncoded(CGXByteBuffer& value);

    std::string ToString();

    /**
     * Returns Key usage.
     */
    DLMS_KEY_USAGE GetKeyUsage();

    /**
     * value: Key usage.
     */
    void SetKeyUsage(DLMS_KEY_USAGE value);

    /**
     * Returns Indicates that a certificate can be used as an TLS server or
     *         client certificate.
     */
    DLMS_EXTENDED_KEY_USAGE GetExtendedKeyUsage();

    /**
     * value: Indicates that a certificate
     * can be used as an TLS server or
     * client certificate.
     */
    void SetExtendedKeyUsage(DLMS_EXTENDED_KEY_USAGE value);

    /**
     * Returns Identifies the key being certified.
     */
    CGXByteBuffer& GetSubjectKeyIdentifier();

    /**
     * value: Identifies the key being certified.
     */
    void SetSubjectKeyIdentifier(CGXByteBuffer& value);

    /**
     * Returns May be used either as a certificate or
     * CRL extension.
     */
    CGXByteBuffer& GetAuthorityKeyIdentifier();

    /**
     * value: May be used either as a certificate or
     * CRL extension.
     */
    void SetAuthorityKeyIdentifier(CGXByteBuffer& value);

    /**
     * Returns Indicates if the m_Subject may act as a CA.
     */
    bool IsBasicConstraints();

    /**
     * value: Indicates if the m_Subject may act as a CA.
     */
    void SetBasicConstraints(bool value);

#if defined(_WIN32) || defined(_WIN64) || defined(__linux__)
    /**
     * Load key from the PEM file.
     *
     * path File path.
     * Returns Created GXPkcs8 object.
     */
    static int Load(std::string& path,
        CGXx509Certificate& cert);

    /**
     * Save key to PEM file.
     *
     * path: File path.
     */
    int Save(std::string& path);
#endif //defined(_WIN32) || defined(_WIN64) || defined(__linux__)

    /**
     * Returns Public key in PEM format.
     */
    int ToPem(std::string& value);

    /**
     * Returns Public key in DER format.
     */
    int ToDer(std::string& value);

    /**
     * Returns Raw Issuer in ASN1 format.
     */
    CGXByteBuffer& GetIssuerRaw();

    //Is the content of the objects equal.
    bool Equals(CGXx509Certificate& cert);

    /**
     * Returns system title from the certificate.
     */
    int GetSystemTitle(CGXByteBuffer& value);
};

#endif //GXX509CERTIFICATE_H
