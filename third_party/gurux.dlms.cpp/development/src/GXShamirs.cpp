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

#include "../include/GXShamirs.h"

int CGXShamirs::Trick(CGXCurve& curve,
    CGXPublicKey& pub,
    CGXEccPoint& ret,
    CGXBigInteger& u1,
    CGXBigInteger& u2)
{
    CGXEccPoint sum;
    CGXByteArray x = pub.GetX(), y = pub.GetY();
    CGXBigInteger x1(x);
    CGXBigInteger y1(y);
    CGXEccPoint op2(x1, y1);
    PointAdd(curve, sum, curve.m_G, op2);
    uint16_t bits1 = u1.GetUsedBits();
    uint16_t bits2 = u2.GetUsedBits();
    uint16_t pos = bits1 > bits2 ? bits1 : bits2;
    --pos;
    if (u1.IsBitSet(pos) && u2.IsBitSet(pos))
    {
        ret.X = sum.X;
        ret.Y = sum.Y;
    }
    else if (u1.IsBitSet(pos))
    {
        ret.X = curve.m_G.X;
        ret.Y = curve.m_G.Y;
    }
    else if (u2.IsBitSet(pos))
    {
        CGXByteArray tmp = pub.GetX();
        ret.X = CGXBigInteger(tmp);
        tmp = pub.GetY();
        ret.Y = CGXBigInteger(tmp);
    }
    CGXEccPoint tmp;
    --pos;
    while (true)
    {
        tmp.X = ret.X;
        tmp.Y = ret.Y;
        PointDouble(curve, ret, tmp);
        tmp.X = ret.X;
        tmp.Y = ret.Y;
        if (u1.IsBitSet(pos) && u2.IsBitSet(pos))
        {
            PointAdd(curve, ret, tmp, sum);
        }
        else if (u1.IsBitSet(pos))
        {
            PointAdd(curve, ret, tmp, curve.m_G);
        }
        else if (u2.IsBitSet(pos))
        {
            PointAdd(curve, ret, tmp, op2);
        }
        if (pos == 0)
        {
            break;
        }
        --pos;
    }
    return 0;
}

int CGXShamirs::PointAdd(CGXCurve& curve,
    CGXEccPoint& ret,
    CGXEccPoint& p1,
    CGXEccPoint& p2)
{
    CGXBigInteger negy(curve.m_P);
    negy.Sub(p2.Y);
    // Calculate lambda.
    CGXBigInteger ydiff(p2.Y);
    ydiff.Sub(p1.Y);
    CGXBigInteger xdiff(p2.X);
    xdiff.Sub(p1.X);
    xdiff.Inv(curve.m_P);
    CGXBigInteger lambda(ydiff);
    lambda.Multiply(xdiff);
    lambda.Mod(curve.m_P);
    // calculate resulting x coord.
    ret.X = CGXBigInteger(lambda);
    ret.X.Multiply(lambda);
    ret.X.Sub(p1.X);
    ret.X.Sub(p2.X);
    ret.X.Mod(curve.m_P);
    //calculate resulting y coord
    ret.Y = (p1.X);
    ret.Y.Sub(ret.X);
    ret.Y.Multiply(lambda);
    ret.Y.Sub(p1.Y);
    ret.Y.Mod(curve.m_P);
    return 0;
}

int CGXShamirs::PointDouble(CGXCurve& curve,
    CGXEccPoint& ret,
    CGXEccPoint& p1)
{
    CGXBigInteger numer(p1.X);
    numer.Multiply(p1.X);
    numer.Multiply(3);
    numer.Add(curve.m_A);
    CGXBigInteger denom(p1.Y);
    denom.Multiply(2);
    denom.Inv(curve.m_P);
    CGXBigInteger lambda(numer);
    lambda.Multiply(denom);
    lambda.Mod(curve.m_P);
    // calculate resulting x coord
    ret.X = CGXBigInteger(lambda);
    ret.X.Multiply(lambda);
    ret.X.Sub(p1.X);
    ret.X.Sub(p1.X);
    ret.X.Mod(curve.m_P);
    //calculate resulting y coord
    ret.Y = CGXBigInteger(p1.X);
    ret.Y.Sub(ret.X);
    ret.Y.Multiply(lambda);
    ret.Y.Sub(p1.Y);
    ret.Y.Mod(curve.m_P);
    return 0;
}

int CGXShamirs::PointMulti(CGXCurve& curve,
    CGXEccPoint& ret,
    CGXEccPoint& point,
    CGXBigInteger& scalar)
{
    CGXBigInteger x(point.X);
    CGXBigInteger y(point.Y);
    CGXEccPoint R0(x, y);
    CGXEccPoint R1;
    CGXEccPoint tmp;
    PointDouble(curve, R1, point);
    uint16_t dbits = scalar.GetUsedBits();
    dbits -= 2;
    while (true)
    {
        if (scalar.IsBitSet(dbits))
        {
            tmp.X = R0.X;
            tmp.Y = R0.Y;
            PointAdd(curve, R0, R1, tmp);
            tmp.X = R1.X;
            tmp.Y = R1.Y;
            PointDouble(curve, R1, tmp);
        }
        else
        {
            tmp.X = R1.X;
            tmp.Y = R1.Y;
            PointAdd(curve, R1, R0, tmp);
            tmp.X = R0.X;
            tmp.Y = R0.Y;
            PointDouble(curve, R0, tmp);
        }
        if (dbits == 0)
        {
            break;
        }
        --dbits;
    }
    ret.X = R0.X;
    ret.Y = R0.Y;
    return 0;
}