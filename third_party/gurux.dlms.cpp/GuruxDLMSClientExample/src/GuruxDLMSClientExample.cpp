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

// GuruxDLMSClientExample.cpp : Defines the entry point for the Gurux DLMS Client example.
#if defined(_WIN32) || defined(_WIN64)//Windows
#include "../include/getopt.h"
//Windows doesn't implement strcasecmp. It uses strcmpi.
#define strcasecmp _strcmpi
#else
#include <strings.h>
#endif
#include "../include/communication.h"

static void ShowHelp()
{
    printf("GuruxDlmsSample reads data from the DLMS/COSEM device.\n");
    printf("GuruxDlmsSample -h [Meter IP Address] -p [Meter Port No] -c 16 -s 1 -r SN\n");
    printf(" -h \t host name or IP address.\n");
    printf(" -p \t port number or name (Example: 1000).\n");
    printf(" -S [COM1:9600:8None1]\t serial port.");
    printf(" -a \t Authentication (None, Low, High, HighMd5, HighSha1, HighGmac, HighSha256 and HighEcdsa).\n");
    printf(" -P \t Password for authentication.\n");
    printf(" -c \t Client address. (Default: 16)\n");
    printf(" -s \t Server address. (Default: 1)\n");
    printf(" -n \t Server address as serial number.\n");
    printf(" -l \t Logical Server address.");
    printf(" -r [sn, sn]\t Short name or Logican Name (default) referencing is used.\n");
    printf(" -t Trace messages.\n");
    printf(" -g \"0.0.1.0.0.255:1; 0.0.1.0.0.255:2\" Get selected object(s) with given attribute index.\n");
    printf(" -C \t Security Level. (None, Authentication, Encrypted, AuthenticationEncryption)");
    printf(" -V \t Security Suite version. (Default: Suite0). (Suite0, Suite1 or Suite2)");
    printf(" -v \t Invocation counter data object Logical Name. Ex. 0.0.43.1.1.255");
    printf(" -I \t Auto increase invoke ID");
    printf(" -o \t Cache association view to make reading faster. Ex. -o C:\\device.xml");
    printf(" -T \t System title that is used with chiphering. Ex -T 4775727578313233");
    printf(" -A \t Authentication key that is used with chiphering. Ex -A D0D1D2D3D4D5D6D7D8D9DADBDCDDDEDF");
    printf(" -B \t Block cipher key that is used with chiphering. Ex -B 000102030405060708090A0B0C0D0E0F");
    printf(" -D \t Dedicated key that is used with chiphering. Ex -D 00112233445566778899AABBCCDDEEFF");
    printf(" -i \t Used communication interface. Ex. -i WRAPPER.");
    printf(" -m \t Used PLC MAC address. Ex. -m 1.");
    printf(" -W \t General Block Transfer window size.");
    printf(" -w \t HDLC Window size. Default is 1");
    printf(" -f \t HDLC Frame size. Default is 128");
    printf(" -L \t Manufacturer ID (Flag ID) is used to use manufacturer depending functionality. -L LGZ");
    printf("Example:\n");
    printf("Read LG device using TCP/IP connection.\n");
    printf("GuruxDlmsSample -r SN -c 16 -s 1 -h [Meter IP Address] -p [Meter Port No]\n");
    printf("Read LG device using serial port connection.\n");
    printf("GuruxDlmsSample -r SN -c 16 -s 1 -sp COM1 -i\n");
    printf("Read Indian device using serial port connection.\n");
    printf("GuruxDlmsSample -S COM1:9600:8None1 -c 16 -s 1 -a Low -P [password]\n");
}

