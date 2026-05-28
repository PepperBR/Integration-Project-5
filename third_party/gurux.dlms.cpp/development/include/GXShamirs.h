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

#ifndef GXSHAMIRS_H
#define GXSHAMIRS_H

#include "GXCurve.h"
#include "GXPublicKey.h"

/// <summary>
/// This class implements GXShamir's trick.
/// </summary>
class CGXShamirs
{
public:    
    /// <summary>
    /// Count Shamir's trick.
    /// </summary>
    /// <param name="curve">Used curve.</param>
    /// <param name="pub">Public key.</param>
    /// <param name="ret">Result.</param>
    /// <param name="u1"></param>
    /// <param name="u2"></param>
    static int Trick(CGXCurve& curve,
        CGXPublicKey& pub,
        CGXEccPoint& ret,
        CGXBigInteger& u1,
        CGXBigInteger& u2);

    /// <summary>
    /// Add points.
    /// </summary>
    /// <param name="curve">Used curve.</param>
    /// <param name="ret">Result.</param>
    /// <param name="p1">Point 1.</param>
    /// <param name="p2">Point 2.</param>
    static int PointAdd(CGXCurve& curve,
        CGXEccPoint& ret,
        CGXEccPoint& p1,
        CGXEccPoint& p2);

    /// <summary>
    /// Double point.
    /// </summary>
    /// <param name="curve">Used curve.</param>
    /// <param name="ret">Result value.</param>
    /// <param name="p1">Point to double</param>
    static int PointDouble(CGXCurve& curve,
        CGXEccPoint& ret,
        CGXEccPoint& p1);

    /// <summary>
       /// Multiply point with big integer value.
       /// </summary>
       /// <param name="curve">Used curve.</param>
       /// <param name="ret">Return value.</param>
       /// <param name="point">Point.</param>
       /// <param name="scalar">Scaler.</param>
    static int PointMulti(CGXCurve& curve,
        CGXEccPoint& ret,
        CGXEccPoint& point,
        CGXBigInteger& scalar);
};

#endif //GXSHAMIRS_H
