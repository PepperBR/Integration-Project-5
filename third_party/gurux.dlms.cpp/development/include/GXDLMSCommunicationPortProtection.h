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

#ifndef GXDLMSCommunicationPortProtection_H
#define GXDLMSCommunicationPortProtection_H

#include "GXIgnore.h"
#include "GXDLMSObject.h"
#ifndef DLMS_IGNORE_COMMUNICATION_PORT_PROTECTION

/**
Online help:
http://www.gurux.fi/Gurux.DLMS.Objects.GXDLMSCommunicationPortProtection
*/
class CGXDLMSCommunicationPortProtection : public CGXDLMSObject
{
    /**
     * Controls the protection mode.
     */
    DLMS_PROTECTION_MODE m_ProtectionMode;
    /**
     * Number of allowed failed communication attempts before port is disabled.
     */
    uint16_t m_AllowedFailedAttempts;
    /**
     * The lockout time.
     */
    uint32_t m_InitialLockoutTime;
    /**
     * Holds a factor that controls how the lockout time is increased with each
     * failed attempt.
     */
    unsigned char m_SteepnessFactor;
    /**
     * The lockout time.
     */
    uint32_t m_MaxLockoutTime;
    /**
     * The communication port being protected
     */
    CGXDLMSObject* m_Port;
    /**
     * Current protection status.
     */
    DLMS_PROTECTION_STATUS m_ProtectionStatus;
    /**
     * Failed attempts.
     */
    uint32_t m_FailedAttempts;
    /**
     * Total failed attempts.
     */
    uint32_t m_CumulativeFailedAttempts;
public:
    //Constructor.
    CGXDLMSCommunicationPortProtection();

    //SN Constructor.
    CGXDLMSCommunicationPortProtection(std::string ln, unsigned short sn);

    //LN Constructor.
    CGXDLMSCommunicationPortProtection(std::string ln);

    /**
    * Returns Controls the protection mode.
    */
    DLMS_PROTECTION_MODE GetProtectionMode();

    /**
     * value: Controls the protection mode.
     */
    void SetProtectionMode(DLMS_PROTECTION_MODE value);

    /**
     * Returns Number of allowed failed communication attempts before port is
     *         disabled.
     */
    uint16_t GetAllowedFailedAttempts();

    /**
     * value: Number of allowed failed communication attempts before port is disabled.
     */
    void SetAllowedFailedAttempts(uint16_t value);

    /**
     * Returns The lockout time.
     */
    uint32_t GetInitialLockoutTime();

    /**
     * value: The lockout time.
     */
    void SetInitialLockoutTime(uint32_t value);

    /**
     * Returns Holds a factor that controls how the lockout time is increased
     *         with each failed attempt.
     */
    unsigned char GetSteepnessFactor();

    /**
     * value: Holds a factor that controls how the lockout time is increased
     *            with each failed attempt.
     */
    void SetSteepnessFactor(unsigned char value);

    /**
     * Returns The lockout time.
     */
    uint32_t GetMaxLockoutTime();

    /**
     * value: The lockout time.
     */
    void SetMaxLockoutTime(uint32_t value);

    /**
     * Returns The communication port being protected
     */
    CGXDLMSObject* GetPort();

    /**
     * value: The communication port being protected
     */
    void SetPort(CGXDLMSObject* value);

    /**
     * Returns Current protection status.
     */
    DLMS_PROTECTION_STATUS GetProtectionStatus();

    /**
     * value: Current protection status.
     */
    void SetProtectionStatus(DLMS_PROTECTION_STATUS value);

    /**
     * Returns Failed attempts.
     */
    uint32_t GetFailedAttempts();

    /**
     * value: Failed attempts.
     */
    void SetFailedAttempts(uint32_t value);

    /**
     * Returns Total failed attempts.
     */
    uint32_t GetCumulativeFailedAttempts();

    /**
     * value: Total failed attempts.
     */
    void SetCumulativeFailedAttempts(uint32_t value);

    /*
     * Resets FailedAttempts and current lockout time to zero. Protection status
     * is set to unlocked.
     *
     * client: DLMS client.
     * Returns Action bytes.
     */
    int Reset(
        CGXDLMSClient* client,
        std::vector<CGXByteBuffer>& reply);

    // Returns amount of attributes.
    int GetAttributeCount();

    // Returns amount of methods.
    int GetMethodCount();

    //Get attribute values of object.
    void GetValues(std::vector<std::string>& values);

    /////////////////////////////////////////////////////////////////////////
    // Returns collection of attributes to read.
    //
    // If attribute is static and already read or device is returned
    // HW error it is not returned.
    //
    // all: All items are returned even if they are read already.
    // attributes: Collection of attributes to read.
    void GetAttributeIndexToRead(bool all, std::vector<int>& attributes);

    int GetDataType(int index, DLMS_DATA_TYPE& type);

    // Returns value of given attribute.
    int GetValue(CGXDLMSSettings& settings, CGXDLMSValueEventArg& e);

    // Set value of given attribute.
    int SetValue(CGXDLMSSettings& settings, CGXDLMSValueEventArg& e);
};
#endif //DLMS_IGNORE_COMMUNICATION_PORT_PROTECTION
#endif //GXDLMSCommunicationPortProtection_H