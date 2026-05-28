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

#ifndef GXCURVE_H
#define GXCURVE_H

#include "GXBigInteger.h"
#include "GXEccPoint.h"

/*ECC x and y points in the curve.*/
class CGXCurve
{
private:
    friend class CGXPrivateKey;
    friend class CGXEcdsa;
    friend class CGXShamirs;
    /**
    * ECC curve a value.
    */
    CGXBigInteger m_A;
    /**
    * ECC curve p value.
    */
    CGXBigInteger m_P;
    /**
    * ECC curve b parameter.
    */
    CGXBigInteger m_B;
    /**
     x and y-coordinate of base point G
    */
    CGXEccPoint m_G;
    /**
     Order of point G in ECC curve.
    */
    CGXBigInteger m_N;

    /**
 Init curve.

 @param a ECC curve a value.
 @param b ECC curve b parameter.
 @param p ECC curve p value.
 @param g x and y-coordinate of base point G
 @param n Order of point G in ECC curve.
*/
    int Init(ECC scheme)
    {
        if (scheme == ECC_P256)
        {
            //Table A. 1 – ECC_P256_Domain_Parameters
            const uint32_t INIT_A[] = {
                0xFFFFFFFC, 0xFFFFFFFF,
                0xFFFFFFFF, 0x00000000,
                0x00000000, 0x00000000,
                0x00000001, 0xFFFFFFFF,
            };
            const uint32_t INIT_G1[] = {
                0xD898C296, 0xF4A13945,
                0x2DEB33A0,0x77037D81,
                0x63A440F2,0xF8BCE6E5,
                0xE12C4247,0x6B17D1F2,
            };
            const uint32_t INIT_G2[] = {
                0x37BF51F5, 0xCBB64068,
                0x6B315ECE,0x2BCE3357,
                0x7C0F9E16,0x8EE7EB4A,
                0xFE1A7F9B,0x4FE342E2,
            };
            const uint32_t INIT_N[] = {
                0xFC632551,0xF3B9CAC2,
                0xA7179E84,0xBCE6FAAD,
                0xFFFFFFFF,0xFFFFFFFF,
                0x00000000,0xFFFFFFFF,
            };
            const uint32_t INIT_P[] = {
                0xFFFFFFFF,0xFFFFFFFF,
                0xFFFFFFFF,0x00000000,
                0x00000000,0x00000000,
                0x00000001,0xFFFFFFFF,
            };
            const uint32_t INIT_B[] = {
                0x27D2604B, 0x3BCE3C3E,
                0xCC53B0F6,0x651D06B0,
                0x769886BC,0xB3EBBD55,
                0xAA3A93E7,0x5AC635D8,
            };
            m_A.AddRange(INIT_A, sizeof(INIT_A) / sizeof(uint32_t));
            CGXBigInteger g1(INIT_G1, sizeof(INIT_G1) / sizeof(uint32_t));
            CGXBigInteger g2(INIT_G2, sizeof(INIT_G2) / sizeof(uint32_t));
            m_G = CGXEccPoint(g1, g2);
            m_N.AddRange(INIT_N, sizeof(INIT_N) / sizeof(uint32_t));
            m_P.AddRange(INIT_P, sizeof(INIT_P) / sizeof(uint32_t));
            m_B.AddRange(INIT_B, sizeof(INIT_B) / sizeof(uint32_t));
        }
        else if (scheme == ECC_P384)
        {
            //Table A. 2 – ECC_P384_Domain_Parameters
            const uint32_t INIT_A[] = {
                0xFFFFFFFC, 0x00000000,
                0x00000000, 0xFFFFFFFF,
                0xFFFFFFFE, 0xFFFFFFFF,
                0xFFFFFFFF, 0xFFFFFFFF,
                0xFFFFFFFF, 0xFFFFFFFF,
                0xFFFFFFFF, 0xFFFFFFFF,
            };
            const uint32_t INIT_G1[] = {
                0x72760AB7, 0x3A545E38,
                0xBF55296C, 0x5502F25D,
                0x82542A38, 0x59F741E0,
                0x8BA79B98, 0x6E1D3B62,
                0xF320AD74, 0x8EB1C71E,
                0xBE8B0537, 0xAA87CA22,
            };
            const uint32_t INIT_G2[] = {
                0x90EA0E5F, 0x7A431D7C,
                0x1D7E819D, 0x0A60B1CE,
                0xB5F0B8C0, 0xE9DA3113,
                0x289A147C, 0xF8F41DBD,
                0x9292DC29, 0x5D9E98BF,
                0x96262C6F, 0x3617DE4A,
            };
            const uint32_t INIT_N[] = {
                0xCCC52973, 0xECEC196A,
                0x48B0A77A, 0x581A0DB2,
                0xF4372DDF, 0xC7634D81,
                0xFFFFFFFF, 0xFFFFFFFF,
                0xFFFFFFFF, 0xFFFFFFFF,
                0xFFFFFFFF, 0xFFFFFFFF,
            };
            const uint32_t INIT_P[] = {
                0xFFFFFFFF, 0x00000000,
                0x00000000, 0xFFFFFFFF,
                0xFFFFFFFE, 0xFFFFFFFF,
                0xFFFFFFFF, 0xFFFFFFFF,
                0xFFFFFFFF, 0xFFFFFFFF,
                0xFFFFFFFF, 0xFFFFFFFF,
            };
            const uint32_t INIT_B[] = {
                0xD3EC2AEF, 0x2A85C8ED,
                0x8A2ED19D, 0xC656398D,
                0x5013875A, 0x0314088F,
                0xFE814112, 0x181D9C6E,
                0xE3F82D19, 0x988E056B,
                0xE23EE7E4, 0xB3312FA7,
            };
            CGXBigInteger g1(INIT_G1, sizeof(INIT_G1) / sizeof(uint32_t));
            CGXBigInteger g2(INIT_G2, sizeof(INIT_G2) / sizeof(uint32_t));
            m_A.AddRange(INIT_A, sizeof(INIT_A) / sizeof(uint32_t));
            m_G = CGXEccPoint(g1, g2);
            m_N.AddRange(INIT_N, sizeof(INIT_N) / sizeof(uint32_t));
            m_P.AddRange(INIT_P, sizeof(INIT_P) / sizeof(uint32_t));
            m_B.AddRange(INIT_B, sizeof(INIT_B) / sizeof(uint32_t));
        }
        else
        {
#ifdef _DEBUG
            printf("%s\n", "Invalid scheme.");
#endif //_DEBUG
            return DLMS_ERROR_CODE_INVALID_PARAMETER;
        }
        return 0;
    }
public:
    /// <summary>
    /// Constructor.
    /// </summary>
    CGXCurve()
    {

    }

    /**
    * ECC curve a value.
    */
    CGXBigInteger& GetA()
    {
        return m_A;
    }
    /**
    * ECC curve a value.
    */
    void SetA(CGXBigInteger& value)
    {
        m_A = value;
    }

    CGXBigInteger& GetP()
    {
        return m_P;
    }
    void SetP(CGXBigInteger value)
    {
        m_P = value;
    }
    CGXBigInteger GetB()
    {
        return m_B;
    }

    /**
     ECC curve b parameter.
    */
    void SetB(CGXBigInteger value)
    {
        m_B = value;
    }
    CGXEccPoint& GetG()
    {
        return m_G;
    }
    void SetG(CGXEccPoint& value)
    {
        m_G = value;
    }
    /**
     Order of point G in ECC curve.
    */
    CGXBigInteger& GetN()
    {
        return m_N;
    }
    /**
     Order of point G in ECC curve.
    */
    void SetN(CGXBigInteger& value)
    {
        m_N = value;
    }
};

#endif //GXCURVE_H
