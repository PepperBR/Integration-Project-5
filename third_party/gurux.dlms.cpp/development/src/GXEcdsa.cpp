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

#include "../include/GXEcdsa.h"
#include "../include/GXDLMSSha256.h"
#include "../include/GXDLMSSha384.h"
#include "../include/GXShamirs.h"

int CGXEcdsa::GetRandomNumber(CGXBigInteger& N,
    CGXByteBuffer& value)
{
    value.SetSize(0);
    value.Capacity(4 * N.GetCount());
    unsigned char val;
    srand((unsigned int)time(NULL));
    for (uint16_t pos = 0; pos != (uint16_t)value.Capacity(); ++pos)
    {
        val = rand();
        if (val == 0 && pos == 0)
        {
            val = (1 + rand()) % 100;
        }
        value.SetUInt8(val);
    }
    return 0;
}

CGXEcdsa::CGXEcdsa(ECC scheme)
{
    m_Curve.Init(scheme);
}

int CGXEcdsa::GetSchemeSize(ECC scheme)
{
    return scheme == ECC_P256 ? 32 : 48;
}

CGXEcdsa::CGXEcdsa(CGXPublicKey& key) :
    CGXEcdsa(key.GetScheme())
{
    m_PublicKey = key;
}

CGXEcdsa::CGXEcdsa(CGXPrivateKey& key) :
    CGXEcdsa(key.GetScheme())
{
    m_PrivateKey = key;
}

