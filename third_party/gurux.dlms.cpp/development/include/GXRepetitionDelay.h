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

#ifndef GXREPETITIONDELAY_H
#define GXREPETITIONDELAY_H

#include "GXIgnore.h"
#ifndef DLMS_IGNORE_PUSH_SETUP
#include "GXDLMSObject.h"
#include "GXDLMSPushObject.h"
#include "GXDLMSCaptureObject.h"

/**
This class is used to count repetition delay for the next push message.
*/
class CGXRepetitionDelay
{
private:
    /**
    * The minimum delay until a next push attempt is started in seconds.
    */
    uint16_t m_Min;

    /**
     * Calculating the next delay.
     */
    uint16_t m_Exponent;

    /**
     * The maximum delay until a next push attempt is started in seconds.
     */
    uint16_t m_Max;

public:
    /**
     * Returns The minimum delay until a next push attempt is started in
     *         seconds.
     */
    uint16_t GetMin()
    {
        return m_Min;
    }

    /**
     * value:
     *            The minimum delay until a next push attempt is started in
     *            seconds.
     */
    void SetMin(uint16_t value)
    {
        m_Min = value;
    }

    /**
     * Returns Calculating the next delay.
     */
    uint16_t GetExponent()
    {
        return m_Exponent;
    }

    /**
     * value:
     *            Calculating the next delay.
     */
    void SetExponent(uint16_t value)
    {
        m_Exponent = value;
    }

    /**
     * Returns The maximum delay until a next push attempt is started in
     *         seconds.
     */
    int GetMax()
    {
        return m_Max;
    }

    /**
     * value:
     *            The maximum delay until a next push attempt is started in
     *            seconds.
     */
    void SetMax(uint16_t value)
    {
        m_Max = value;
    }
};
#endif //DLMS_IGNORE_PUSH_SETUP
#endif //GXREPETITIONDELAY_H