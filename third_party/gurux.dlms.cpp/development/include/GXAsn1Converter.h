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
// See the GNU General Public License for more details->
//
// More information of Gurux products: http://www.gurux.org
//
// This code is licensed under the GNU General Public License v2.
// Full text may be retrieved at http://www.gnu.org/licenses/gpl-2.0.txt
//---------------------------------------------------------------------------

#ifndef GXASN1CONVERTER_H
#define GXASN1CONVERTER_H

#include <vector>
#include <map>
#include "GXDLMSVariant.h"
#include "GXHelpers.h"
#include "GXAsn1Base.h"
#include "GXAsn1BitString.h"
#include "GXDLMSConverter.h"
#include "GXAsn1Context.h"
#include "GXAsn1Integer.h"
#include "GXAsn1Ia5String.h"
#include "GXAsn1Utf8String.h"
#include "GXAsn1Sequence.h"
#include "GXAsn1ObjectIdentifier.h"
#include "GXAsn1Variant.h"
#include "GXAsn1Set.h"
#include "GXAsn1Helpers.h"
#include "GXAsn1Time.h"

class CGXAsn1Converter
{
private:
    friend class CGXPkcs8;
    static int GetValue(CGXByteBuffer& bb,
        std::vector<CGXAsn1Base*>* objects,
        bool getNext);

    static CGXDateTime GetUtcTime(
        std::string& dateString);

    static CGXDateTime GetGeneralizedTime(
        std::string& dateString);

    static std::string ToString(
        int value, int numbers);

    static std::string DateToString(
        CGXDateTime& date);

    /*
     Add ASN1 object to byte buffer.
     bb: Byte buffer where ANS1 object is serialized.
     target: ANS1 object
     Returns Size of the object.
    */
    static int GetBytes(CGXByteBuffer& bb,
        CGXAsn1Base* target,
        int& count);

    /**
     Get certificate type from byte array.
     */
    static DLMS_PKCS_TYPE GetCertificateType(
        CGXByteBuffer& data,
        CGXAsn1Sequence* seq);
public:
    /////////////////////////////////////////////////////////////////////////////
    // Constructor.
    /////////////////////////////////////////////////////////////////////////////
    CGXAsn1Converter();


    static int EncodeSubject(std::string& value,
        CGXAsn1Sequence* list);

    static int GetSubject(
        CGXAsn1Sequence* values,
        std::string& value);

    /**
     Convert byte array to ASN1 objects->
     data: ASN-1 bytes.
     Returns  Parsed objects->
    */
    static int FromByteArray(CGXByteBuffer& data,
        CGXAsn1Base*& value);

    /*Get next ASN1 value from the byte buffer.*/
    static int GetNext(CGXByteBuffer& data,
        CGXDLMSVariant& value);

    /**
     Convert ASN1 objects to byte array.
     @param objects ASN.1 objects->
     Returns  ASN.1 objects as byte array.
     */
    static int ToByteArray(CGXAsn1Base* objects,
        CGXByteBuffer& value);

    /**
     Convert system title to subject.

     @param systemTitle System title.
     Returns Subject.
     */
    static std::string SystemTitleToSubject(
        CGXByteBuffer& systemTitle);

    /**
     Get system title from the subject.

     @param subject Subject.
     Returns System title.
     */
    static int SystemTitleFromSubject(
        std::string& subject,
        CGXByteBuffer& value);

    /**
     Get system title in hex string from the subject.

     @param subject Subject.
     Returns System title.
     */
    static int HexSystemTitleFromSubject(
        std::string& subject,
        std::string& value);

    /**
     Convert ASN1 certificate type to DLMS key usage.

     @param type
     Returns
    */
    static DLMS_KEY_USAGE CertificateTypeToKeyUsage(
        DLMS_CERTIFICATE_TYPE type);

    /**
     Get certificate type from byte array.
     */
    static DLMS_PKCS_TYPE GetCertificateType(
        CGXByteBuffer& data);

    /**
     Get certificate type from DER string.

     @param der DER string
     Returns error code.
     */
    static DLMS_PKCS_TYPE GetCertificateType(
        std::string& der);
};

#endif //GXASN1CONVERTER_H
