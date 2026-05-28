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

#include "../include/GXAsn1Converter.h"
#include "../include/GXPkcs8.h"
#include "../include/GXPkcs10.h"
#include "../include/GXx509Certificate.h"

CGXAsn1Converter::CGXAsn1Converter()
{
}

int CGXAsn1Converter::GetValue(
    CGXByteBuffer& bb,
    std::vector<CGXAsn1Base*>* objects,
    bool getNext)
{
    int ret;
    unsigned char type;
    unsigned long len;
    CGXByteBuffer tmp2;
    if ((ret = bb.GetUInt8(&type)) != 0)
    {
        return ret;
    }
    GXHelpers::GetObjectCount(bb, len);
    if (len > bb.Available())
    {
        return DLMS_ERROR_CODE_OUTOFMEMORY;
    }
    int start = bb.GetPosition();
    switch (type)
    {
    case BER_TYPE_CONSTRUCTED | BER_TYPE_CONTEXT:
    case BER_TYPE_CONSTRUCTED | BER_TYPE_CONTEXT | 1:
    case BER_TYPE_CONSTRUCTED | BER_TYPE_CONTEXT | 2:
    case BER_TYPE_CONSTRUCTED | BER_TYPE_CONTEXT | 3:
    case BER_TYPE_CONSTRUCTED | BER_TYPE_CONTEXT | 4:
    case BER_TYPE_CONSTRUCTED | BER_TYPE_CONTEXT | 5:
    {
        CGXAsn1Context* it = new CGXAsn1Context();
        it->SetIndex(type & 0xF);
        objects->push_back(it);
        while (bb.GetPosition() < start + len)
        {
            if ((ret = GetValue(bb, it->GetValues(), false)) != 0)
            {
                return ret;
            }
        }
    }
    break;
    case BER_TYPE_CONSTRUCTED | BER_TYPE_SEQUENCE:
    {
        CGXAsn1Sequence* it = new CGXAsn1Sequence();
        objects->push_back(it);
        int cnt = 0;
        while (bb.GetPosition() < start + len)
        {
            ++cnt;
            if ((ret = GetValue(bb, it->GetValues(), false)) != 0)
            {
                return ret;
            }
            if (getNext)
            {
                break;
            }
        }
    }
    break;
    case BER_TYPE_CONSTRUCTED | BER_TYPE_SET:
    {
        std::vector<CGXAsn1Base*> tmp;
        if ((ret = GetValue(bb, &tmp, false)) != 0)
        {
            return ret;
        }
        CGXAsn1Base* t = tmp.at(0);
        if (CGXAsn1Sequence* a = dynamic_cast<CGXAsn1Sequence*>(t))
        {
            objects->push_back(new CGXAsn1Set(CGXAsn1Helpers::Clone(a->GetValues()->at(0)),
                CGXAsn1Helpers::Clone(a->GetValues()->at(1))));
        }
        else
        {
            objects->push_back(new CGXAsn1Set(CGXAsn1Helpers::Clone(t), NULL));
        }
        for (std::vector<CGXAsn1Base*>::iterator it = tmp.begin(); it != tmp.end(); ++it)
        {
            delete (*it);
        }
    }
    break;
    case BER_TYPE_OBJECT_IDENTIFIER:
    {
        CGXAsn1ObjectIdentifier* oi = new CGXAsn1ObjectIdentifier(bb, len);
        objects->push_back(oi);
    }
    break;
    case BER_TYPE_PRINTABLE_STRING:
    {
        std::string str;
        bb.GetString(len, str);
        objects->push_back(new CGXAsn1Variant(str));
    }
    break;
    case BER_TYPE_BMP_STRING:
    {
        std::wstring str;
        bb.GetStringUnicode(0, len, str);
        objects->push_back(new CGXAsn1Variant(str));
    }
    break;
    case BER_TYPE_UTF8_STRING:
    {
        std::string str;
        bb.GetString(len, str);
        objects->push_back(new CGXAsn1Utf8String(str));
    }
    break;
    case BER_TYPE_IA5_STRING:
    {
        std::string str;
        bb.GetString(len, str);
        objects->push_back(new CGXAsn1Ia5String(str));
    }
    break;
    case BER_TYPE_INTEGER:
    {
        CGXAsn1Variant* var = NULL;
        if (len == 1)
        {
            char value;
            bb.GetInt8(&value);
            var = new CGXAsn1Variant(value);
            objects->push_back(var);
        }
        else if (len == 2)
        {
            short value;
            bb.GetInt16(&value);
            var = new CGXAsn1Variant(value);
            objects->push_back(var);
        }
        else if (len == 4)
        {
            long value;
            bb.GetInt32(&value);
            var = new CGXAsn1Variant(value);
            objects->push_back(var);
        }
        else
        {
            if ((ret = tmp2.Set(&bb, bb.GetPosition(), len)) != 0)
            {
                return ret;
            }
            objects->push_back(new CGXAsn1Variant(tmp2));
        }
    }
    break;
    case BER_TYPE_NULL:
        objects->push_back(NULL);
        break;
    case BER_TYPE_BIT_STRING:
    {
        CGXByteBuffer tmp;
        tmp.Set(&bb, bb.GetPosition(), len);
        CGXAsn1BitString* tmp3 = new CGXAsn1BitString(tmp);
        objects->push_back(tmp3);
    }
    break;
    case BER_TYPE_UTC_TIME:
    {
        std::string str;
        bb.SubArray(bb.GetPosition(), len, tmp2);
        tmp2.GetString(len, str);
        CGXDateTime dt = GetUtcTime(str);
        objects->push_back(new CGXAsn1Time(dt));
    }
    break;
    case BER_TYPE_GENERALIZED_TIME:
    {
        std::string str;
        bb.SubArray(bb.GetPosition(), len, tmp2);
        tmp2.GetString(len, str);
        CGXDateTime dt = GetGeneralizedTime(str);
        objects->push_back(new CGXAsn1Time(dt));
    }
    break;
    case BER_TYPE_CONTEXT:
    case BER_TYPE_CONTEXT | 1:
    case BER_TYPE_CONTEXT | 2:
    case BER_TYPE_CONTEXT | 3:
    case BER_TYPE_CONTEXT | 4:
    case BER_TYPE_CONTEXT | 5:
    case BER_TYPE_CONTEXT | 6:
    {
        bb.SubArray(bb.GetPosition(), len, tmp2);
        CGXAsn1Context* var = new CGXAsn1Context();
        var->SetConstructed(false);
        var->SetIndex(type & 0xF);
        var->GetValues()->push_back(new CGXAsn1Variant(tmp2));
        objects->push_back(var);
    }
    break;
    case BER_TYPE_OCTET_STRING:
    {
        unsigned char ch;
        bb.GetUInt8(bb.GetPosition(), &ch);
        switch (ch)
        {
        case BER_TYPE_CONSTRUCTED | BER_TYPE_SEQUENCE:
        case BER_TYPE_BIT_STRING:
            if ((ret = GetValue(bb, objects, false)) != 0)
            {
                return ret;
            }
            break;
        default:
        {
            tmp2.Set(&bb, bb.GetPosition(), len);
            CGXAsn1Variant* var = new CGXAsn1Variant(tmp2);
            objects->push_back(var);
        }
        break;
        }
    }
    break;
    case BER_TYPE_BOOLEAN:
    {
        unsigned char value;
        bb.GetUInt8(&value);
        CGXAsn1Variant* var = new CGXAsn1Variant(value);
        objects->push_back(var);
    }
    break;
    default:
        return DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    return 0;
}

CGXDateTime CGXAsn1Converter::GetUtcTime(
    std::string& dateString)
{
    int year, month, day, hour, minute, second = 0;
    year = 2000 + atoi(dateString.substr(0, 2).c_str());
    month = atoi(dateString.substr(2, 2).c_str());
    day = atoi(dateString.substr(4, 2).c_str());
    hour = atoi(dateString.substr(6, 2).c_str());
    minute = atoi(dateString.substr(8, 2).c_str());
    // If UTC time.
    if (GXHelpers::EndsWith(dateString, "Z"))
    {
        if (dateString.length() > 11)
        {
            second = atoi(dateString.substr(10, 2).c_str());
        }
        return CGXDateTime(year, month, day, hour, minute, second, 0, 0);
    }
    if (dateString.length() > 15)
    {
        second = atoi(dateString.substr(10, 2).c_str());
    }
    std::string tmp = dateString.substr(dateString.length() - 6, dateString.length() - 1 - (dateString.length() - 6));
    return CGXDateTime(year, month, day, hour, minute, second, 0, 0);
}

CGXDateTime CGXAsn1Converter::GetGeneralizedTime(
    std::string& dateString)
{
    int year, month, day, hour, minute, second = 0;
    year = atoi(dateString.substr(0, 4).c_str());
    month = atoi(dateString.substr(4, 2).c_str());
    day = atoi(dateString.substr(6, 2).c_str());
    hour = atoi(dateString.substr(8, 2).c_str());
    minute = atoi(dateString.substr(10, 2).c_str());
    // If UTC time.
    if (GXHelpers::EndsWith(dateString, "Z"))
    {
        if (dateString.length() > 13)
        {
            second = atoi(dateString.substr(12, 2).c_str());
        }
        return CGXDateTime(year, month, day, hour, minute, second, 0);
    }
    if (dateString.length() > 17)
    {
        second = atoi(dateString.substr(12, 2).c_str());
    }
    int deviation = 60 * atoi(dateString.substr(dateString.length() - 4, 2).c_str());
    deviation += atoi(dateString.substr(dateString.length() - 2, 2).c_str());
    CGXDateTime dt(year, month, day, hour, minute, second, 0);
    return dt;
}

std::string CGXAsn1Converter::ToString(int value, int numbers)
{
    char buff[50];
#if _MSC_VER > 1000
    sprintf_s(buff, 50, "%.*d", numbers, value);
#else
    sprintf(buff, "%.*d", numbers, value);
#endif
    return buff;
}

std::string CGXAsn1Converter::DateToString(CGXDateTime& date)
{
    std::string sb;
    sb += ToString(date.GetValue().tm_year - 2000, 2);
    sb += ToString(date.GetValue().tm_mon, 2);
    sb += ToString(date.GetValue().tm_mday, 2);
    sb += ToString(date.GetValue().tm_hour, 2);
    sb += ToString(date.GetValue().tm_min, 2);
    sb += ToString(date.GetValue().tm_sec, 2);
    sb += "Z";
    return sb;
}

int CGXAsn1Converter::GetBytes(
    CGXByteBuffer& bb,
    CGXAsn1Base* target,
    int& count)
{
    CGXByteBuffer tmp;
    std::string str;
    int ret = 0;
    int start = bb.GetSize();
    int cnt;
    if (CGXAsn1Context* a = dynamic_cast<CGXAsn1Context*>(target))
    {
        cnt = 0;
        for (std::vector<CGXAsn1Base*>::iterator it = a->GetValues()->begin(); it != a->GetValues()->end(); ++it)
        {
            cnt = 0;
            if ((ret = GetBytes(tmp, *it, cnt)) != 0)
            {
                return ret;
            }
            count += cnt;
        }
        start = bb.GetSize();
        if (a->GetConstructed())
        {
            bb.SetUInt8(BER_TYPE_CONSTRUCTED | BER_TYPE_CONTEXT | a->GetIndex());
            GXHelpers::SetObjectCount(cnt, bb);
        }
        else
        {
            tmp.SetUInt8(0, BER_TYPE_CONTEXT | a->GetIndex());
        }
        count += bb.GetSize() - start;
        bb.Set(&tmp);
        return 0;
    }
    else if (CGXAsn1Variant* a = dynamic_cast<CGXAsn1Variant*>(target))
    {
        switch (a->GetValue().vt)
        {
        case DLMS_DATA_TYPE_STRING:
            bb.SetUInt8(BER_TYPE_PRINTABLE_STRING);
            GXHelpers::SetObjectCount((unsigned long)a->GetValue().strVal.length(),
                bb);
            bb.AddString(a->GetValue().strVal);
            break;
        case DLMS_DATA_TYPE_INT8:
            if ((ret = bb.SetInt8(BER_TYPE_INTEGER)) == 0 &&
                (ret = GXHelpers::SetObjectCount(1, bb)) == 0 &&
                (ret = bb.SetInt8(a->GetValue().bVal)) == 0)
            {

            }
            break;
        case DLMS_DATA_TYPE_INT16:
            if ((ret = bb.SetUInt8(BER_TYPE_INTEGER)) == 0 &&
                (ret = GXHelpers::SetObjectCount(2, bb)) == 0 &&
                (ret = bb.SetInt16(a->GetValue().iVal)) == 0)
            {

            }
            break;
        case DLMS_DATA_TYPE_INT32:
            if ((ret = bb.SetUInt8(BER_TYPE_INTEGER)) == 0 &&
                (ret = GXHelpers::SetObjectCount(4, bb)) == 0 &&
                (ret = bb.SetInt32(a->GetValue().lVal)) == 0)
            {

            }
            break;
        case DLMS_DATA_TYPE_INT64:
            if ((ret = bb.SetUInt8(BER_TYPE_INTEGER)) == 0 &&
                (ret = GXHelpers::SetObjectCount(8, bb)) == 0 &&
                (ret = bb.SetInt64(a->GetValue().llVal)) == 0)
            {

            }
            break;
        case DLMS_DATA_TYPE_OCTET_STRING:
            if ((ret = bb.SetUInt8(BER_TYPE_OCTET_STRING)) == 0 &&
                (ret = GXHelpers::SetObjectCount(a->GetValue().size, bb)) == 0 &&
                (ret = bb.Set(a->GetValue().byteArr, a->GetValue().size)) == 0)
            {

            }
            break;
        case DLMS_DATA_TYPE_BOOLEAN:
            if ((ret = bb.SetUInt8(BER_TYPE_BOOLEAN)) == 0 &&
                (ret = bb.SetUInt8(1)) == 0 &&
                (ret = bb.SetUInt8(a->GetValue().bVal ? 255 : 0)) == 0)
            {

            }
            break;
        default:
            ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
        }
    }
    else if (CGXAsn1Integer* a = dynamic_cast<CGXAsn1Integer*>(target))
    {
        bb.SetUInt8(BER_TYPE_INTEGER);
        GXHelpers::SetObjectCount(a->GetValue().GetSize(), bb);
        bb.Set(a->GetValue().GetData(), a->GetValue().GetSize());
    }
    else if (target == NULL)
    {
        bb.SetUInt8(BER_TYPE_NULL);
        GXHelpers::SetObjectCount(0, bb);
    }
    else if (CGXAsn1ObjectIdentifier* a = dynamic_cast<CGXAsn1ObjectIdentifier*>(target))
    {
        bb.SetUInt8(BER_TYPE_OBJECT_IDENTIFIER);
        CGXByteBuffer t;
        t.Capacity(10);
        if ((ret = a->GetEncoded(t)) == 0)
        {
            GXHelpers::SetObjectCount(t.GetSize(), bb);
            bb.Set(&t);
        }
    }
    else if (CGXAsn1Set* a = dynamic_cast<CGXAsn1Set*>(target))
    {
        CGXByteBuffer tmp2;
        if (a->GetValue() != NULL)
        {
            cnt = 0;
            int count;
            if ((ret = GetBytes(tmp2, a->GetKey(), count)) != 0)
            {
                return ret;
            }
            cnt += count;
            count = 0;
            if ((ret = GetBytes(tmp2, a->GetValue(), count)) != 0)
            {
                return ret;
            }
            cnt += count;
            tmp.SetUInt8(BER_TYPE_CONSTRUCTED | BER_TYPE_SEQUENCE);
            GXHelpers::SetObjectCount(cnt, tmp);
            tmp.Set(&tmp2);
        }
        else
        {
            GetBytes(tmp2, a->GetKey(), count);
            tmp = tmp2;
        }
        // Update len.
        cnt = bb.GetSize();
        bb.SetUInt8(BER_TYPE_CONSTRUCTED | BER_TYPE_SET);
        GXHelpers::SetObjectCount(tmp.GetSize(), bb);
        bb.Set(&tmp);
        count = bb.GetSize() - cnt;
        return ret;
    }
    else if (CGXAsn1Utf8String* a = dynamic_cast<CGXAsn1Utf8String*>(target))
    {
        str = a->GetValue();
        if ((ret = bb.SetUInt8(BER_TYPE_UTF8_STRING)) == 0 &&
            (ret = GXHelpers::SetObjectCount((unsigned long)str.length(), bb)) == 0 &&
            (ret = bb.AddString(str)) == 0)
        {

        }
    }
    else if (CGXAsn1Ia5String* a = dynamic_cast<CGXAsn1Ia5String*>(target))
    {
        str = a->GetValue();
        if ((ret = bb.SetUInt8(BER_TYPE_IA5_STRING)) == 0 &&
            (ret = GXHelpers::SetObjectCount((unsigned long)str.length(), bb)) == 0 &&
            (ret = bb.AddString(str)) == 0)
        {

        }
    }
    else if (CGXAsn1BitString* bs = dynamic_cast<CGXAsn1BitString*>(target))
    {
        bs->GetValue().SetPosition(0);
        if ((ret = bb.SetUInt8(BER_TYPE_BIT_STRING)) == 0 &&
            (ret = GXHelpers::SetObjectCount(1 + bs->GetValue().GetSize(), bb)) == 0 &&
            (ret = bb.SetUInt8(bs->GetPadBits())) == 0 &&
            (ret = bb.Set(&bs->GetValue())) == 0)
        {

        }
    }
    else if (CGXAsn1Time* dt = dynamic_cast<CGXAsn1Time*>(target))
    {
        // Save date time in UTC.
        str = DateToString(dt->GetValue());
        if ((ret = bb.SetUInt8(BER_TYPE_UTC_TIME)) == 0 &&
            (ret = bb.SetUInt8((unsigned char)str.length())) == 0 &&
            (ret = bb.AddString(str)) == 0)
        {

        }
    }
    else if (CGXAsn1Sequence* seq = dynamic_cast<CGXAsn1Sequence*>(target))
    {
        tmp.Clear();
        for (std::vector<CGXAsn1Base*>::iterator it = seq->GetValues()->begin(); it != seq->GetValues()->end(); ++it)
        {
            cnt = 0;
            if ((ret = GetBytes(tmp, *it, cnt)) != 0)
            {
                return ret;
            }
            count += cnt;
        }
        start = bb.GetSize();
        if (CGXAsn1Context* c = dynamic_cast<CGXAsn1Context*>(target))
        {
            if (c->GetConstructed())
            {
                bb.SetUInt8(BER_TYPE_CONSTRUCTED | BER_TYPE_SEQUENCE | c->GetIndex());
            }
            else
            {
                bb.SetUInt8(BER_TYPE_SEQUENCE | c->GetIndex());
            }
        }
        else
        {
            bb.SetUInt8(BER_TYPE_CONSTRUCTED | BER_TYPE_SEQUENCE);
        }
        GXHelpers::SetObjectCount(count, bb);
        count += bb.GetSize() - start;
        return bb.Set(&tmp);
    }
    else
    {
        ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    count = bb.GetSize() - start;
    return ret;
}

DLMS_PKCS_TYPE CGXAsn1Converter::GetCertificateType(
    CGXByteBuffer& data,
    CGXAsn1Sequence* seq)
{
    DLMS_PKCS_TYPE ret = DLMS_PKCS_TYPE_NONE;
    CGXAsn1Base* tmp = NULL;
    if (seq == NULL)
    {
        if (FromByteArray(data, tmp) != 0)
        {
            return ret;
        }
        data.SetPosition(0);
        seq = (CGXAsn1Sequence*)tmp;
    }
    if (dynamic_cast<CGXAsn1Sequence*>(seq->GetValues()->at(0)))
    {
        CGXx509Certificate cert;
        if (CGXx509Certificate::FromByteArray(data, cert) == 0)
        {
            ret = DLMS_PKCS_TYPE_X509_CERTIFICATE;
        }
        else
        {
            CGXPkcs10 cert;
            if (CGXPkcs10::FromByteArray(data, cert) == 0)
            {
                ret = DLMS_PKCS_TYPE_PKCS10;
            }
        }
    }
    if (dynamic_cast<CGXAsn1Variant*>(seq->GetValues()->at(0)))
    {
        CGXPkcs8 cert;
        if (CGXPkcs8::FromByteArray(data, cert) == 0)
        {
            ret = DLMS_PKCS_TYPE_PKCS8;
        }
    }
    if (tmp != NULL)
    {
        delete tmp;
    }
    return ret;
}

int CGXAsn1Converter::EncodeSubject(std::string& value,
    CGXAsn1Sequence* list)
{
    DLMS_X509_NAME name;
    CGXAsn1Base* val = NULL;
    std::vector< std::string > arr = GXHelpers::Split(value, ',');
    for (std::vector<std::string>::iterator str = arr.begin();
        str != arr.end(); ++str)
    {
        std::vector< std::string > it = GXHelpers::Split(*str, "=", true);
        if (it.size() != 2)
        {
            return DLMS_ERROR_CODE_INVALID_PARAMETER;
        }
        name = CGXDLMSConverter::GetNameValue(it.at(0).c_str());
        switch (name)
        {
        case DLMS_X509_NAME_C:
            // Country code is printable string
            val = new CGXAsn1Variant(it[1]);
            break;
        case DLMS_X509_NAME_E:
            // email address in Verisign certificates
            val = new CGXAsn1Ia5String(it[1]);
            break;
        default:
            val = new CGXAsn1Utf8String(it[1]);
            break;
        }
        std::string oid = CGXDLMSConverter::ToString(name);
        CGXAsn1ObjectIdentifier* key =
            new CGXAsn1ObjectIdentifier(oid);
        list->GetValues()->push_back(
            new CGXAsn1Set(key, val));
    }
    return 0;
}

int CGXAsn1Converter::GetSubject(CGXAsn1Sequence* values,
    std::string& value)
{
    value.clear();
    CGXAsn1Variant tmp2;
    for (std::vector<CGXAsn1Base*>::iterator tmp = values->GetValues()->begin(); tmp != values->GetValues()->end(); ++tmp)
    {
        if (CGXAsn1Set* it = dynamic_cast<CGXAsn1Set*>(*tmp))
        {
            DLMS_X509_NAME name = CGXDLMSConverter::ValueOfx509Name(it->GetKey()->ToString().c_str());
            value += CGXDLMSConverter::GetName(name);
            value += '=';
            value += it->GetValue()->ToString();
            value += ", ";
        }
    }
    // Remove last comma.
    if (value.length() != 0)
    {
        value.erase(value.length() - 2);
    }
    return 0;
}

int CGXAsn1Converter::FromByteArray(
    CGXByteBuffer& data,
    CGXAsn1Base*& value)
{
    int ret;
    std::vector<CGXAsn1Base*> objects;
    while (data.GetPosition() != data.GetSize())
    {
        if ((ret = GetValue(data, &objects, false)) != 0)
        {
            return ret;
        }
    }
    if (objects.empty())
    {
        return 0;
    }
    else
    {
        value = objects.at(0);
    }
    return 0;
}

int CGXAsn1Converter::GetNext(CGXByteBuffer& data,
    CGXDLMSVariant& value)
{
    int ret;
    std::vector<CGXAsn1Base*> objects;
    if ((ret = GetValue(data, &objects, true)) == 0)
    {
        value = objects.at(0);
    }
    return ret;
}

int CGXAsn1Converter::ToByteArray(CGXAsn1Base* objects,
    CGXByteBuffer& value)
{
    int count = 0;
    return GetBytes(value, objects, count);
}

std::string CGXAsn1Converter::SystemTitleToSubject(
    CGXByteBuffer& systemTitle)
{
    return "CN=" + systemTitle.ToHexString(false);
}

int CGXAsn1Converter::SystemTitleFromSubject(std::string& subject,
    CGXByteBuffer& value)
{
    std::string tmp;
    HexSystemTitleFromSubject(subject, tmp);
    value.SetHexString(tmp);
    return 0;
}

int CGXAsn1Converter::HexSystemTitleFromSubject(
    std::string& subject,
    std::string& value)
{
    size_t index = subject.find("CN=");
    if (index == std::string::npos)
    {
        printf("System title not found from the subject.");
        return DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    value = subject.substr(index + 3, index + 3 + 16);
    return 0;
}

DLMS_KEY_USAGE CGXAsn1Converter::CertificateTypeToKeyUsage(DLMS_CERTIFICATE_TYPE type)
{
    DLMS_KEY_USAGE k;
    switch (type)
    {
    case DLMS_CERTIFICATE_TYPE_DIGITAL_SIGNATURE:
        k = DLMS_KEY_USAGE_DIGITAL_SIGNATURE;
        break;
    case DLMS_CERTIFICATE_TYPE_KEY_AGREEMENT:
        k = DLMS_KEY_USAGE_KEY_AGREEMENT;
        break;
    case DLMS_CERTIFICATE_TYPE_TLS:
        k = DLMS_KEY_USAGE_KEY_CERT_SIGN;
        break;
    case DLMS_CERTIFICATE_TYPE_OTHER:
        k = DLMS_KEY_USAGE_CRL_SIGN;
        break;
    default:
        // At least one bit must be used.
        k = DLMS_KEY_USAGE_NONE;
        break;
    }
    return k;
}

DLMS_PKCS_TYPE CGXAsn1Converter::GetCertificateType(CGXByteBuffer& data)
{
    return GetCertificateType(data, NULL);
}

DLMS_PKCS_TYPE CGXAsn1Converter::GetCertificateType(
    std::string& der)
{
    CGXByteBuffer bb;
    bb.FromBase64(der);
    return GetCertificateType(bb);
}