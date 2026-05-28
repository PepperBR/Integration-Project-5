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

#ifndef GXBIGINTEGER_H
#define GXBIGINTEGER_H

#include "GXBytebuffer.h"
#include "GXByteArray.h"

class CGXBigInteger
{
private:
    friend class CGXPrivateKey;
    /**
     List of values. Least Significated is in the first item.
    */
    uint32_t* m_Data;

    /**
     Items count in the data buffer.
    */
    uint16_t m_Count;

    /**
     Items count in the data buffer.
    */
    uint16_t m_Capacity;

    /**
     Is value IsNegative.

     @return True, if value is IsNegative.
    */
    bool m_IsNegative;

    /**
    Get values from byte buffer.

    @param value
   */
    int FromByteBuffer(CGXByteBuffer& value);
    int FromByteBuffer(CGXByteArray& value);

    void SetIsNegative(bool value);

    int Add(const uint32_t value);

    int AddValue(uint32_t* list,
        uint16_t length,
        uint32_t value,
        uint16_t index);

    int Capacity(uint16_t value);
public:
    /**
    * Constructor.
    */
    CGXBigInteger();

    /**
     Constuctor.

     @param value Big UInt32eger value in MSB.
    */
    CGXBigInteger(std::string& value);

    CGXBigInteger(uint64_t value);

    CGXBigInteger(uint32_t value);

    CGXBigInteger(int value);

    /**
     Constructor value.
    */
    CGXBigInteger(const uint32_t* values, uint16_t count);

    /**
     Constructor value.

     @param value Byte array Data in MSB format.
    */
    CGXBigInteger(const unsigned char* values, uint16_t count);

    /**
     Constructor value.

     @param value Byte array Data in MSB format.
    */
    CGXBigInteger(CGXByteBuffer& value);

    /**
     Constructor value.

     @param value Byte array Data in MSB format.
    */
    CGXBigInteger(CGXByteArray& value);
    
    CGXBigInteger(CGXBigInteger* value);

    CGXBigInteger(const CGXBigInteger& value);

    /**
    * Destructor.
    */
    ~CGXBigInteger();

    /**
     Items count in the data buffer.
    */
    uint16_t GetCount();

    /**
     Items count in the data buffer.
    */
    void SetCount(uint16_t value);

    /**
    Is value IsNegative.
    */
    bool IsNegative();

    /**
     Is value zero.

     @return True, if value is zero.
    */
    bool IsZero();

    /**
     Is value even.

     @return True, if value is even.
    */
    bool IsEven();

    /**
     Is value One.

     @return True, if value is one.
    */
    bool IsOne();

    /**
     Convert value to byte array.
    */
    int ToArray(CGXByteBuffer& data,
        bool removeLeadingZeroes);

    /**
     Convert value to byte array.
    */
    int ToArray(CGXByteBuffer& data);

    /*
     Convert value to byte array.
    */
    int ToArray(uint32_t start,
        uint32_t size,
        CGXByteBuffer& data);

    int AddRange(const uint32_t* values, uint16_t count);

    void Or(CGXBigInteger& value);
    int Add(CGXBigInteger& value);
    void Sub(CGXBigInteger& value);

    void Multiply(int value);
    void Multiply(CGXBigInteger& value);

    int Compare(CGXBigInteger& value);

    /**
     Compare value to UInt32 integer value.

     @param value Returns 1 is compared value is bigger, -1 if smaller and 0 if values are equals.
     @return
    */
    int Compare(uint32_t value);

    void Lshift(uint32_t amount);

    void Rshift(uint32_t amount);

    /**
     Reset value to Zero.
    */
    void Clear();

    void Pow(uint32_t exponent);

    void Div(CGXBigInteger* value);

    /**
     Modulus.
    */
    void Mod(CGXBigInteger& mod);

    /**
     Invert value.
    */
    void Inv(CGXBigInteger& value);

    std::string ToString();

    CGXBigInteger& operator=(
        const CGXBigInteger& value);

    /// <summary>
    /// Used bits.
    /// </summary>
    uint16_t GetUsedBits();

    /// <summary>
    /// This method checks if the bit is set.
    /// </summary>
    /// <param name="index">Bit index.</param>
    /// <returns></returns>
    bool IsBitSet(uint16_t index);
};
#endif //GXBIGINTEGER_H
