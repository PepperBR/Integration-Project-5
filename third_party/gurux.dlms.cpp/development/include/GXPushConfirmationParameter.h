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

#ifndef GXPUSHCONFIRMATIONPARAMETER_H
#define GXPUSHCONFIRMATIONPARAMETER_H

#include "GXIgnore.h"
#ifndef DLMS_IGNORE_PUSH_SETUP
#include "GXDateTime.h"

/**
    Push confirmation parameters.
*/
class CGXPushConfirmationParameter
{
private:
    /**
    * Confirmation start date. Fields of date-time not specified are not used.
    */
    CGXDateTime m_StartDate;
    /**
     * Confirmation time interval in seconds. Disabled, if zero.
     */
    uint32_t m_Interval;

public:
    /**
     * Returns Confirmation start date. Fields of date-time not specified are
     *         not used.
     */
    CGXDateTime& GetStartDate()
    {
        return m_StartDate;
    }

    /**
     * value: Confirmation start date.
     Fields of date-time not specified are not used.
     */
    void SetStartDate(CGXDateTime& value)
    {
        m_StartDate = value;
    }

    /**
     * Returns Confirmation time interval in seconds. Disabled, if zero.
     */
    uint32_t GetInterval()
    {
        return m_Interval;
    }

    /**
     * value: Confirmation time interval in seconds. Disabled, if zero.
     */
    void SetInterval(uint32_t value)
    {
        m_Interval = value;
    }

    std::string ToString()
    {
        std::string str;
        str += m_StartDate.ToString();
        str += ", ";
        str += std::to_string(m_Interval);
        return str;
    }
};
#endif //DLMS_IGNORE_PUSH_SETUP
#endif //GXPUSHCONFIRMATIONPARAMETER_H