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

#include "../include/GXPrivateKey.h"
#include "../include/GXCurve.h"
#include "../include/GXEcdsa.h"
#include "../include/GXShamirs.h"

CGXPrivateKey::CGXPrivateKey()
{
    m_Scheme = ECC_P256;
}

CGXPrivateKey& CGXPrivateKey::operator=(const CGXPrivateKey& value)
{
    m_Scheme = value.m_Scheme;
    m_RawValue = value.m_RawValue;
    m_PublicKey = value.m_PublicKey;
    return *this;
}

ECC CGXPrivateKey::GetScheme()
{
    return m_Scheme;
}

CGXByteArray& CGXPrivateKey::GetRawValue()
{
    return m_RawValue;
}

CGXByteArray& CGXPrivateKey::GetSystemTitle()
{
    return m_SystemTitle;
}

void CGXPrivateKey::SetSystemTitle(
    CGXByteBuffer& value)
{
    m_SystemTitle = value;
}

int CGXPrivateKey::FromRawBytes(
    CGXByteBuffer& value,
    CGXPrivateKey& key)
{
    //If private key is given
    if (value.GetSize() == 32)
    {
        key.m_Scheme = ECC_P256;
        key.m_RawValue = value;
    }
    else if (value.GetSize() == 48)
    {
        key.m_Scheme = ECC_P384;
        key.m_RawValue = value;
    }
    else
    {
        return DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    return 0;
}

int CGXPrivateKey::UpdateSchema(
    CGXAsn1Sequence* seq,
    CGXPrivateKey& key)
{
    int ret = 0;
    if (CGXAsn1Sequence* tmp = dynamic_cast<CGXAsn1Sequence*>(seq->GetValues()->at(2)))
    {
        DLMS_X9_OBJECT_IDENTIFIER id = CGXDLMSConverter::ValueOfX9Identifier(tmp->GetValues()->at(0)->ToString().c_str());
        switch (id)
        {
        case DLMS_X9_OBJECT_IDENTIFIER_PRIME_256_V1:
            key.m_Scheme = ECC_P256;
            break;
        case DLMS_X9_OBJECT_IDENTIFIER_SECP_384_R1:
            key.m_Scheme = ECC_P384;
            break;
        default:
#ifdef _DEBUG
            printf("Invalid private key.\n");
#endif //_DEBUG
            ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
        }
    }
    else
    {
        ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    return ret;
}

int CGXPrivateKey::UpdatePublicKey(
    CGXAsn1Sequence* seq,
    CGXPrivateKey& key)
{
    int ret = 0;
    if (CGXAsn1Variant* tmp = dynamic_cast<CGXAsn1Variant*>(seq->GetValues()->at(1)))
    {
        key.m_RawValue.Clear();
        key.m_RawValue.Set(tmp->GetValue().byteArr, tmp->GetValue().size);
        if (CGXAsn1Variant* tmp = dynamic_cast<CGXAsn1Variant*>(seq->GetValues()->at(3)))
        {
            CGXByteBuffer bb;
            bb.Set(tmp->GetValue().byteArr, tmp->GetValue().size);
            ret = CGXPublicKey::FromRawBytes(bb, key.m_PublicKey);
        }
        else if (CGXAsn1BitString* tmp = dynamic_cast<CGXAsn1BitString*>(seq->GetValues()->at(3)))
        {
            ret = CGXPublicKey::FromRawBytes(tmp->GetValue(), key.m_PublicKey);
        }
        else if (CGXAsn1Context* tmp = dynamic_cast<CGXAsn1Context*>(seq->GetValues()->at(3)))
        {
            if (CGXAsn1BitString* bs = dynamic_cast<CGXAsn1BitString*>(tmp->GetValues()->at(0)))
            {
                ret = CGXPublicKey::FromRawBytes(bs->GetValue(), key.m_PublicKey);
            }
        }
    }
    else
    {
        ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    return ret;
}

int CGXPrivateKey::FromDer(
    std::string der,
    CGXPrivateKey& key)
{
    GXHelpers::Replace(der, "\r\n", "");
    GXHelpers::Replace(der, "\n", "");
    CGXByteBuffer bb;
    int ret = bb.FromBase64(der);
    if (ret == 0)
    {
        CGXAsn1Base* value = NULL;
        ret = CGXAsn1Converter::FromByteArray(bb, value);
        if (ret == 0)
        {
            if (CGXAsn1Sequence* seq = dynamic_cast<CGXAsn1Sequence*>(value))
            {
                if (CGXAsn1Variant* var = dynamic_cast<CGXAsn1Variant*>(seq->GetValues()->at(0)))
                {
                    if (var->GetValue().cVal > 3)
                    {
#ifdef _DEBUG
                        printf("Invalid private key version.");
#endif //_DEBUG
                        ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
                    }
                    if ((ret = UpdateSchema(seq, key)) == 0)
                    {
                        ret = UpdatePublicKey(seq, key);
                    }
                }
                else
                {
                    printf("Invalid Certificate. This looks more like private key, not PKCS 8.");
                    ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
                }
            }
            else
            {
                ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
            }
        }
        delete value;
    }
    return ret;
}

int CGXPrivateKey::FromPem(std::string pem,
    CGXPrivateKey& value)
{
    GXHelpers::Replace(pem, "\r\n", "\n");
    std::string START = "PRIVATE KEY-----\n";
    std::string END = "-----END";
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
    return FromDer(pem, value);
}

#if defined(_WIN32) || defined(_WIN64) || defined(__linux__)

int CGXPrivateKey::Load(std::string& path,
    CGXPrivateKey& value)
{
    std::string text;
    int ret = GXHelpers::Load(path, text);
    if (ret != 0)
    {
        return ret;
    }
    return FromPem(text, value);
}

int CGXPrivateKey::Save(std::string& path)
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

int CGXPrivateKey::ToDer(std::string& value)
{
    CGXAsn1Sequence d;
    d.GetValues()->push_back(new CGXAsn1Variant((char)DLMS_CERTIFICATE_VERSION_2));
    d.GetValues()->push_back(new CGXAsn1Variant(m_RawValue));
    CGXAsn1Sequence* d1 = new CGXAsn1Sequence();
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
#ifdef _DEBUG
        printf("%s\n", "Invalid ECC scheme.");
#endif //_DEBUG
        return DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    d.GetValues()->push_back(d1);
    CGXAsn1Context* d2 = new CGXAsn1Context();
    d2->SetIndex(1);
    d2->GetValues()->push_back(new CGXAsn1BitString(m_PublicKey.GetRawValue(), 0));
    d.GetValues()->push_back(d2);
    CGXByteBuffer bb;
    int ret = CGXAsn1Converter::ToByteArray(&d, bb);
    if (ret == 0)
    {
        ret = bb.ToBase64(value);
    }
    return ret;
}

int CGXPrivateKey::ToPem(std::string& value)
{
    int ret;
    std::string der;
    value.clear();
    if ((ret = ToDer(der)) == 0)
    {
        value = "-----BEGIN EC PRIVATE KEY-----\n";
        value += der;
        value += "\n-----END EC PRIVATE KEY-----";
    }
    return ret;
}

int CGXPrivateKey::GetPublicKey(CGXPublicKey& value)
{
    if (m_PublicKey.GetRawValue().GetSize() == 0)
    {
        //Public key = private key multiple by curve.G.
        CGXBigInteger pk(m_RawValue);
        CGXCurve curve;
        int ret2 = curve.Init(m_Scheme);
        if (ret2 == 0)
        {
            CGXEccPoint ret;
            ret2 = CGXShamirs::PointMulti(curve, ret, curve.m_G, pk);
            if (ret2 == 0)
            {
                int size = m_Scheme == ECC_P256 ? 32 : 48;
                CGXByteBuffer bb;
                CGXByteBuffer tmp;
                //key is un-compressed format.
                bb.SetUInt8(4);
                ret.X.ToArray(tmp, false);
                bb.Set(&tmp, tmp.GetSize() % size, size);
                tmp.Clear();
                ret.Y.ToArray(tmp, false);
                bb.Set(&tmp, tmp.GetSize() % size, size);
                ret2 = CGXPublicKey::FromRawBytes(bb, m_PublicKey);
                if (ret2 != 0)
                {
                    return ret2;
                }
            }
        }     
    }
    value = m_PublicKey;
    return 0;
}

std::string CGXPrivateKey::ToHex()
{
    return ToHex(true);
}

std::string CGXPrivateKey::ToHex(bool addSpace)
{
    return m_RawValue.ToHexString(addSpace);
}

std::string CGXPrivateKey::ToString()
{
    return ToHex(true);
}