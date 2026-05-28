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

#include "../include/GXx509Certificate.h"
#include "../include/GXEcdsa.h"

CGXx509Certificate::CGXx509Certificate()
{
    m_BasicConstraints = false;
    m_Version = DLMS_CERTIFICATE_VERSION_3;
    m_SignatureAlgorithm = DLMS_HASH_ALGORITHM_NONE;
    m_PublicKeyAlgorithm = DLMS_HASH_ALGORITHM_NONE;
    m_KeyUsage = DLMS_KEY_USAGE_NONE;
    m_ExtendedKeyUsage = DLMS_EXTENDED_KEY_USAGE_NONE;
}

int CGXx509Certificate::GetFilePath(
    CGXx509Certificate& cert,
    std::string& path)
{
    if ((cert.m_KeyUsage & (DLMS_KEY_USAGE_DIGITAL_SIGNATURE | DLMS_KEY_USAGE_KEY_AGREEMENT)) == (DLMS_KEY_USAGE_DIGITAL_SIGNATURE | DLMS_KEY_USAGE_KEY_AGREEMENT))
    {
        path = "T";
    }
    else if ((cert.m_KeyUsage & DLMS_KEY_USAGE_DIGITAL_SIGNATURE) != 0)
    {
        path = "D";
    }
    else if ((cert.m_KeyUsage & DLMS_KEY_USAGE_KEY_AGREEMENT) != 0)
    {
        path = "A";
    }
    else
    {
        printf("Unknown certificate type.");
        return DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    std::string tmp;
    CGXAsn1Converter::HexSystemTitleFromSubject(cert.GetSubject(), tmp);
    path += tmp;
    path += ".pem";
    CGXByteBuffer bb;
    cert.m_PublicKey.GetEncoded(bb);
    if (bb.GetSize() < 100)
    {
        path = "Certificates/" + path;
    }
    else
    {
        path = "Certificates384/" + path;
    }
    return 0;
}

int CGXx509Certificate::GetFilePath(
    ECC ecc,
    DLMS_KEY_USAGE usage,
    std::string& systemTitle,
    std::string& path)
{
    if ((usage & (DLMS_KEY_USAGE_DIGITAL_SIGNATURE | DLMS_KEY_USAGE_KEY_AGREEMENT)) ==
        (DLMS_KEY_USAGE_DIGITAL_SIGNATURE | DLMS_KEY_USAGE_KEY_AGREEMENT))
    {
        path = "T";
    }
    else if ((usage & DLMS_KEY_USAGE_DIGITAL_SIGNATURE) != 0)
    {
        path = "D";
    }
    else if ((usage & DLMS_KEY_USAGE_KEY_AGREEMENT) != 0)
    {
        path = "A";
    }
    else
    {
        printf("Unknown certificate type.");
        return DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    path += systemTitle;
    path += ".pem";
    if (ecc == ECC_P256)
    {
        path = "Certificates/" + path;
    }
    else if (ecc == ECC_P384)
    {
        path = "Certificates384/" + path;
    }
    else
    {
        return DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    return 0;
}

int CGXx509Certificate::FromHexString(
    std::string& data,
    CGXx509Certificate& cert)
{
    CGXByteBuffer bb;
    GXHelpers::HexToBytes(data, bb);
    cert.Init(bb);
    return 0;
}

int CGXx509Certificate::FromPem(
    std::string data,
    CGXx509Certificate& cert)
{
    std::string START = "BEGIN CERTIFICATE-----\n";
    std::string END = "-----END";
    GXHelpers::Replace(data, "\r\n", "\n");
    size_t start = data.find(START);
    if (start == std::string::npos)
    {
        printf("Invalid PEM file.");
        return DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    data = data.substr(start + START.length());
    size_t end = data.find(END);
    if (end == std::string::npos)
    {
        printf("Invalid PEM file.");
        return DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    data = data.substr(0, end);
    return FromDer(data, cert);
}

int CGXx509Certificate::FromDer(
    std::string& data,
    CGXx509Certificate& cert)
{
    int ret;
    CGXByteBuffer bb;
    if ((ret = bb.FromBase64(data)) == 0)
    {
        ret = cert.Init(bb);
    }
    return ret;
}

int CGXx509Certificate::FromByteArray(
    CGXByteBuffer& data,
    CGXx509Certificate& cert)
{
    return cert.Init(data);
}

int CGXx509Certificate::FromByteArray(
    const unsigned char* data,
    uint16_t length,
    CGXx509Certificate& cert)
{
    CGXByteBuffer bb;
    bb.Set(data, length);
    return cert.Init(bb);
}

int CGXx509Certificate::GetAlgorithm(
    std::string& algorithm,
    std::string& value)
{
    if (GXHelpers::EndsWith(algorithm, "RSA"))
    {
        value = "RSA";
    }
    else if (GXHelpers::EndsWith(algorithm, "ECDSA"))
    {
        value = "EC";
    }
    else
    {
        printf("Unknown algorithm: %s", algorithm.c_str());
        return DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    return 0;
}

int CGXx509Certificate::UpdateVersion(CGXAsn1Context* tmp)
{
    if (CGXAsn1Variant* value = dynamic_cast<CGXAsn1Variant*>(tmp->GetValues()->at(0)))
    {
        m_Version = (DLMS_CERTIFICATE_VERSION)value->GetValue().bVal;
    }
    else
    {
        return DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    return 0;
}

int CGXx509Certificate::UpdateSerialNumber(CGXAsn1Sequence* reqInfo)
{
    if (CGXAsn1Integer* value = dynamic_cast<CGXAsn1Integer*>(reqInfo->GetValues()->at(1)))
    {
        m_SerialNumber = CGXBigInteger(value->GetValue());
    }
    else
    {
        if (CGXAsn1Variant* value = dynamic_cast<CGXAsn1Variant*>(reqInfo->GetValues()->at(1)))
        {
            if (value->GetValue().vt == DLMS_DATA_TYPE_OCTET_STRING)
            {
                int size = value->GetValue().GetSize();
                if (size < 0)
                {
                    return DLMS_ERROR_CODE_INVALID_PARAMETER;
                }
                CGXByteBuffer bb;
                bb.Set(value->GetValue().byteArr, size);
                m_SerialNumber = CGXBigInteger(bb);
            }
            else
            {
                m_SerialNumber = CGXBigInteger(value->GetValue().ToInteger());
            }
        }
        else
        {
            return DLMS_ERROR_CODE_INVALID_PARAMETER;
        }
    }
    return 0;
}

int CGXx509Certificate::UpdateSignatureAlgorithm(CGXAsn1Sequence* reqInfo)
{
    int ret = 0;
    if (CGXAsn1Sequence* tmp = dynamic_cast<CGXAsn1Sequence*>(reqInfo->GetValues()->at(2)))
    {
        if (CGXAsn1ObjectIdentifier* value = dynamic_cast<CGXAsn1ObjectIdentifier*>(tmp->GetValues()->at(0)))
        {
            m_SignatureAlgorithm = CGXDLMSConverter::ValueOfHashAlgorithm(value->GetObjectIdentifier().c_str());
            if (m_SignatureAlgorithm != DLMS_HASH_ALGORITHM_SHA_256_WITH_ECDSA &&
                m_SignatureAlgorithm != DLMS_HASH_ALGORITHM_SHA_384_WITH_ECDSA)
            {
                ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
            }
        }
        else
        {
            ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
        }
    }
    else
    {
        ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    return ret;
}

int CGXx509Certificate::UpdateIssuer(CGXAsn1Sequence* reqInfo)
{
    int ret;
    if (CGXAsn1Sequence* tmp = dynamic_cast<CGXAsn1Sequence*>(reqInfo->GetValues()->at(3)))
    {
        ret = CGXAsn1Converter::ToByteArray(tmp, m_IssuerRaw);
        if (ret == 0)
        {
            ret = CGXAsn1Converter::GetSubject(tmp, m_Issuer);
        }
    }
    else
    {
        ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    return ret;
}

int CGXx509Certificate::UpdateValidity(CGXAsn1Sequence* reqInfo)
{
    int ret;
    if (CGXAsn1Sequence* value = dynamic_cast<CGXAsn1Sequence*>(reqInfo->GetValues()->at(4)))
    {
        if (CGXAsn1Sequence* tmp = dynamic_cast<CGXAsn1Sequence*>(value))
        {
            if (CGXAsn1Time* tm = dynamic_cast<CGXAsn1Time*>(tmp->GetValues()->at(0)))
            {
                m_ValidFrom = tm->GetValue().GetValue();
            }
            if (CGXAsn1Time* tm = dynamic_cast<CGXAsn1Time*>(tmp->GetValues()->at(1)))
            {
                m_ValidTo = tm->GetValue().GetValue();
            }
            if (CGXAsn1Sequence* tmp = dynamic_cast<CGXAsn1Sequence*>(reqInfo->GetValues()->at(5)))
            {
                ret = CGXAsn1Converter::GetSubject(tmp, m_Subject);
            }
            else
            {
                ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
            }
        }
        else
        {
            ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
        }
    }
    else
    {
        ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    return ret;
}
int CGXx509Certificate::UpdateSubjectKeyIdentifier(CGXAsn1Base* value)
{
    int ret;
    if (CGXAsn1Variant* s = dynamic_cast<CGXAsn1Variant*>(value))
    {
        m_SubjectKeyIdentifier.Clear();
        ret = m_SubjectKeyIdentifier.Set(s->GetValue().byteArr, s->GetValue().size);
    }
    else
    {
        ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    return ret;
}

int CGXx509Certificate::UpdateKeyUsage(CGXAsn1Sequence* s,
    CGXAsn1Base* value)
{
    int ret = 0;
    if (CGXAsn1BitString* tmp = dynamic_cast<CGXAsn1BitString*>(value))
    {
        // critical is optional. BOOLEAN DEFAULT FALSE,
        m_KeyUsage = (DLMS_KEY_USAGE)tmp->ToInteger();
    }
    else if (dynamic_cast<CGXAsn1Variant*>(value))
    {
        value = s->GetValues()->at(2);
        if (CGXAsn1BitString* tmp = dynamic_cast<CGXAsn1BitString*>(value))
        {
            // critical is optional. BOOLEAN DEFAULT FALSE,
            m_KeyUsage = (DLMS_KEY_USAGE)tmp->ToInteger();
        }
        else
        {
            ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
        }
    }
    else
    {
        ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    return ret;
}

int CGXx509Certificate::UpdateBasicConstraints(CGXAsn1Base* value)
{
    int ret = 0;
    if (CGXAsn1Sequence* tmp = dynamic_cast<CGXAsn1Sequence*>(value))
    {
        if (tmp->GetValues()->size() != 0)
        {
            if (CGXAsn1Variant* tmp2 = dynamic_cast<CGXAsn1Variant*>(tmp->GetValues()->at(0)))
            {
                m_BasicConstraints = tmp2->GetValue().bVal != 0;
            }
        }
    }
    else if (CGXAsn1Variant* tmp2 = dynamic_cast<CGXAsn1Variant*>(value))
    {
        m_BasicConstraints = tmp2->GetValue().bVal != 0;
    }
    else
    {
        ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    return ret;
}

int CGXx509Certificate::UpdateExtendedKeyUsage(CGXAsn1Base* value)
{
    int ret = 0;
    if (CGXAsn1Sequence* tmp = dynamic_cast<CGXAsn1Sequence*>(value))
    {
        for (std::vector<CGXAsn1Base*>::iterator it = tmp->GetValues()->begin(); it != tmp->GetValues()->end(); ++it)
        {
            if (CGXAsn1ObjectIdentifier* eku = dynamic_cast<CGXAsn1ObjectIdentifier*>(*it))
            {
                if (eku->GetObjectIdentifier() == "1.3.6.1.5.5.7.3.1")
                {
                    m_ExtendedKeyUsage = (DLMS_EXTENDED_KEY_USAGE)(m_ExtendedKeyUsage | DLMS_EXTENDED_KEY_USAGE_SERVER_AUTH);
                }
                else if (eku->GetObjectIdentifier() == "1.3.6.1.5.5.7.3.2")
                {
                    m_ExtendedKeyUsage = (DLMS_EXTENDED_KEY_USAGE)(m_ExtendedKeyUsage | DLMS_EXTENDED_KEY_USAGE_CLIENT_AUTH);
                }
                else
                {
                    printf("Invalid extended key usage.");
                    ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
                    break;
                }
            }
        }
    }
    else
    {
        ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    return ret;
}

int CGXx509Certificate::UpdateAuthorityKeyIdentifier(CGXAsn1Base* value)
{
    int ret = 0;
    if (CGXAsn1Sequence* s = dynamic_cast<CGXAsn1Sequence*>(value))
    {
        for (std::vector<CGXAsn1Base*>::iterator it = s->GetValues()->begin(); it != s->GetValues()->end(); ++it)
        {
            if (CGXAsn1Context* a = dynamic_cast<CGXAsn1Context*>(*it))
            {
                switch (a->GetIndex())
                {
                case 0:
                    // Authority Key Identifier.
                    if (CGXAsn1Variant* value = dynamic_cast<CGXAsn1Variant*>(a->GetValues()->at(0)))
                    {
                        m_AuthorityKeyIdentifier.Set(value->GetValue().byteArr, value->GetValue().size);
                    }
                    break;
                case 1:
                {
                    std::string sb;
                    if (CGXAsn1Context* a2 = dynamic_cast<CGXAsn1Context*>(a->GetValues()->at(0)))
                    {
                        if (CGXAsn1Sequence* value = dynamic_cast<CGXAsn1Sequence*>(a2->GetValues()->at(0)))
                        {
                            for (std::vector<CGXAsn1Base*>::iterator kp = value->GetValues()->begin(); kp != value->GetValues()->end(); ++kp)
                            {
                                if (CGXAsn1Set* it2 = dynamic_cast<CGXAsn1Set*>(*kp))
                                {
                                    DLMS_X509_NAME name = CGXDLMSConverter::ValueOfx509Name(it2->GetKey()->ToString().c_str());
                                    sb += CGXDLMSConverter::GetName(name);
                                    sb += "=";
                                    sb += it2->GetValue()->ToString();
                                    sb += ", ";
                                }
                            }
                            // Remove last comma.
                            if (sb.length() != 0)
                            {
                                sb.erase(sb.length() - 2);
                            }
                            m_AuthorityCertIssuer = sb;
                        }
                    }
                }
                break;
                case 2:
                    // Authority cert serial number.
                    if (CGXAsn1Variant* value = dynamic_cast<CGXAsn1Variant*>(a->GetValues()->at(0)))
                    {
                        m_AuthorityCertificationSerialNumber.Set(value->GetValue().byteArr, value->GetValue().size);
                    }
                    break;
                default:
#if defined(_WIN32) || defined(_WIN64) || defined(__linux__)
                    printf("Invalid context %d.\r\n", a->GetIndex());
#endif//defined(_WIN32) || defined(_WIN64) || defined(__linux__)
                    return DLMS_ERROR_CODE_INVALID_PARAMETER;
                }
            }
        }
    }
    return ret;
}

int CGXx509Certificate::UpdateStandardExtensions(
    CGXAsn1Sequence* reqInfo,
    bool& basicConstraintsExists)
{
    int ret = 0;
    CGXAsn1Base* value = NULL;
    if (dynamic_cast<CGXAsn1Sequence*>(reqInfo->GetValues()->at(4)))
    {
        // Get Standard Extensions.
        if (reqInfo->GetValues()->size() > 7)
        {
            if (CGXAsn1Context* list = dynamic_cast<CGXAsn1Context*>(reqInfo->GetValues()->at(7)))
            {
                if (CGXAsn1Sequence* s = dynamic_cast<CGXAsn1Sequence*>(list->GetValues()->at(0)))
                {
                    DLMS_X509_CERTIFICATE_TYPE t;
                    for (std::vector<CGXAsn1Base*>::iterator tmp = s->GetValues()->begin(); tmp != s->GetValues()->end(); ++tmp)
                    {
                        if (CGXAsn1Sequence* s = dynamic_cast<CGXAsn1Sequence*>(*tmp))
                        {
                            if (CGXAsn1ObjectIdentifier* id = dynamic_cast<CGXAsn1ObjectIdentifier*>(s->GetValues()->at(0)))
                            {
                                t = CGXDLMSConverter::ValueOfSourceDiagnosticX509CertificateType(id->GetObjectIdentifier().c_str());
                                value = s->GetValues()->at(1);
                                switch (t)
                                {
                                case DLMS_X509_CERTIFICATE_TYPE_SUBJECT_KEY_IDENTIFIER:
                                    ret = UpdateSubjectKeyIdentifier(value);
                                    break;
                                case DLMS_X509_CERTIFICATE_TYPE_AUTHORITY_KEY_IDENTIFIER:
                                {
                                    ret = UpdateAuthorityKeyIdentifier(value);
                                }
                                break;
                                case DLMS_X509_CERTIFICATE_TYPE_KEY_USAGE:
                                    ret = UpdateKeyUsage(s, value);
                                    break;
                                case DLMS_X509_CERTIFICATE_TYPE_EXTENDED_KEY_USAGE:
                                    ret = UpdateExtendedKeyUsage(value);
                                    break;
                                case DLMS_X509_CERTIFICATE_TYPE_BASIC_CONSTRAINTS:
                                    basicConstraintsExists = true;
                                    ret = UpdateBasicConstraints(value);
                                    break;
                                default:
                                    printf("Unknown extensions: %s\r", id->ToString().c_str());
                                    break;
                                }
                            }
                            else
                            {
                                ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
                                break;
                            }
                        }
                        else
                        {
                            ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
                        }
                        if (ret != 0)
                        {
                            break;
                        }
                    }
                }
                else
                {
                    ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
                }
            }
            else
            {
                ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
            }
        }
    }
    else
    {
        ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    return ret;
}

int CGXx509Certificate::Init(CGXByteBuffer& data)
{
    m_RawData = data;
    CGXAsn1Base* value = NULL;
    bool basicConstraintsExists = false;
    int ret = CGXAsn1Converter::FromByteArray(data, value);
    if (ret == 0)
    {
        if (CGXAsn1Sequence* seq = dynamic_cast<CGXAsn1Sequence*>(value))
        {
            if (seq->GetValues()->size() != 3)
            {
                printf("Wrong number of elements in sequence.");
                ret = DLMS_ERROR_CODE_INVALID_DATA_FORMAT;
            }
            else if (CGXAsn1Sequence* reqInfo = dynamic_cast<CGXAsn1Sequence*>(seq->GetValues()->at(0)))
            {
                if (CGXAsn1Context* tmp = dynamic_cast<CGXAsn1Context*>(reqInfo->GetValues()->at(0)))
                {
                    if ((ret = UpdateVersion(tmp)) != 0 ||
                        (ret = UpdateSerialNumber(reqInfo)) != 0 ||
                        (ret = UpdateSignatureAlgorithm(reqInfo)) != 0 ||
                        (ret = UpdateIssuer(reqInfo)) != 0 ||
                        (ret = UpdateValidity(reqInfo)) != 0 ||
                        (ret = UpdateStandardExtensions(reqInfo, basicConstraintsExists)) != 0)
                    {
                        delete value;
                        return ret;
                    }
                    CGXAsn1Sequence* subjectPKInfo = dynamic_cast<CGXAsn1Sequence*>(reqInfo->GetValues()->at(6));
                    if (CGXAsn1BitString* bs = dynamic_cast<CGXAsn1BitString*>(subjectPKInfo->GetValues()->at(1)))
                    {
                        if ((ret = CGXPublicKey::FromRawBytes(bs->GetValue(), m_PublicKey)) != 0)
                        {
                            delete value;
                            return ret;
                        }
                        ret = CGXEcdsa::Validate(m_PublicKey);
                        if (ret != 0)
                        {
                            delete value;
                            return ret;
                        }
                    }
                    if (!basicConstraintsExists)
                    {
                        std::string CN = CGXDLMSConverter::ToString(DLMS_X509_NAME_CN);
                        // Verify that subject Common Name includes system title.
                        bool commonNameFound = false;
                        CGXAsn1Sequence* tmp = dynamic_cast<CGXAsn1Sequence*>(reqInfo->GetValues()->at(5));
                        {
                            for (std::vector<CGXAsn1Base*>::iterator it = tmp->GetValues()->begin();
                                it != tmp->GetValues()->end(); ++it)
                            {
                                CGXAsn1Set* tmp2 = dynamic_cast<CGXAsn1Set*>(*it);
                                {
                                    if (CN == tmp2->GetKey()->ToString())
                                    {
                                        if (tmp2->GetValue()->ToString().length() != 16)
                                        {
                                            printf("System title is not included in Common Name.");
                                            delete value;
                                            return DLMS_ERROR_CODE_INVALID_PARAMETER;
                                        }
                                        commonNameFound = true;
                                        break;
                                    }
                                }
                            }
                        }
                        if (!commonNameFound)
                        {
                            printf("Common name doesn't exist.\n");
                            delete value;
                            return DLMS_ERROR_CODE_INVALID_PARAMETER;
                        }
                    }
                    if (m_KeyUsage == DLMS_KEY_USAGE_NONE)
                    {
                        printf("Key usage not present. It's mandotory.\n");
                        delete value;
                        return DLMS_ERROR_CODE_INVALID_PARAMETER;
                    }
                    if ((m_KeyUsage & (DLMS_KEY_USAGE_KEY_CERT_SIGN | DLMS_KEY_USAGE_CRL_SIGN)) != 0 && !basicConstraintsExists)
                    {
                        printf("Basic Constraints value not present. It's mandotory.");
                        delete value;
                        return DLMS_ERROR_CODE_INVALID_PARAMETER;
                    }
                    if (m_KeyUsage == (DLMS_KEY_USAGE_DIGITAL_SIGNATURE | DLMS_KEY_USAGE_KEY_AGREEMENT) &&
                        m_ExtendedKeyUsage == DLMS_EXTENDED_KEY_USAGE_NONE)
                    {
                        printf("Extended key usage not present. It's mandotory for TLS.");
                        delete value;
                        return DLMS_ERROR_CODE_INVALID_PARAMETER;
                    }
                    if (m_ExtendedKeyUsage != DLMS_EXTENDED_KEY_USAGE_NONE &&
                        m_KeyUsage != (DLMS_KEY_USAGE_DIGITAL_SIGNATURE | DLMS_KEY_USAGE_KEY_AGREEMENT))
                    {
                        printf("Extended key usage present. It's used only for TLS.");
                        delete value;
                        return DLMS_ERROR_CODE_INVALID_PARAMETER;
                    }
                    CGXAsn1Sequence* tmp2 = dynamic_cast<CGXAsn1Sequence*>(seq->GetValues()->at(1));
                    {
                        m_PublicKeyAlgorithm = CGXDLMSConverter::ValueOfHashAlgorithm(tmp2->GetValues()->at(0)->ToString().c_str());
                    }
                    if (m_PublicKeyAlgorithm != DLMS_HASH_ALGORITHM_SHA_256_WITH_ECDSA &&
                        m_PublicKeyAlgorithm != DLMS_HASH_ALGORITHM_SHA_384_WITH_ECDSA)
                    {
                        printf("DLMS certificate must be signed with ECDSA with SHA256 or SHA384.");
                        delete value;
                        return DLMS_ERROR_CODE_INVALID_PARAMETER;
                    }
                    /////////////////////////////
                    //Get signature.
                    if (CGXAsn1BitString* tmp3 = dynamic_cast<CGXAsn1BitString*>(seq->GetValues()->at(2)))
                    {
                        m_Signature = tmp3->GetValue();
                    }
                    else
                    {
                        delete value;
                        ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
                    }
                }
                else
                {
                    ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
                }
            }
            else
            {
                ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
            }
        }
        else
        {
            ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
        }
    }
    delete value;
    return ret;
}

std::string& CGXx509Certificate::GetSubject()
{
    return m_Subject;
}

void CGXx509Certificate::SetSubject(std::string& value)
{
    m_Subject = value;
}

std::string& CGXx509Certificate::GetIssuer()
{
    return m_Issuer;
}

void CGXx509Certificate::SetIssuer(std::string& value)
{
    m_Issuer = value;
}

CGXBigInteger& CGXx509Certificate::GetSerialNumber()
{
    return m_SerialNumber;
}

void CGXx509Certificate::SetSerialNumber(CGXBigInteger& value)
{
    m_SerialNumber = value;
}

DLMS_CERTIFICATE_VERSION CGXx509Certificate::GetVersion()
{
    return m_Version;
}

void CGXx509Certificate::SetVersion(DLMS_CERTIFICATE_VERSION value)
{
    m_Version = value;
}

struct tm& CGXx509Certificate::GetValidFrom()
{
    return m_ValidFrom;
}

void CGXx509Certificate::SetValidFrom(struct tm& value)
{
    m_ValidFrom = value;
}

struct tm& CGXx509Certificate::GetValidTo()
{
    return m_ValidTo;
}

void CGXx509Certificate::SetValidTo(struct tm& value)
{
    m_ValidTo = value;
}

DLMS_HASH_ALGORITHM CGXx509Certificate::GetSignatureAlgorithm()
{
    return m_SignatureAlgorithm;
}

void CGXx509Certificate::SetSignatureAlgorithm(DLMS_HASH_ALGORITHM value)
{
    m_SignatureAlgorithm = value;
}

CGXAsn1Base& CGXx509Certificate::GetParameters()
{
    return m_Parameters;
}

CGXPublicKey& CGXx509Certificate::GetPublicKey()
{
    return m_PublicKey;
}

void CGXx509Certificate::SetPublicKey(CGXPublicKey& value)
{
    m_PublicKey = value;
}

CGXByteBuffer& CGXx509Certificate::GetSignature()
{
    return m_Signature;
}

void CGXx509Certificate::SetSignature(CGXByteBuffer& value)
{
    m_Signature = value;
}

/**
 * Returns Encoded x509 certificate.
 */
int CGXx509Certificate::GetEncoded(
    CGXByteBuffer& value)
{
    if (m_RawData.GetSize() != 0)
    {
        value = m_RawData;
        return 0;
    }
    return DLMS_ERROR_CODE_INVALID_PARAMETER;
}

std::string CGXx509Certificate::ToString()
{
    int ret;
    char buff[50];
    std::string bb;
    bb += "Version: ";
    bb += (int)m_Version;
    bb += "\n";
    bb += "Serial Number: ";
    bb += m_SerialNumber.ToString();
    bb += "\n";
    bb += "Signature Algorithm: ";
    bb += CGXDLMSConverter::ToString(m_SignatureAlgorithm);
    bb += ", OID = ";
    bb += CGXDLMSConverter::ToString(m_SignatureAlgorithm);
    bb += "\n";
    bb += "Issuer: ";
    bb += m_Issuer;
    bb += "\n";
    bb += "Validity: [From: ";
    ret = (int)strftime(buff, 50, "%x %X", &m_ValidFrom);
    bb.append(buff, ret);
    bb += ", \n";
    bb += "To: ";
    ret = (int)strftime(buff, 50, "%x %X", &m_ValidTo);
    bb.append(buff, ret);
    bb += "]\n";
    bb += "Subject Public Key Info:\n";
    bb += "Public Key Algorythm: ";
    bb += CGXDLMSConverter::ToString(m_PublicKeyAlgorithm);
    bb += "\n";
    bb += m_PublicKey.ToString();
    bb += "\n";
    if (m_SubjectKeyIdentifier.GetSize() != 0)
    {
        bb += "X509v3 Subject Key Identifier:\n";
        bb += m_SubjectKeyIdentifier.ToHexString();
        bb += "\n";
    }
    if (m_AuthorityKeyIdentifier.GetSize() != 0)
    {
        bb += "X509v3 Authority Key Identifier:\n";
        bb += m_AuthorityKeyIdentifier.ToHexString();
        bb += "\n";
    }
    bb += "Signature Algorithm: ";
    bb += CGXDLMSConverter::ToString(m_SignatureAlgorithm);
    bb += "\n";
    bb += m_Signature.ToHexString();
    bb += "\n";
    return bb;
}

DLMS_KEY_USAGE CGXx509Certificate::GetKeyUsage()
{
    return m_KeyUsage;
}

void CGXx509Certificate::SetKeyUsage(DLMS_KEY_USAGE value)
{
    m_KeyUsage = value;
}

DLMS_EXTENDED_KEY_USAGE CGXx509Certificate::GetExtendedKeyUsage()
{
    return m_ExtendedKeyUsage;
}

void CGXx509Certificate::SetExtendedKeyUsage(DLMS_EXTENDED_KEY_USAGE value) {
    m_ExtendedKeyUsage = value;
}

CGXByteBuffer& CGXx509Certificate::GetSubjectKeyIdentifier()
{
    return m_SubjectKeyIdentifier;
}

void CGXx509Certificate::SetSubjectKeyIdentifier(CGXByteBuffer& value)
{
    m_SubjectKeyIdentifier = value;
}

CGXByteBuffer& CGXx509Certificate::GetAuthorityKeyIdentifier()
{
    return m_AuthorityKeyIdentifier;
}

void CGXx509Certificate::SetAuthorityKeyIdentifier(CGXByteBuffer& value)
{
    m_AuthorityKeyIdentifier = value;
}

bool CGXx509Certificate::IsBasicConstraints()
{
    return m_BasicConstraints;
}

void CGXx509Certificate::SetBasicConstraints(bool value)
{
    m_BasicConstraints = value;
}

#if defined(_WIN32) || defined(_WIN64) || defined(__linux__)

int CGXx509Certificate::Load(
    std::string& path,
    CGXx509Certificate& cert)
{
    std::string text;
    int ret = GXHelpers::Load(path, text);
    if (ret != 0)
    {
        return ret;
    }
    return FromPem(text, cert);
}

int CGXx509Certificate::Save(std::string& path)
{
    std::string text;
    int ret = ToPem(text);
    if (ret != 0)
    {
        return ret;
    }
    return GXHelpers::Save(path, text);
}
#endif //defined(_WIN32) || defined(_WIN64) || defined(__linux__)

int CGXx509Certificate::ToPem(std::string& value)
{
    int ret;
    value.clear();
    if (m_PublicKey.GetRawValue().GetSize() == 0)
    {
        printf("Public or key is not set.");
        ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    else
    {
        std::string der;
        if ((ret = ToDer(der)) == 0)
        {
            value += "-----BEGIN CERTIFICATE-----\n";
            value += der;
            value += "\n-----END CERTIFICATE-----\n";
        }
    }
    return ret;
}

int CGXx509Certificate::ToDer(std::string& value)
{
    value.clear();
    CGXByteBuffer bb;
    int ret = GetEncoded(bb);
    if (ret == 0)
    {
        ret = bb.ToBase64(value);
    }
    return ret;
}

CGXByteBuffer& CGXx509Certificate::GetIssuerRaw()
{
    return m_IssuerRaw;
}

bool CGXx509Certificate::Equals(CGXx509Certificate& cert)
{
    return m_SerialNumber.Compare(cert.m_SerialNumber) == 0;
}

int CGXx509Certificate::GetSystemTitle(CGXByteBuffer& value)
{
    value.Clear();
    if (m_Subject.empty())
    {
        return DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    std::string tmp;
    int ret = CGXAsn1Converter::HexSystemTitleFromSubject(m_Subject, tmp);
    if (ret == 0)
    {
        GXHelpers::HexToBytes(tmp, value);
        if (value.GetSize() != 8)
        {
            ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
        }
    }
    return ret;
}