#if defined(_WIN32) || defined(_WIN64)//Windows
int _tmain(int argc, _TCHAR* argv[])
#else
int main(int argc, char* argv[])
#endif
{
    {
#if defined(_WIN32) || defined(_WIN64)//Windows
        //Use user locale settings. This is causing errors in some Linux distros. Example Fedora
        std::locale::global(std::locale(""));
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        {
            // Tell the user that we could not find a usable WinSock DLL.
            return 1;
        }
#endif
        int ret;
        //Remove trace file if exists.
        remove("trace.txt");
        remove("LogFile.txt");
        GX_TRACE_LEVEL trace = GX_TRACE_LEVEL_INFO;
        bool useLogicalNameReferencing = true;
        int clientAddress = 16, serverAddress = 1;
        DLMS_AUTHENTICATION authentication = DLMS_AUTHENTICATION_NONE;
        DLMS_INTERFACE_TYPE interfaceType = DLMS_INTERFACE_TYPE_HDLC;
        DLMS_SECURITY security = DLMS_SECURITY_NONE;
        DLMS_SECURITY_SUITE securitySuite = DLMS_SECURITY_SUITE_V0;
        char* password = NULL;
        char* p, * p2, * readObjects = NULL;
        int index, a, b, c, d, e, f;
        int opt = 0;
        int port = 0;
        char* address = NULL;
        char* serialPort = NULL;
        bool autoIncreaseInvokeID = false;
        char* invocationCounter = NULL;
        char* outputFile = NULL;
        //Client and server certificates are exported from the meter.
        std::string exportSecuritySetupLN;

        //Generate new client and server certificates and import them to the server.
        std::string generateSecuritySetupLN;

        char* systemTitle = NULL;
        char* authenticationKey = NULL;
        char* blockCipherKey = NULL;
        char* dedicatedKey = NULL;
        uint16_t macDestinationAddress;
        unsigned char gbtWindowSize = 1;
        unsigned char windowSize = 1;
        uint16_t maxInfo = 128;
        char* manufacturerId = NULL;
        while ((opt = getopt(argc, argv, "h:p:c:s:r:i:d:t:a:P:g:S:n:C:v:o:T:A:B:D:m:l:W:w:f:L:V:N:E:")) != -1)
        {
            switch (opt)
            {
            case 'r':
                if (strcasecmp("sn", optarg) == 0)
                {
                    useLogicalNameReferencing = false;
                }
                else if (strcasecmp("ln", optarg) == 0)
                {
                    useLogicalNameReferencing = 1;
                }
                else
                {
                    printf("Invalid reference option.\n");
                    return 1;
                }
                break;
            case 'h':
                //Host address.
                address = optarg;
                break;
            case 't':
                //Trace.
                if (strcasecmp("Error", optarg) == 0)
                    trace = GX_TRACE_LEVEL_ERROR;
                else  if (strcasecmp("Warning", optarg) == 0)
                    trace = GX_TRACE_LEVEL_WARNING;
                else  if (strcasecmp("Info", optarg) == 0)
                    trace = GX_TRACE_LEVEL_INFO;
                else  if (strcasecmp("Verbose", optarg) == 0)
                    trace = GX_TRACE_LEVEL_VERBOSE;
                else  if (strcasecmp("Off", optarg) == 0)
                    trace = GX_TRACE_LEVEL_OFF;
                else
                {
                    printf("Invalid trace option '%s'. (Error, Warning, Info, Verbose, Off)", optarg);
                    return 1;
                }
                break;
            case 'p':
                //Port.
                port = atoi(optarg);
                break;
            case 'P':
                password = optarg;
                break;
            case 'i':
                //Interface
                if (strcasecmp("HDLC", optarg) == 0)
                    interfaceType = DLMS_INTERFACE_TYPE_HDLC;
                else  if (strcasecmp("WRAPPER", optarg) == 0)
                    interfaceType = DLMS_INTERFACE_TYPE_WRAPPER;
                else  if (strcasecmp("HdlcModeE", optarg) == 0)
                    interfaceType = DLMS_INTERFACE_TYPE_HDLC_WITH_MODE_E;
                else  if (strcasecmp("Plc", optarg) == 0)
                    interfaceType = DLMS_INTERFACE_TYPE_PLC;
                else if (strcasecmp("PlcHdlc", optarg) == 0)
                    interfaceType = DLMS_INTERFACE_TYPE_PLC_HDLC;
                else if (strcasecmp("PlcPrime", optarg) == 0)
                    interfaceType = DLMS_INTERFACE_TYPE_PLC_PRIME;
                else
                {
                    printf("Invalid interface option '%s'. (HDLC, WRAPPER, HdlcModeE, Plc, PlcHdlc)", optarg);
                    return 1;
                }
                break;
            case 'I':
                // AutoIncreaseInvokeID.
                autoIncreaseInvokeID = true;
                break;
            case 'C':
                if (strcasecmp("None", optarg) == 0)
                {
                    security = DLMS_SECURITY_NONE;
                }
                else if (strcasecmp("Authentication", optarg) == 0)
                {
                    security = DLMS_SECURITY_AUTHENTICATION;
                }
                else if (strcasecmp("Encryption", optarg) == 0)
                {
                    security = DLMS_SECURITY_ENCRYPTION;
                }
                else if (strcasecmp("AuthenticationEncryption", optarg) == 0)
                {
                    security = DLMS_SECURITY_AUTHENTICATION_ENCRYPTION;
                }
                else
                {
                    printf("Invalid Ciphering option '%s'. (None, Authentication, Encryption, AuthenticationEncryption)", optarg);
                    return 1;
                }
                break;
            case 'V':
                if (strcasecmp("Suite0", optarg) == 0)
                {
                    securitySuite = DLMS_SECURITY_SUITE_V0;
                }
                else if (strcasecmp("Suite1", optarg) == 0)
                {
                    securitySuite = DLMS_SECURITY_SUITE_V1;
                }
                else if (strcasecmp("Suite2", optarg) == 0)
                {
                    securitySuite = DLMS_SECURITY_SUITE_V2;
                }
                else
                {
                    printf("Invalid Security suite option '%s'. (Suite0, Suite1, Suite2)", optarg);
                    return 1;
                }
                break;
            case 'T':
                systemTitle = optarg;
                break;
            case 'A':
                authenticationKey = optarg;
                break;
            case 'B':
                blockCipherKey = optarg;
                break;
            case 'D':
                dedicatedKey = optarg;
                break;
            case 'N':
                generateSecuritySetupLN = optarg;
                break;
            case 'E':
                exportSecuritySetupLN = optarg;
                break;
            case 'o':
                outputFile = optarg;
                break;
            case 'v':
                invocationCounter = optarg;
#if defined(_WIN32) || defined(_WIN64)//Windows
                if ((ret = sscanf_s(optarg, "%d.%d.%d.%d.%d.%d", &a, &b, &c, &d, &e, &f)) != 6)
#else
                if ((ret = sscanf(optarg, "%d.%d.%d.%d.%d.%d", &a, &b, &c, &d, &e, &f)) != 6)
#endif
                {
                    ShowHelp();
                    return 1;
                }
                break;
            case 'g':
                //Get (read) selected objects.
                p = optarg;
                do
                {
                    if (p != optarg)
                    {
                        ++p;
                }
#if defined(_WIN32) || defined(_WIN64)//Windows
                    if ((ret = sscanf_s(p, "%d.%d.%d.%d.%d.%d:%d", &a, &b, &c, &d, &e, &f, &index)) != 7)
#else
                    if ((ret = sscanf(p, "%d.%d.%d.%d.%d.%d:%d", &a, &b, &c, &d, &e, &f, &index)) != 7)
#endif
                    {
                        ShowHelp();
                        return 1;
                    }
            } while ((p = strchr(p, ',')) != NULL);
            readObjects = optarg;
            break;
            case 'S':
                serialPort = optarg;
                break;
            case 'a':
                if (strcasecmp("None", optarg) == 0)
                {
                    authentication = DLMS_AUTHENTICATION_NONE;
                }
                else if (strcasecmp("Low", optarg) == 0)
                {
                    authentication = DLMS_AUTHENTICATION_LOW;
                }
                else if (strcasecmp("High", optarg) == 0)
                {
                    authentication = DLMS_AUTHENTICATION_HIGH;
                }
                else if (strcasecmp("HighMd5", optarg) == 0)
                {
                    authentication = DLMS_AUTHENTICATION_HIGH_MD5;
                }
                else if (strcasecmp("HighSha1", optarg) == 0)
                {
                    authentication = DLMS_AUTHENTICATION_HIGH_SHA1;
                }
                else if (strcasecmp("HighGmac", optarg) == 0)
                {
                    authentication = DLMS_AUTHENTICATION_HIGH_GMAC;
                }
                else if (strcasecmp("HighSha256", optarg) == 0)
                {
                    authentication = DLMS_AUTHENTICATION_HIGH_SHA256;
                }
                else if (strcasecmp("HighEcdsa", optarg) == 0)
                {
                    authentication = DLMS_AUTHENTICATION_HIGH_ECDSA;
                }
                else
                {
                    printf("Invalid Authentication option. (None, Low, High, HighMd5, HighSha1, HighGmac, HighSha256 and HighEcdsa)\n");
                    return 1;
                }
                break;
            case 'c':
                clientAddress = atoi(optarg);
                break;
            case 's':
                if (serverAddress != 1)
                {
                    serverAddress = CGXDLMSClient::GetServerAddress(serverAddress, atoi(optarg));
                }
                else
                {
                    serverAddress = atoi(optarg);
                }
                break;
            case 'l':
                serverAddress = CGXDLMSClient::GetServerAddress(atoi(optarg), serverAddress);
                break;
            case 'n':
                serverAddress = CGXDLMSClient::GetServerAddressFromSerialNumber(atoi(optarg), 1);
                break;
            case 'm':
                macDestinationAddress = atoi(optarg);
                break;
            case 'W':
                gbtWindowSize = atoi(optarg);
                break;
            case 'w':
                windowSize = atoi(optarg);
                break;
            case 'f':
                maxInfo = atoi(optarg);
                break;
            case 'L':
                manufacturerId = optarg;
                break;
            case '?':
            {
                if (optarg[0] == 'c') {
                    printf("Missing mandatory client option.\n");
                }
                else if (optarg[0] == 's') {
                    printf("Missing mandatory server option.\n");
                }
                else if (optarg[0] == 'h') {
                    printf("Missing mandatory host name option.\n");
                }
                else if (optarg[0] == 'p') {
                    printf("Missing mandatory port option.\n");
                }
                else if (optarg[0] == 'r') {
                    printf("Missing mandatory reference option.\n");
                }
                else if (optarg[0] == 'a') {
                    printf("Missing mandatory authentication option.\n");
                }
                else if (optarg[0] == 'S') {
                    printf("Missing mandatory Serial port option.\n");
                }
                else if (optarg[0] == 'g') {
                    printf("Missing mandatory OBIS code option.\n");
                }
                else if (optarg[0] == 'C') {
                    printf("Missing mandatory Ciphering option.\n");
                }
                else if (optarg[0] == 'v') {
                    printf("Missing mandatory invocation counter logical name option.\n");
                }
                else if (optarg[0] == 'T') {
                    printf("Missing mandatory system title option.");
                }
                else if (optarg[0] == 'A') {
                    printf("Missing mandatory authentication key option.");
                }
                else if (optarg[0] == 'B') {
                    printf("Missing mandatory block cipher key option.");
                }
                else if (optarg[0] == 'D') {
                    printf("Missing mandatory dedicated key option.");
                }
                else
                {
                    ShowHelp();
                    return 1;
                }
            }
            break;
            default:
                ShowHelp();
                return 1;
        }
    }
        CGXDLMSSecureClient cl(useLogicalNameReferencing, clientAddress, serverAddress, authentication, password, interfaceType);
        cl.GetCiphering()->SetSecurity(security);
        cl.GetCiphering()->SetSecuritySuite(securitySuite);
        cl.SetAutoIncreaseInvokeID(autoIncreaseInvokeID);
        cl.SetGbtWindowSize(gbtWindowSize);
        cl.GetHdlcSettings().SetWindowSizeRX(windowSize);
        cl.GetHdlcSettings().SetWindowSizeTX(windowSize);
        cl.GetHdlcSettings().SetMaxInfoRX(maxInfo);
        cl.GetHdlcSettings().SetMaxInfoTX(maxInfo);
        cl.GetHdlcSettings().SetMaxInfoTX(maxInfo);
        cl.SetManufacturerId(manufacturerId);
        CGXByteBuffer bb;
        if (systemTitle != NULL)
        {
            bb.Clear();
            bb.SetHexString(systemTitle);
            cl.GetCiphering()->SetSystemTitle(bb);
        }
        if (authenticationKey != NULL)
        {
            bb.Clear();
            bb.SetHexString(authenticationKey);
            cl.GetCiphering()->SetAuthenticationKey(bb);
        }
        if (blockCipherKey != NULL)
        {
            bb.Clear();
            bb.SetHexString(blockCipherKey);
            cl.GetCiphering()->SetBlockCipherKey(bb);
        }
        if (dedicatedKey != NULL)
        {
            bb.Clear();
            bb.SetHexString(dedicatedKey);
            cl.GetCiphering()->SetDedicatedKey(bb);
        }
        CGXCommunication comm(&cl, 5000, trace, invocationCounter);

        if (port != 0 || address != NULL)
        {
            if (port == 0)
            {
                printf("Missing mandatory port option.\n");
                return 1;
            }
            if (address == NULL)
            {
                printf("Missing mandatory host option.\n");
                return 1;
            }
            if ((ret = comm.Connect(address, port)) != 0)
            {
                printf("Connect failed %s.\n", CGXDLMSConverter::GetErrorMessage(ret));
                return 1;
            }
        }
        else if (serialPort != NULL)
        {
            if ((ret = comm.Open(serialPort)) != 0)
            {
                if (ret == DLMS_ERROR_CODE_RECEIVE_FAILED)
                {
                    printf("Failed to receive reply for IEC. Check is DLMS connection already established.\n");
                }
                else
                {
                    printf("Serial port open failed %s.\n", CGXDLMSConverter::GetErrorMessage(ret));
                    comm.Close();
                    return 1;
                }
            }
        }
        else
        {
            printf("Missing mandatory connection information for TCP/IP or serial port connection.\n");
            printf("-----------------------------------------------------------------------------.\n");
            ShowHelp();
            return 1;
        }
        //Export client and server certificates from the meter.
        if (!exportSecuritySetupLN.empty())
        {
            comm.ExportMeterCertificates(exportSecuritySetupLN);
        }
        //Generate new client and server certificates and import them to the server.
        else if (!generateSecuritySetupLN.empty())
        {
            comm.GenerateCertificates(generateSecuritySetupLN);
        }
        else if (readObjects != NULL)
        {
            bool read = false;
            if (outputFile != NULL)
            {
                if ((ret = cl.GetObjects().Load(outputFile)) == 0)
                {
                    ret = 0;
                    read = true;
                }
            }
            ret = comm.InitializeConnection();
            if (ret == 0 && !read)
            {
                ret = comm.GetAssociationView();
            }
            if (ret == 0)
            {
                std::string str;
                std::string value;
                char buff[200];
                p = readObjects;
                do
                {
                    if (p != readObjects)
                    {
                        ++p;
                    }
                    str.clear();
                    p2 = strchr(p, ':');
                    ++p2;
#if defined(_WIN32) || defined(_WIN64)//Windows
                    sscanf_s(p2, "%d", &index);
#else
                    sscanf(p2, "%d", &index);
#endif
                    str.append(p, p2 - p);
                    CGXDLMSObject* obj = cl.GetObjects().FindByLN(DLMS_OBJECT_TYPE_ALL, str);
                    if (obj != NULL)
                    {
                        value.clear();
                        if ((ret = comm.Read(obj, index, value)) != DLMS_ERROR_CODE_OK)
                        {
#if _MSC_VER > 1000
                            sprintf_s(buff, sizeof(buff), "Error! Index: %d %s\n", index, CGXDLMSConverter::GetErrorMessage(ret));
#else
                            sprintf(buff, "Error! Index: %d read failed: %s\n", index, CGXDLMSConverter::GetErrorMessage(ret));
#endif
                            comm.WriteValue(GX_TRACE_LEVEL_ERROR, buff);
                            //Continue reading.
                        }
                        else
                        {
#if _MSC_VER > 1000
                            sprintf_s(buff, sizeof(buff), "Index: %d Value: ", index);
#else
                            sprintf(buff, "Index: %d Value: ", index);
#endif
                            comm.WriteValue(trace, buff);
                            comm.WriteValue(trace, value.c_str());
                            comm.WriteValue(trace, "\n");
                    }
                }
                    else
                    {
#if _MSC_VER > 1000
                        sprintf_s(buff, 100, "Unknown object: %s", str.c_str());
#else
                        sprintf(buff, "Unknown object: %s", str.c_str());
#endif
                        str = buff;
                        comm.WriteValue(GX_TRACE_LEVEL_ERROR, str);
                    }
            } while ((p = strchr(p, ',')) != NULL);
            //Close connection.
            comm.Close();
            if (outputFile != NULL && ret == 0)
            {
                ret = cl.GetObjects().Save(outputFile);
            }
        }
}
        else
        {
            ret = comm.ReadAll(outputFile);
        }
        //Close connection.
        comm.Close();
    }
#if defined(_WIN32) || defined(_WIN64)//Windows
    WSACleanup();
#if _MSC_VER > 1400
    _CrtDumpMemoryLeaks();
#endif
#endif
    return 0;
}

