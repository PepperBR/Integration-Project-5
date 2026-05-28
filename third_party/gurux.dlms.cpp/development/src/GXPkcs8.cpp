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

#include "../include/GXPkcs8.h"
#include "../include/GXEcdsa.h"

CGXPkcs8::CGXPkcs8()
{
    m_Version = DLMS_CERTIFICATE_VERSION_1;
    m_Algorithm = DLMS_X9_OBJECT_IDENTIFIER_ID_EC_PUBLIC_KEY;
}

int CGXPkcs8::GetFilePath(
    ECC scheme,
    DLMS_CERTIFICATE_TYPE certificateType,
    CGXByteBuffer& systemTitle,
    std::string& path)
{
    switch (certificateType)
    {
    case DLMS_CERTIFICATE_TYPE_DIGITAL_SIGNATURE:
        path = "D";
        break;
    case DLMS_CERTIFICATE_TYPE_KEY_AGREEMENT:
        path = "A";
        break;
    case DLMS_CERTIFICATE_TYPE_TLS:
        path = "T";
        break;
    default:
        return DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    path += systemTitle.ToHexString(false);
    path += ".pem";
    if (scheme == ECC_P256)
    {
        path = "Keys/" + path;
    }
    else
    {
        path = "Keys384/" + path;
    }
    return 0;
}

std::string CGXPkcs8::GetDescription()
{
    return m_Description;
}

void CGXPkcs8::SetDescription(std::string& value)
{
    m_Description = value;
}

DLMS_CERTIFICATE_VERSION CGXPkcs8::GetVersion()
{
    return m_Version;
}

void CGXPkcs8::SetVersion(DLMS_CERTIFICATE_VERSION value)
{
    m_Version = value;
}

CGXPrivateKey& CGXPkcs8::GetPrivateKey()
{
    return m_PrivateKey;
}

void CGXPkcs8::SetPrivateKey(CGXPrivateKey& value)
{
    m_PrivateKey = value;
}

int CGXPkcs8::GetFilePath(ECC scheme,
    DLMS_CERTIFICATE_TYPE certificateType,
    std::string& path,
    CGXByteBuffer& systemTitle)
{
    switch (certificateType)
    {
    case DLMS_CERTIFICATE_TYPE_DIGITAL_SIGNATURE:
        path = "D";
        break;
    case DLMS_CERTIFICATE_TYPE_KEY_AGREEMENT:
        path = "A";
        break;
    case DLMS_CERTIFICATE_TYPE_TLS:
        path = "T";
        break;
    default:
        return DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    path += systemTitle.ToHexString(false) + ".pem";
    if (scheme == ECC_P256)
    {
        path = "Certificates/" + path;
    }
    else
    {
        path = "Certificates384/" + path;
    }
    return 0;
}

int CGXPkcs8::GetEncoded(CGXByteBuffer& encoded)
{
    int ret;
    CGXByteBuffer bb;
    CGXAsn1Sequence* d = new CGXAsn1Sequence();
    d->GetValues()->push_back(new CGXAsn1Variant((char)m_Version));
    CGXAsn1Sequence* d1 = new CGXAsn1Sequence();
    d1->GetValues()->push_back(
        new CGXAsn1ObjectIdentifier(CGXDLMSConverter::ToString(m_Algorithm)));
    CGXAsn1ObjectIdentifier* alg;
    if (m_PublicKey.GetScheme() == ECC_P256)
    {
        alg = new CGXAsn1ObjectIdentifier("1.2.840.10045.3.1.7");
    }
    else
    {
        alg = new CGXAsn1ObjectIdentifier("1.3.132.0.34");
    }
    d1->GetValues()->push_back(alg);
    d->GetValues()->push_back(d1);
    CGXAsn1Sequence* d2 = new CGXAsn1Sequence();
    d2->GetValues()->push_back(new CGXAsn1Variant((char)1));
    d2->GetValues()->push_back(new CGXAsn1Variant(m_PrivateKey.m_RawValue));
    CGXAsn1Context* d3 = new CGXAsn1Context();
    d3->SetIndex(1);
    d3->GetValues()->push_back(new CGXAsn1BitString(m_PublicKey.GetRawValue(), 0));
    d2->GetValues()->push_back(d3);
    if ((ret = CGXAsn1Converter::ToByteArray(d2, bb)) == 0)
    {
        d->GetValues()->push_back(new CGXAsn1Variant(bb));
        ret = CGXAsn1Converter::ToByteArray(d, encoded);
    }
    delete d2; 
    delete d;
    return ret;

}

CGXPkcs8::CGXPkcs8(CGXPrivateKey& key) : CGXPkcs8()
{
    m_PrivateKey = key;
    key.GetPublicKey(m_PublicKey);
}

CGXPkcs8::CGXPkcs8(std::pair<CGXPublicKey, CGXPrivateKey>& pair) : CGXPkcs8()
{
    m_PublicKey = pair.first;
    m_PrivateKey = pair.second;
}

int CGXPkcs8::FromPem(
    std::string data,
    CGXPkcs8& cert)
{
    std::string START = "PRIVATE KEY-----\n";
    std::string END = "-----END";
    GXHelpers::Replace(data, "\r\n", "\n");
    size_t start = data.find(START);
    if (start == std::string::npos)
    {
        return DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    std::string desc;
    if (start != START.length())
    {
        desc = data.substr(0, start);
        std::string DESCRIPTION = "#Description";
        //Check if there is a description metadata.
        size_t descStart = desc.rfind(DESCRIPTION);
        if (descStart != std::string::npos)
        {
            size_t descEnd = desc.find("\n", descStart, start);
            desc = desc.substr(descStart + DESCRIPTION.length(), descStart + DESCRIPTION.length() + descEnd - DESCRIPTION.length());
            GXHelpers::Trim(desc);
        }
        else
        {
            desc = "";
        }
    }
    data = data.substr(start + START.length());
    size_t end = data.find(END);
    if (end == std::string::npos)
    {
        return DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    data = data.substr(0, end);
    int ret = FromDer(data, cert);
    if (ret == 0)
    {
        cert.m_Description = desc;
    }
    return ret;
}

int CGXPkcs8::FromHexString(
    std::string& data,
    CGXPkcs8& cert)
{
    CGXByteBuffer bb;
    GXHelpers::HexToBytes(data, bb);
    return cert.Init(bb);
}

int CGXPkcs8::FromByteArray(CGXByteBuffer& data,
    CGXPkcs8& cert)
{
    return cert.Init(data);
}

int CGXPkcs8::FromByteArray(
    const unsigned char* data,
    uint16_t length,
    CGXPkcs8& cert)
{
    CGXByteBuffer bb;
    bb.Set(data, length);
    return cert.Init(bb);
}

int CGXPkcs8::FromDer(std::string& der,
    CGXPkcs8& cert)
{
    GXHelpers::Replace(der, "\r\n", "");
    GXHelpers::Replace(der, "\n", "");
    CGXByteBuffer bb;
    int ret = bb.FromBase64(der);
    if (ret == 0)
    {
        ret = cert.Init(bb);
    }
    return ret;
}

int CGXPkcs8::Init(CGXByteBuffer& data)
{
    m_RawData = data;
    CGXAsn1Base* value = NULL;
    int ret = CGXAsn1Converter::FromByteArray(data, value);
    if (ret == 0)
    {
        if (CGXAsn1Sequence* seq = dynamic_cast<CGXAsn1Sequence*>(value))
        {
            if (seq->GetValues()->size() < 3)
            {
                //"Wrong number of elements in sequence.";
                ret = DLMS_ERROR_CODE_INVALID_DATA_FORMAT;
            }
            else
            {
                if (CGXAsn1Variant* var = dynamic_cast<CGXAsn1Variant*>(seq->GetValues()->at(0)))
                {
                    if (var->GetValue().vt != DLMS_DATA_TYPE_INT8)
                    {
                        DLMS_PKCS_TYPE type = CGXAsn1Converter::GetCertificateType(data, seq);
                        delete value;
                        switch (type)
                        {
                        case DLMS_PKCS_TYPE_PKCS10:
                            printf("Invalid Certificate. This is PKCS 10 certification requests, not PKCS 8.");
                            return DLMS_ERROR_CODE_INVALID_PARAMETER;
                        case DLMS_PKCS_TYPE_X509_CERTIFICATE:
                            printf("Invalid Certificate. This is PKCS x509 certificate, not PKCS 8.");
                            return DLMS_ERROR_CODE_INVALID_PARAMETER;
                        default:
                            printf("Invalid Certificate Version.");
                            return DLMS_ERROR_CODE_INVALID_PARAMETER;
                        }
                    }
                    m_Version = (DLMS_CERTIFICATE_VERSION)var->GetValue().cVal;
                }
                if (CGXAsn1Sequence* var = dynamic_cast<CGXAsn1Sequence*>(seq->GetValues()->at(1)))
                {
                    if (CGXAsn1ObjectIdentifier* tmp = dynamic_cast<CGXAsn1ObjectIdentifier*>(var->GetValues()->at(0)))
                    {
                        m_Algorithm = CGXDLMSConverter::ValueOfX9Identifier(tmp->GetObjectIdentifier().c_str());
                    }
                }
                else
                {
#ifdef _DEBUG
                    printf("Invalid Certificate. This looks more like private key, not PKCS 8.");
#endif //_DEBUG
                    delete value;
                    return DLMS_ERROR_CODE_INVALID_PARAMETER;
                }
                if (CGXAsn1Sequence* var = dynamic_cast<CGXAsn1Sequence*>(seq->GetValues()->at(2)))
                {
                    if (CGXAsn1Variant* tmp = dynamic_cast<CGXAsn1Variant*>(var->GetValues()->at(1)))
                    {
                        CGXByteBuffer bb;
                        bb.Set(tmp->GetValue().byteArr, tmp->GetValue().size);
                        if ((ret = CGXPrivateKey::FromRawBytes(bb, m_PrivateKey)) != 0)
                        {
#ifdef _DEBUG
                            printf("Invalid private key.\n");
#endif //_DEBUG
                            delete value;
                            return ret;
                        }
                    }
                    if (CGXAsn1Context* tmp = dynamic_cast<CGXAsn1Context*>(var->GetValues()->at(2)))
                    {
                        if (CGXAsn1BitString* tmp2 = dynamic_cast<CGXAsn1BitString*>(tmp->GetValues()->at(0)))
                        {
                            ret = CGXPublicKey::FromRawBytes(tmp2->GetValue(), m_PublicKey);
                            if (ret != 0)
                            {
#ifdef _DEBUG
                                printf("Invalid private key.\n");
#endif //_DEBUG
                            }
                            else
                            {
                                ret = CGXEcdsa::Validate(m_PublicKey);
                            }
                        }
                    }
                }
                else
                {
                    ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
                }
            }
        }
    }
    delete value;
    return ret;
}

std::string CGXPkcs8::ToString()
{
    std::string bb;
    bb += "PKCS #8:\n";
    bb += "Version: ";
    bb += m_Version;
    bb += "Algorithm: ";
    bb += CGXDLMSConverter::ToString(m_Algorithm);
    bb += "PrivateKey: ";
    bb += m_PrivateKey.ToHex();
    bb += "PublicKey: ";
    bb += m_PublicKey.ToString();
    return bb;
}

#if defined(_WIN32) || defined(_WIN64) || defined(__linux__)

int CGXPkcs8::Load(std::string& path, CGXPkcs8& value)
{
    std::string text;
    int ret = GXHelpers::Load(path, text);
    if (ret != 0)
    {
        return ret;
    }
    return FromPem(text, value);
}

int CGXPkcs8::Save(std::string& path)
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

int CGXPkcs8::ToPem(std::string& pem)
{
    pem.clear();
    if (m_PrivateKey.GetRawValue().GetSize() == 0)
    {
        return DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    if (!m_Description.empty())
    {
        pem = "#Description";
        pem += m_Description;
    }
    int ret;
    std::string der;
    if ((ret = ToDer(der)) == 0)
    {
        pem += "-----BEGIN PRIVATE KEY-----\n";
        pem += der;
        pem += "\n-----END PRIVATE KEY-----\n";
    }
    return ret;
}

int CGXPkcs8::ToDer(std::string& value)
{
    int ret;
    if (m_RawData.GetSize() != 0)
    {
        ret = m_RawData.ToBase64(value);
    }
    else
    {
        if ((ret = GetEncoded(m_RawData)) == 0)
        {
            ret = m_RawData.ToBase64(value);
        }
        if (ret != 0)
        {
            m_RawData.Clear();
        }
    }
    return ret;
}

bool CGXPkcs8::Equals(CGXPkcs8& cert)
{
    return m_RawData.Compare(
        cert.m_RawData.GetData(),
        cert.m_RawData.GetSize());
}