int CGXEcdsa::Sign(CGXByteBuffer& data,
    CGXByteBuffer& signature)
{
    signature.Clear();
    int ret;
    unsigned long pos = data.GetPosition();
    if (m_PrivateKey.GetRawValue().GetSize() == 0)
    {
        printf("Invalid private key.");
        return DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    if (m_PrivateKey.GetScheme() == ECC_P256)
    {
        if ((ret = CGXDLMSSha256::Hash(data, signature)) != 0)
        {
            return ret;
        }
    }
    else
    {
        if ((ret = CGXDLMSSha384::Hash(data, signature)) != 0)
        {
            return ret;
        }
    }
    data.SetPosition(pos);
    CGXBigInteger msg(signature);
    signature.Clear();
    CGXByteBuffer tmp;
    if ((ret = GetRandomNumber(m_Curve.m_N, tmp)) == 0)
    {
        CGXBigInteger k(tmp);
        CGXBigInteger pk(m_PrivateKey.m_RawValue.m_Data,
            (uint16_t)m_PrivateKey.m_RawValue.m_Size);
        CGXEccPoint R;
        CGXShamirs::PointMulti(m_Curve, R, m_Curve.m_G, k);
        CGXBigInteger r(R.X);
        r.Mod(m_Curve.m_N);
        //s = (k ^ -1 * (e + d * r)) mod n
        CGXBigInteger s(pk);
        s.Multiply(r);
        s.Add(msg);
        CGXBigInteger kinv(k);
        kinv.Inv(m_Curve.m_N);
        s.Multiply(kinv);
        s.Mod(m_Curve.m_N);
        tmp.Clear();
        if (ret == 0 &&
            (ret = r.ToArray(signature, false)) == 0 &&
            (ret = s.ToArray(tmp, false)) == 0)
        {
            ret = signature.Set(tmp.GetData(), tmp.GetSize());
        }
    }
    return ret;
}

int CGXEcdsa::GenerateSecret(
    CGXPublicKey& publicKey,
    CGXByteBuffer& secret)
{
    int ret;
    if (m_PrivateKey.GetRawValue().GetSize() == 0)
    {
        printf("Invalid private key.");
        ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    else if (m_PrivateKey.GetScheme() != publicKey.GetScheme())
    {
        printf("Private key scheme is different than key.");
        ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    else
    {
        CGXBigInteger pk(m_PrivateKey.m_RawValue.m_Data,
            (uint16_t)m_PrivateKey.m_RawValue.m_Size);
        CGXByteBuffer bb;
        CGXByteBuffer raw;
        publicKey.m_RawValue.ToByteBuffer(raw);
        int size = GetSchemeSize(m_PrivateKey.GetScheme());
        raw.SubArray(1, size, bb);
        CGXBigInteger x(bb);
        bb.SetSize(0);
        raw.SubArray(1 + size, size, bb);
        CGXBigInteger y(bb);
        CGXCurve curve;
        curve.Init(m_PrivateKey.GetScheme());
        CGXEccPoint p(x, y);
        CGXEccPoint ret2;
        CGXShamirs::PointMulti(curve, ret2, p, pk);
        //        p = JacobianMultiply(p, pk, m_Curve.m_N, m_Curve.m_A, m_Curve.m_P);
          //      FromJacobian(p, m_Curve.m_P);
        ret = ret2.X.ToArray(secret);
    }
    return ret;
}

int CGXEcdsa::GenerateKeyPair(
    ECC scheme,
    std::pair<CGXPublicKey, CGXPrivateKey>& kp)
{
    CGXPrivateKey pk;
    CGXPublicKey pub;
    CGXByteBuffer rn;
    CGXCurve curve;
    int ret;
    if ((ret = curve.Init(scheme)) == 0 &&
        (ret = GetRandomNumber(curve.m_N, rn)) == 0 &&
        (ret = CGXPrivateKey::FromRawBytes(rn, pk)) == 0 &&
        (ret = pk.GetPublicKey(pub)) == 0)
    {
        kp.first = pub;
        kp.second = pk;
    }
    return ret;
}

int CGXEcdsa::Verify(CGXByteBuffer& signature,
    CGXByteBuffer& data,
    bool& value)
{
    value = false;
    int ret;
    CGXByteBuffer bb;
    if (m_PublicKey.GetRawValue().GetSize() == 0)
    {
        if (m_PrivateKey.GetRawValue().GetSize() == 0)
        {
            printf("Invalid private key.");
            ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
        }
        else 
        {
            ret = m_PrivateKey.GetPublicKey(m_PublicKey);
        }
    }
    if (m_PublicKey.GetScheme() == ECC_P256)
    {
        ret = CGXDLMSSha256::Hash(data, bb);
    }
    else if (m_PublicKey.GetScheme() == ECC_P384)
    {
        ret = CGXDLMSSha384::Hash(data, bb);
    }
    else
    {
        ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    if (ret == 0)
    {
        CGXBigInteger u1(bb);
        int size = GetSchemeSize(m_PublicKey.m_Scheme);
        signature.SubArray(0, size, bb);
        CGXBigInteger sigR(bb);
        signature.SubArray(size, size, bb);
        CGXBigInteger sigS(bb);
        CGXBigInteger w = sigS;
        w.Inv(m_Curve.m_N);
        u1.Multiply(w);
        u1.Mod(m_Curve.m_N);
        CGXBigInteger u2(sigR);
        u2.Multiply(w);
        u2.Mod(m_Curve.m_N);
        CGXEccPoint tmp;
        CGXShamirs::Trick(m_Curve, m_PublicKey, tmp, u1, u2);
        tmp.X.Mod(m_Curve.m_N);
        value = tmp.X.Compare(sigR) == 0;
    }
    return ret;
}

int CGXEcdsa::Validate(CGXPublicKey& publicKey)
{
    int ret;
    if (publicKey.GetRawValue().GetSize() == 0)
    {
        printf("Invalid key.");
        ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    else
    {
        CGXByteBuffer bb;
        int size = GetSchemeSize(publicKey.GetScheme());
        publicKey.GetRawValue().SubArray(1, size, bb);
        CGXBigInteger x(bb);
        bb.SetSize(0);
        publicKey.GetRawValue().SubArray(1 + size, size, bb);
        CGXBigInteger y(bb);
        CGXCurve curve;
        ret = curve.Init(publicKey.GetScheme());
        y.Multiply(y);
        y.Mod(curve.m_P);
        CGXBigInteger tmpX(x);
        tmpX.Multiply(x);
        tmpX.Mod(curve.m_P);
        tmpX.Add(curve.m_A);
        tmpX.Multiply(x);
        tmpX.Add(curve.m_B);
        tmpX.Mod(curve.m_P);
        if (y.Compare(tmpX) != 0)
        {
            printf("Public key validate failed. Public key is not valid ECDSA key.");
            ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
        }
    }
    return ret;
}
