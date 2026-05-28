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

#include "../include/GXPublicKey.h"

CGXPublicKey::CGXPublicKey()
{
    m_Scheme = ECC_P256;
}

CGXPublicKey& CGXPublicKey::operator=(const CGXPublicKey& value)
{
    m_Scheme = value.m_Scheme;
    m_RawValue = value.m_RawValue;
    m_SystemTitle = value.m_SystemTitle;
    return *this;
}

ECC CGXPublicKey::GetScheme()
{
    return m_Scheme;
}

CGXByteArray& CGXPublicKey::GetRawValue()
{
    return m_RawValue;
}

CGXByteArray& CGXPublicKey::GetSystemTitle()
{
    return m_SystemTitle;
}

void CGXPublicKey::SetSystemTitle(CGXByteBuffer& value)
{
    m_SystemTitle = value;
}

int CGXPublicKey::FromRawBytes(CGXByteBuffer& key,
    CGXPublicKey& value)
{
    if (key.GetSize() == 65)
    {
        value.m_Scheme = ECC_P256;
        value.m_RawValue = key;
    }
    else if (key.GetSize() == 97)
    {
        value.m_Scheme = ECC_P384;
        value.m_RawValue = key;
    }
    else if (key.GetSize() == 64)
    {
        //Compression tag is not send in DLMS messages.
        value.m_Scheme = ECC_P256;
        value.m_RawValue.SetUInt8(4);
        value.m_RawValue.Set(&key);
    }
    else if (key.GetSize() == 96)
    {
        //Compression tag is not send in DLMS messages.
        value.m_Scheme = ECC_P384;
        value.m_RawValue.SetUInt8(4);
        value.m_RawValue.Set(&key);
    }
    else
    {
        //Invalid key.
        return DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    return 0;
}

int CGXPublicKey::FromDer(std::string der,
    CGXPublicKey& key)
{
    GXHelpers::Replace(der, "\r\n", "");
    GXHelpers::Replace(der, "\n", "");
    CGXByteBuffer bb;
    bb.FromBase64(der);
    CGXAsn1Base* value = NULL;
    int ret = CGXAsn1Converter::FromByteArray(bb, value);
    if (ret == 0)
    {
        if (CGXAsn1Sequence* seq = dynamic_cast<CGXAsn1Sequence*>(value))
        {
            if (CGXAsn1Sequence* tmp = dynamic_cast<CGXAsn1Sequence*>(seq->GetValues()->at(0)))
            {
                if (CGXAsn1ObjectIdentifier* id = dynamic_cast<CGXAsn1ObjectIdentifier*>(tmp->GetValues()->at(1)))
                {
                    switch (CGXDLMSConverter::ValueOfX9Identifier(id->GetObjectIdentifier().c_str()))
                    {
                    case DLMS_X9_OBJECT_IDENTIFIER_PRIME_256_V1:
                        key.m_Scheme = ECC_P256;
                        break;
                    case DLMS_X9_OBJECT_IDENTIFIER_SECP_384_R1:
                        key.m_Scheme = ECC_P384;
                        break;
                    default:
                        ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
                    }
                    if (ret == 0)
                    {
                        if (CGXAsn1BitString* bs = dynamic_cast<CGXAsn1BitString*>(seq->GetValues()->at(1)))
                        {
                            //Open SSL PEM.
                            key.m_RawValue = bs->GetValue();
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
#ifdef _DEBUG
        if (ret != 0)
        {
            printf("Invalid public key.");
        }
#endif //_DEBUG
        delete value;
    }
    return ret;
}

int CGXPublicKey::FromPem(
    std::string pem,
    CGXPublicKey& key)
{
    GXHelpers::Replace(pem, "\r\n", "\n");
    std::string START = "-----BEGIN PUBLIC KEY-----\n";
    std::string END = "\n-----END PUBLIC KEY-----";
    size_t index = pem.find(START);
    if (index == std::string::npos)
    {
        return DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    pem = pem.substr(index + START.length());
    index = pem.rfind(END);
    if (index == std::string::npos)
    {
        return DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    pem = pem.substr(0, index);
    return FromDer(pem, key);
}

#if defined(_WIN32) || defined(_WIN64) || defined(__linux__)

int CGXPublicKey::Load(std::string& path,
    CGXPublicKey& value)
{
    std::string text;
    int ret = GXHelpers::Load(path, text);
    if (ret != 0)
    {
        return ret;
    }
    return FromPem(text, value);
}

int CGXPublicKey::Save(std::string& path)
{
    std::string text;
    int ret = ToPem(text);
    if (ret != 0)
    {
        return ret;
    }
    return GXHelpers::Save(path, text);
    return 0;
}
#endif //defined(_WIN32) || defined(_WIN64) || defined(__linux__)

std::string CGXPublicKey::ToHex()
{
    return m_RawValue.ToHexString();
}

int CGXPublicKey::ToDer(std::string& value)
{
    int ret;
    CGXByteBuffer bb;
    if ((ret = GetEncoded(bb)) == 0)
    {
        bb.ToBase64(value);
    }
    return ret;
}

int CGXPublicKey::GetEncoded(
    CGXByteBuffer& value)
{
    //Subject Key Info.
    CGXAsn1Sequence d;
    CGXAsn1Sequence* d1 = new CGXAsn1Sequence();
    d1->GetValues()->push_back(new CGXAsn1ObjectIdentifier("1.2.840.10045.2.1"));
    if (m_Scheme == ECC_P256)
    {
        d1->GetValues()->push_back(new CGXAsn1ObjectIdentifier("1.2.840.10045.3.1.7"));
    }
    else if (m_Scheme == ECC_P384)
    {
        d1->GetValues()->push_back(new CGXAsn1ObjectIdentifier("1.3.132.0.34"));
    }
    else
    {
        return DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    d.GetValues()->push_back(d1);
    d.GetValues()->push_back(new CGXAsn1BitString(m_RawValue, 0));
    return CGXAsn1Converter::ToByteArray(&d, value);
}

int CGXPublicKey::ToPem(std::string& value)
{
    value.clear();
    std::string der;
    int ret = ToDer(der);
    if (ret == 0)
    {
        value = "-----BEGIN PUBLIC KEY-----\n";
        value += der;
        value += "\n-----END PUBLIC KEY-----\n";
    }
    return ret;
}

CGXByteArray CGXPublicKey::GetX()
{
    CGXByteArray bb;
    int size = m_RawValue.GetSize() / 2;
    m_RawValue.SubArray(1, size, bb);
    return bb;
}

CGXByteArray CGXPublicKey::GetY()
{
    CGXByteArray bb;
    int size = m_RawValue.GetSize() / 2;
    m_RawValue.SubArray(1 + size, size, bb);
    return bb;
}

std::string CGXPublicKey::ToString()
{
    std::string sb;
    if (m_Scheme == ECC_P256)
    {
        sb += "NIST P-256\n";
    }
    else if (m_Scheme == ECC_P384)
    {
        sb += "NIST P-384\n";
    }
    CGXByteBuffer pk;
    int size = m_RawValue.GetSize() / 2;
    sb += " x coord: ";
    m_RawValue.SubArray(1, size, pk);
    sb += CGXBigInteger(pk).ToString();
    sb += " y coord: ";
    pk.SetSize(0);
    m_RawValue.SubArray(1 + size, size, pk);
    sb += CGXBigInteger(pk).ToString();
    return sb;
}