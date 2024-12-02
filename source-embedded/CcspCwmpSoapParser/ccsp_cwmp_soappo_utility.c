/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2015 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

/**********************************************************************
   Copyright [2014] [Cisco Systems, Inc.]
 
   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at
 
       http://www.apache.org/licenses/LICENSE-2.0
 
   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
**********************************************************************/


/**********************************************************************

    module: ccsp_cwmp_soappo_utility.c

        For CCSP CWMP protocol implementation

    ---------------------------------------------------------------

    description:

        This module implements the advanced management functions
        of the CCSP CWMP SoapParser Parser Object.

        *   CcspCwmpSoappoUtilGetNodeNameWithoutNS
        *   CcspCwmpSoappoUtilFindChildNode
        *   CcspCwmpSoappoUtilGetSoapNamespace
        *   CcspCwmpSoappoUtilGetCwmpMethod
        *   CcspCwmpSoappoUtilGetCwmpMethodName
        *   CcspCwmpSoappoUtilGetParamValue
        *   CcspCwmpSoappoUtilGetParamAttribute
        *   CcspCwmpSoappoUtilCreateSoapEnvelopeHandle
        *   CcspCwmpSoappoUtilAddSoapHeader
        *   CcspCwmpSoappoUtilProcessInvalidArgumentRequest
        *   CcspCwmpSoappoUtilProcessTooManyEnvelopes

    ---------------------------------------------------------------

    environment:

        platform independent

    ---------------------------------------------------------------

    author:

        Bin Zhu
        Kang Quan

    ---------------------------------------------------------------

    revision:

        09/08/05    initial revision.
        10/13/11    resolve name conflicts with DM library.

**********************************************************************/

#include "ccsp_cwmp_soappo_global.h"

BOOL                                bAddEncodingStyle             = TRUE;


/**********************************************************************

    prototype:

        char*
        CcspCwmpSoappoUtilGetNodeNameWithoutNS
            (
                PCHAR                       pNodeName
            );

    description:

        This function is called to get the node name without name space;

    argument:   PCHAR                       pNodeName
                The input node name;

    return:     the output node name;

**********************************************************************/
char*
CcspCwmpSoappoUtilGetNodeNameWithoutNS
    (
        PCHAR                       pNodeName
    )
{
    PCHAR                           pFound    = NULL;

    if( pNodeName == NULL)
    {
        return NULL;
    }

    pFound  = _ansc_strstr(pNodeName, CHAR_COLON);

    if( pFound == NULL)
    {
        return pNodeName;
    }

    pFound += AnscSizeOfString(CHAR_COLON);

    return pFound;
}

/**********************************************************************

    prototype:

        ANSC_HANDLE
        CcspCwmpSoappoUtilFindChildNode
            (
                ANSC_HANDLE                 hXmlNode,
                PCHAR                       pInputName
            );

    description:

        This function is called to find the child node with the
        specified node name (ignore namespace);

    argument:
                ANSC_HANDLE                 hXmlNode
                The xml node;

                PCHAR                       pInputName
                The input node name;

    return:     the node found;

**********************************************************************/
ANSC_HANDLE
CcspCwmpSoappoUtilFindChildNode
    (
        ANSC_HANDLE                 hXmlNode,
        PCHAR                       pInputName
    )
{
    PANSC_XML_DOM_NODE_OBJECT       pTempNode       = (PANSC_XML_DOM_NODE_OBJECT)hXmlNode;
    PANSC_XML_DOM_NODE_OBJECT       pChildNode      = (PANSC_XML_DOM_NODE_OBJECT)NULL;
    PCHAR                           pNodeName       = NULL;

    pChildNode = (PANSC_XML_DOM_NODE_OBJECT)
		AnscXmlDomNodeGetHeadChild(pTempNode);

    while(pChildNode != NULL)
    {
        pNodeName =
            CcspCwmpSoappoUtilGetNodeNameWithoutNS
                (
                	AnscXmlDomNodeGetName(pChildNode)
                );

        if( pNodeName != NULL && AnscEqualString(pNodeName, pInputName, TRUE))
        {
            return pChildNode;
        }

        pChildNode = (PANSC_XML_DOM_NODE_OBJECT)
			AnscXmlDomNodeGetNextChild(pTempNode, pChildNode);
    }

    return NULL;
}

/**********************************************************************

    prototype:

        ANSC_STATUS
        CcspCwmpSoappoUtilGetSoapNamespace
            (
                PCHAR                       pEnvelopeMessage,
                PCHAR                       pOutNamespaceName
            );

    description:

        This function is called to find the soap namespace value.

    argument:
                PCHAR                       pEnvelopeMessage
                The input SOAP message;

                PCHAR                       pOutNamespaceName
                The output namespace name, buffer is supposed to be
                big enough.

    return:     the status;

**********************************************************************/
ANSC_STATUS
CcspCwmpSoappoUtilGetSoapNamespace
    (
        PCHAR                       pEnvelopeMessage,
        PCHAR                       pOutNamespaceName
    )
{
    PCHAR                           pFound  = NULL;
    PCHAR                           pBegin  = pEnvelopeMessage;
    ULONG                           nLength = 0;

    if( pEnvelopeMessage[0] != '<')
    {
        return ANSC_STATUS_FAILURE;
    }

    if( _ansc_strstr(pEnvelopeMessage, "<?xml") == pEnvelopeMessage)
    {
        /* ignore the <?xml part */
        pBegin = pEnvelopeMessage + 5;
        pBegin = _ansc_strstr(pBegin, "<");
    }
    else
    {
        pFound = pBegin;
    }

    pFound = _ansc_strstr( pBegin, SOAP_ENVELOPE_2);

    if( pFound == NULL)
    {
        return ANSC_STATUS_FAILURE;
    }

    nLength= (ULONG)(pFound - pBegin);

    if( nLength >= 10)
    {
        return ANSC_STATUS_FAILURE;
    }

    /* ignore the '<' */
    pFound = pBegin + 1;
    nLength--;

    while(pFound[0] == ' ')
    {
        pFound  ++;
        nLength --;
    }

    AnscCopyMemory(pOutNamespaceName, pFound, nLength);

    return ANSC_STATUS_SUCCESS;
}

/**********************************************************************

    prototype:

        ULONG
        CcspCwmpSoappoUtilGetCwmpMethod
            (
                PCHAR                       pMethodName,
                BOOL                        bFromServer
            );

    description:

        This function is called to find method value based on the name.

    argument:
                PCHAR                       pMethodName
                The method name

                BOOL                        bFromServer
                It's a server side method or not;

    return:     the method name;

**********************************************************************/
ULONG
CcspCwmpSoappoUtilGetCwmpMethod
    (
        PCHAR                       pMethodName,
        BOOL                        bFromServer
    )
{
    if (strcmp(pMethodName, "GetRPCMethods") == 0)
    {
        return CCSP_CWMP_METHOD_GetRPCMethods;
    }

    if( !bFromServer)
    {
        if (strcmp(pMethodName, "SetParameterValues") == 0)
        {
            return CCSP_CWMP_METHOD_SetParameterValues;
        }
        else if (strcmp(pMethodName, "GetParameterValues") == 0)
        {
            return CCSP_CWMP_METHOD_GetParameterValues;
        }
        else if (strcmp(pMethodName, "GetParameterNames") == 0)
        {
            return CCSP_CWMP_METHOD_GetParameterNames;
        }
        else if (strcmp(pMethodName, "SetParameterAttributes") == 0)
        {
            return CCSP_CWMP_METHOD_SetParameterAttributes;
        }
        else if (strcmp(pMethodName, "GetParameterAttributes") == 0)
        {
            return CCSP_CWMP_METHOD_GetParameterAttributes;
        }
        else if (strcmp(pMethodName, "AddObject") == 0)
        {
            return CCSP_CWMP_METHOD_AddObject;
        }
        else if (strcmp(pMethodName, "DeleteObject") == 0)
        {
            return CCSP_CWMP_METHOD_DeleteObject;
        }
        else if (strcmp(pMethodName, "Reboot") == 0)
        {
            return CCSP_CWMP_METHOD_Reboot;
        }
        else if (strcmp(pMethodName, "Download") == 0)
        {
            return CCSP_CWMP_METHOD_Download;
        }
        else if (strcmp(pMethodName, "Upload") == 0)
        {
            return CCSP_CWMP_METHOD_Upload;
        }
        else if (strcmp(pMethodName, "FactoryReset") == 0)
        {
            return CCSP_CWMP_METHOD_FactoryReset;
        }
        else if (strcmp(pMethodName, "ChangeDUState") == 0)
        {
            return CCSP_CWMP_METHOD_ChangeDUState;
        }
        else if (strcmp(pMethodName, "GetQueuedTransfers") == 0)
        {
            return CCSP_CWMP_METHOD_GetQueuedTransfers;
        }
        else if (strcmp(pMethodName, "ScheduleInform") == 0)
        {
            return CCSP_CWMP_METHOD_ScheduleInform;
        }
        else if (strcmp(pMethodName, "SetVouchers") == 0)
        {
            return CCSP_CWMP_METHOD_SetVouchers;
        }
        else if (strcmp(pMethodName, "GetOptions") == 0)
        {
            return CCSP_CWMP_METHOD_GetOptions;
        }
        else
        {
            CcspTr069PaTraceWarning(("Unknown CWMP Client Method name: '%s'\n", pMethodName));

            return 0;
        }
    }
    else
    {
        if (strcmp(pMethodName, "Inform") == 0)
        {
            return CCSP_CWMP_METHOD_Inform;
        }
        else if (strcmp(pMethodName, "TransferComplete") == 0)
        {
            return CCSP_CWMP_METHOD_TransferComplete;
        }
        else if (strcmp(pMethodName, "AutonomousTransferComplete") == 0)
        {
            return CCSP_CWMP_METHOD_AutonomousTransferComplete;
        }
        else if (strcmp(pMethodName, "DUStateChangeComplete") == 0)
        {
            return CCSP_CWMP_METHOD_DUStateChangeComplete;
        }
        else if (strcmp(pMethodName, "AutonomousDUStateChangeComplete") == 0)
        {
            return CCSP_CWMP_METHOD_AutonomousDUStateChangeComplete;
        }
        else if (strcmp(pMethodName, "RequestDownload") == 0)
        {
            return CCSP_CWMP_METHOD_RequestDownload;
        }
        else if (strcmp(pMethodName, "Kicked") == 0)
        {
            return CCSP_CWMP_METHOD_Kicked;
        }
        else
        {
            CcspTr069PaTraceWarning(("Unknown CWMP Server Method name: '%s'\n", pMethodName));

            return 0;
        }
    }
}

/**********************************************************************

    prototype:

        ANSC_STATUS
        CcspCwmpSoappoUtilGetCwmpMethodName
            (
                ULONG                       uMethod,
                BOOL                        bFromServer
                PCHAR                       pOutBuffer
            );

    description:

        This function is called to find method name based on the value.

    argument:
                ULONG                       uMethod,
                The method value;

                BOOL                        bFromServer
                It's a server side method or not

                PCHAR                       pOutBuffer
                The output method name.

    return:     the status of the operation.

**********************************************************************/
ANSC_STATUS
CcspCwmpSoappoUtilGetCwmpMethodName
    (
        ULONG                       uMethod,
        BOOL                        bFromServer,
        PCHAR                       pOutBuffer
    )
{
    if( uMethod == CCSP_CWMP_METHOD_GetRPCMethods)
    {
        AnscCopyString(pOutBuffer,"GetRPCMethods");

        return ANSC_STATUS_SUCCESS;
    }

    if(!bFromServer)
    {
        switch ( uMethod )
        {
            case CCSP_CWMP_METHOD_SetParameterValues:

                AnscCopyString(pOutBuffer,"SetParameterValues");

                break;

            case CCSP_CWMP_METHOD_GetParameterValues:

                AnscCopyString(pOutBuffer,"GetParameterValues");

                break;

            case CCSP_CWMP_METHOD_GetParameterNames:

                AnscCopyString(pOutBuffer,"GetParameterNames");

                break;

            case CCSP_CWMP_METHOD_SetParameterAttributes:

                AnscCopyString(pOutBuffer,"SetParameterAttributes");

                break;

            case CCSP_CWMP_METHOD_GetParameterAttributes:

                AnscCopyString(pOutBuffer,"GetParameterAttributes");

                break;

            case CCSP_CWMP_METHOD_AddObject:

                AnscCopyString(pOutBuffer,"AddObject");

                break;

            case CCSP_CWMP_METHOD_DeleteObject:

                AnscCopyString(pOutBuffer,"DeleteObject");

                break;

            case CCSP_CWMP_METHOD_Reboot:

                AnscCopyString(pOutBuffer,"Reboot");

                break;

            case CCSP_CWMP_METHOD_Download:

                AnscCopyString(pOutBuffer,"Download");

                break;

            case CCSP_CWMP_METHOD_Upload:

                AnscCopyString(pOutBuffer,"Upload");

                break;

            case CCSP_CWMP_METHOD_FactoryReset:

                AnscCopyString(pOutBuffer,"FactoryReset");

                break;

            case CCSP_CWMP_METHOD_GetQueuedTransfers:

                AnscCopyString(pOutBuffer,"GetQueuedTransfers");

                break;

            case CCSP_CWMP_METHOD_ScheduleInform:

                AnscCopyString(pOutBuffer,"ScheduleInform");

                break;

            case CCSP_CWMP_METHOD_SetVouchers:

                AnscCopyString(pOutBuffer,"SetVouchers");

                break;

            case CCSP_CWMP_METHOD_GetOptions:

                AnscCopyString(pOutBuffer,"GetOptions");

                break;

            default:

                CcspTr069PaTraceError(("Unknown CWMP Client method type: %lu\n", uMethod));

                return ANSC_STATUS_FAILURE;
        }
    }
    else
    {
        switch ( uMethod )
        {
            case CCSP_CWMP_METHOD_Inform:

                AnscCopyString(pOutBuffer,"Inform");

                break;

            case CCSP_CWMP_METHOD_TransferComplete:

                AnscCopyString(pOutBuffer,"TransferComplete");

                break;

            case CCSP_CWMP_METHOD_RequestDownload:

                AnscCopyString(pOutBuffer,"RequestDownload");

                break;

            case CCSP_CWMP_METHOD_Kicked:

                AnscCopyString(pOutBuffer,"Kicked");

                break;

            default:

                CcspTr069PaTraceError(("Unknown CWMP Server method type: %lu\n", uMethod));

                return ANSC_STATUS_FAILURE;
        }
    }

    return ANSC_STATUS_SUCCESS;
}

/**********************************************************************

    prototype:

        ANSC_STATUS
        CcspCwmpSoappoUtilGetParamValue
            (
                ANSC_HANDLE                 hXmlHandle,
                ANSC_HANDLE                 hParamHandle
            );

    description:

        This function is called to get the parameter value from the
        xml handle.

    argument:
                ANSC_HANDLE                 hXmlHandle,
                The XML node;

                ANSC_HANDLE                 hParamHandle
                The parameter handle

    return:     the status of the operation.

**********************************************************************/
ANSC_STATUS
CcspCwmpSoappoUtilGetParamValue
    (
        ANSC_HANDLE                 hCcspCwmpMcoIf,
        ANSC_HANDLE                 hXmlHandle,
        ANSC_HANDLE                 hParamHandle
    )
{
    PCCSP_CWMP_MCO_INTERFACE        pCcspCwmpMcoIf = (PCCSP_CWMP_MCO_INTERFACE)hCcspCwmpMcoIf;
    PANSC_XML_DOM_NODE_OBJECT       pXmlNode     = (PANSC_XML_DOM_NODE_OBJECT)hXmlHandle;
    PANSC_XML_DOM_NODE_OBJECT       pNameNode    = (PANSC_XML_DOM_NODE_OBJECT)NULL;
    PANSC_XML_DOM_NODE_OBJECT       pValueNode   = (PANSC_XML_DOM_NODE_OBJECT)NULL;
    PCCSP_CWMP_PARAM_VALUE          pCwmpParam   = (PCCSP_CWMP_PARAM_VALUE)hParamHandle;
    PSLAP_VARIABLE                  pSlapVariable= (PSLAP_VARIABLE)NULL;
    ANSC_STATUS                     returnStatus = ANSC_STATUS_SUCCESS;
    CHAR                            pValue[1024 + 1];
    char                           *pHugeValue;
    ULONG                           length;
    ULONG                           length2;

    pCwmpParam->Value          = NULL;

    /***************************************************************************
    * Argument     | Type         | Description                               *
    ***************************************************************************
    * Name         | string(256)  | This is the name of a Parameter.          *
    *-------------------------------------------------------------------------*
    * Value        | any          | Teh is the value to be set               .*
    ***************************************************************************/

    pNameNode = (PANSC_XML_DOM_NODE_OBJECT)
		AnscXmlDomNodeGetHeadChild(pXmlNode);

    pValueNode = (PANSC_XML_DOM_NODE_OBJECT)
		AnscXmlDomNodeGetTailChild(pXmlNode);

    if( pNameNode == NULL || pValueNode == NULL || pNameNode == pValueNode)
    {
        CcspTr069PaTraceError(("Invalid arguments for ParameterValueStruct.\n"));

        return ANSC_STATUS_FAILURE;
    }

    /*
     * Get the name first;
     */
    length = 256 + 1;
    returnStatus = AnscXmlDomNodeGetDataString(pNameNode, NULL, pValue, &length);

    if (returnStatus != ANSC_STATUS_SUCCESS)
    {
        CcspTr069PaTraceError(("Failed to get the parameter name.\n"));

        return returnStatus;
    }

    if (length > 256)
    {
        CcspTr069PaTraceError(("Failed to get the parameter name.\n"));

        return ANSC_STATUS_XML_INVALID_LENGTH;
    }

    if (length == 0)
    {
        CcspTr069PaTraceError(("Empty parameter name in SetParameterValues.\n"));

        return ANSC_STATUS_FAILURE;
    }

    pCwmpParam->Name = AnscCloneString(pValue);

    pSlapVariable = (PSLAP_VARIABLE)AnscAllocateMemory(sizeof(SLAP_VARIABLE));

    if ( !pSlapVariable )
    {
        return ANSC_STATUS_RESOURCES;
    }

    SlapInitVariable(pSlapVariable);

    pCwmpParam->Value          = pSlapVariable;

    pSlapVariable->Name        = NULL;
    pSlapVariable->ContentType = 0;
    pSlapVariable->UsageType   = 0;

    /*
       Config file value could be as big as 32K, so allocate the correct size
       buffer before fetching the data to avoid unnecessary data copying etc.

       The first call to AnscXmlDomNodeGetDataString() will return an error
       (since target buffer pointer is NULL) but we rely on it still returning
       the length of the string.
    */
    length = 0;
    AnscXmlDomNodeGetDataString(pValueNode, NULL, NULL, &length);

    length2 = length + 1;

    if ((pHugeValue = AnscAllocateMemoryNoInit(length2)) == NULL)
    {
        AnscFreeMemory(pSlapVariable);
        return ANSC_STATUS_RESOURCES;
    }

    if (length == 0)
    {
        *pHugeValue = 0; /* empty string */

        pSlapVariable->ContentType = SLAP_CONTENT_TYPE_UNSPECIFIED;
        pSlapVariable->UsageType = 0;
        pSlapVariable->Syntax = SLAP_VAR_SYNTAX_string;
        pSlapVariable->Variant.varString = pHugeValue;
    }
    else
    {
        length = length2;
        returnStatus = AnscXmlDomNodeGetDataString(pValueNode, NULL, pHugeValue, &length);

        if ((returnStatus != ANSC_STATUS_SUCCESS) || (length >= length2))
        {
            CcspTr069PaTraceError(("Failed to get the string parameter value.\n"));
            AnscFreeMemory(pHugeValue);
            AnscFreeMemory(pSlapVariable);
            pCwmpParam->Value = NULL;
        }
        else
        {
            pSlapVariable->ContentType = SLAP_CONTENT_TYPE_UNSPECIFIED;
            pSlapVariable->UsageType = 0;
            pSlapVariable->Syntax = SLAP_VAR_SYNTAX_string;
            pSlapVariable->Variant.varString = pHugeValue;
        }
    }

    pCwmpParam->Tr069DataType = CCSP_CWMP_TR069_DATA_TYPE_Unspecified;

    /* retrieve data type */
    if ( TRUE )
    {
        PANSC_XML_ATTRIBUTE         pAttribute;
        
        pCwmpParam->Tr069DataType  = pCcspCwmpMcoIf->GetParamDataType(pCcspCwmpMcoIf->hOwnerContext, pCwmpParam->Name);
        pAttribute = (PANSC_XML_ATTRIBUTE)AnscXmlDomNodeGetFirstAttr(pValueNode);
        if ( pAttribute )
        {
            char *pDataType;

            if (pAttribute->DataSize >= sizeof(pValue))
            {
                return ANSC_STATUS_XML_INVALID_LENGTH;
            }

            AnscCopyMemory(pValue, pAttribute->StringData, pAttribute->DataSize);

            pValue[pAttribute->DataSize] = 0;

            pDataType = _ansc_strstr(pValue, ":");

            if ( !pDataType )
            {
                pDataType = pValue;
            }
            else
            {
                pDataType ++;
            }

            if ( AnscEqualString(pDataType, CCSP_CWMP_DATA_NAME_string, TRUE) )
            {
                pCwmpParam->Tr069DataType = CCSP_CWMP_TR069_DATA_TYPE_String;
            }
            else if ( AnscEqualString(pDataType, CCSP_CWMP_DATA_NAME_int, TRUE) )
            {
                pCwmpParam->Tr069DataType = CCSP_CWMP_TR069_DATA_TYPE_Int;
            }
            else if ( AnscEqualString(pDataType, CCSP_CWMP_DATA_NAME_unsignedInt, TRUE) )
            {
                pCwmpParam->Tr069DataType = CCSP_CWMP_TR069_DATA_TYPE_UnsignedInt;
            }
            else if ( AnscEqualString(pDataType, CCSP_CWMP_DATA_NAME_boolean, TRUE) )
            {
                pCwmpParam->Tr069DataType = CCSP_CWMP_TR069_DATA_TYPE_Boolean;
            }
            else if ( AnscEqualString(pDataType, CCSP_CWMP_DATA_NAME_dateTime, TRUE) )
            {
                pCwmpParam->Tr069DataType = CCSP_CWMP_TR069_DATA_TYPE_DateTime;
            }
            else if ( AnscEqualString(pDataType, CCSP_CWMP_DATA_NAME_base64, TRUE) )
            {
                pCwmpParam->Tr069DataType = CCSP_CWMP_TR069_DATA_TYPE_Base64;
            }
            else
            {
                pCwmpParam->Tr069DataType = CCSP_CWMP_TR069_DATA_TYPE_Unspecified;
            }
        }
    }    

    if ( pCwmpParam->Tr069DataType == CCSP_CWMP_TR069_DATA_TYPE_Unspecified )
    {
        return  ANSC_STATUS_BAD_PARAMETER;
    }

    return ANSC_STATUS_SUCCESS;
}

/**********************************************************************

    prototype:

        ANSC_STATUS
        CcspCwmpSoappoUtilGetParamAttribute
            (
                ANSC_HANDLE                 hXmlHandle,
                ANSC_HANDLE                 hAttrHandle
            );

    description:

        This function is called to get the parameter attribute from the
        xml handle.

    argument:
                ANSC_HANDLE                 hXmlHandle,
                The XML node;

                ANSC_HANDLE                 hAttrHandle
                The parameter attribute handle

    return:     the status of the operation.

**********************************************************************/
ANSC_STATUS
CcspCwmpSoappoUtilGetParamAttribute
    (
        ANSC_HANDLE                 hXmlHandle,
        ANSC_HANDLE                 hAttrHandle
    )
{
    PANSC_XML_DOM_NODE_OBJECT       pXmlNode     = (PANSC_XML_DOM_NODE_OBJECT)hXmlHandle;
    PANSC_XML_DOM_NODE_OBJECT       pListNode    = (PANSC_XML_DOM_NODE_OBJECT)NULL;
    PANSC_XML_DOM_NODE_OBJECT       pChildNode   = (PANSC_XML_DOM_NODE_OBJECT)NULL;
    PCCSP_CWMP_SET_PARAM_ATTRIB     pParamAttr   = (PCCSP_CWMP_SET_PARAM_ATTRIB)hAttrHandle;
    ANSC_STATUS                     returnStatus = ANSC_STATUS_SUCCESS;
    CHAR                            pValue[2048];
    PCHAR                           pBegin;
    ULONG                           length;
    ULONG                           uLeft;

    /***************************************************************************
    * Argument     | Type         | Description                               *
    ***************************************************************************
    * Name         | string(256)  | This is the name of a Parameter.          *
    *-------------------------------------------------------------------------*
    * Notification | boolean      | notification update                      .*
    * Change       |              |     .                                     *
    *-------------------------------------------------------------------------*
    * Notification | int(0:2)     | notification value                       .*
    *-------------------------------------------------------------------------*
    * AccessList   | boolean      | change accesslist or not                 .*
    * Change       |              |     .                                     *
    *-------------------------------------------------------------------------*
    * AccesList    | string(64)[] | array of access list                     .*
    ***************************************************************************/

    /*
     * Get the name first;
     */
    pChildNode = (PANSC_XML_DOM_NODE_OBJECT)
		AnscXmlDomNodeGetHeadChild(pXmlNode);

    if( pChildNode == NULL)
    {
        CcspTr069PaTraceError(("Invalid arguments for ParameterAttributeStruct.\n"));

        return ANSC_STATUS_FAILURE;
    }

    length = 256 + 1;
    returnStatus = AnscXmlDomNodeGetDataString(pChildNode, NULL, pValue, &length);

    if (length > 256)
    {
        returnStatus = ANSC_STATUS_FAILURE;
    }

    if (returnStatus != ANSC_STATUS_SUCCESS)
    {
        CcspTr069PaTraceError(("Failed to get the parameter name.\n"));

        return returnStatus;
    }

    pParamAttr->Name = AnscCloneString(pValue);

    /*
     * Get the notification change flag
     */
    pChildNode = (PANSC_XML_DOM_NODE_OBJECT)
		AnscXmlDomNodeGetNextChild(pXmlNode, pChildNode);

    if( pChildNode == NULL)
    {
        CcspTr069PaTraceError(("Invalid arguments for ParameterAttributeStruct.\n"));

        return ANSC_STATUS_FAILURE;
    }

    returnStatus =
    	AnscXmlDomNodeGetDataBoolean
            (
                pChildNode,
                NULL,
                &pParamAttr->bNotificationChange
            );

    if( returnStatus != ANSC_STATUS_SUCCESS)
    {
        CcspTr069PaTraceError(("Failed to get the notification flag.\n"));

        return returnStatus;
    }

    /*
     * Get the notificateion value
     */
    pChildNode = (PANSC_XML_DOM_NODE_OBJECT)
		AnscXmlDomNodeGetNextChild(pXmlNode, pChildNode);

    if( pChildNode == NULL)
    {
        CcspTr069PaTraceError(("Invalid arguments for ParameterAttributeStruct.\n"));

        return ANSC_STATUS_FAILURE;
    }

    returnStatus =
    	AnscXmlDomNodeGetDataUlong
            (
                pChildNode,
                NULL,
                (PULONG)(&pParamAttr->Notification)   /// This would cause problem on 64 bit machine
            );

    if( returnStatus != ANSC_STATUS_SUCCESS)
    {
        CcspTr069PaTraceError(("Failed to get the notification value.\n"));

        return returnStatus;
    }

    /*
     * Get the accesslist change flag
     */
    pChildNode = (PANSC_XML_DOM_NODE_OBJECT)
		AnscXmlDomNodeGetNextChild(pXmlNode, pChildNode);

    if( pChildNode == NULL)
    {
        CcspTr069PaTraceError(("Invalid arguments for ParameterAttributeStruct.\n"));

        return ANSC_STATUS_FAILURE;
    }

    returnStatus =
    	AnscXmlDomNodeGetDataBoolean
            (
                pChildNode,
                NULL,
                &pParamAttr->bAccessListChange
            );

    if( returnStatus != ANSC_STATUS_SUCCESS)
    {
        CcspTr069PaTraceError(("Failed to get the AccessListChange flag.\n"));

        return returnStatus;
    }

    /*
     * Get the accesslist
     */
    pParamAttr->AccessList = NULL;

    pListNode = (PANSC_XML_DOM_NODE_OBJECT)
		AnscXmlDomNodeGetNextChild(pXmlNode, pChildNode);

    if( pListNode == NULL)
    {
        return ANSC_STATUS_SUCCESS;
    }

    pBegin = pValue;
    uLeft = sizeof(pValue);

    *pBegin = 0;

    pChildNode = (PANSC_XML_DOM_NODE_OBJECT)
		AnscXmlDomNodeGetHeadChild(pListNode);

    while (pChildNode != NULL)
    {
        if (pBegin != pValue)
        {
            if (uLeft < 2)
            {
                CcspTr069PaTraceError(("AccessList value truncated\n"));
                return ANSC_STATUS_FAILURE;
            }

            *pBegin++ = ',';
            *pBegin = 0;
            uLeft--;
        }

        length = uLeft;
        returnStatus = AnscXmlDomNodeGetDataString(pChildNode, NULL, pBegin, &length);

        if (length >= uLeft)
        {
            returnStatus = ANSC_STATUS_FAILURE;
        }

        if (returnStatus != ANSC_STATUS_SUCCESS)
        {
            CcspTr069PaTraceError(("Failed to get the AccessList value.\n"));
            return returnStatus;
        }

        pBegin += length;
        uLeft  -= length;

        pChildNode = (PANSC_XML_DOM_NODE_OBJECT)
			AnscXmlDomNodeGetNextChild(pListNode, pChildNode);
    }

    if( pBegin != pValue)
    {
        pParamAttr->AccessList = AnscCloneString(pValue);
    }

    return ANSC_STATUS_SUCCESS;
}

/**********************************************************************

    prototype:

        ANSC_STATUS
        CcspCwmpSoappoUtilCreateSoapEnvelopeHandle
            (
                char*                       pRequestID
            );

    description:

        This function is called to create SOAP Envelope XML handle.

    argument:
                char*                       pRequestID
                The request ID value;

    return:     The Envelope XML handle;

**********************************************************************/
ANSC_HANDLE
CcspCwmpSoappoUtilCreateSoapEnvelopeHandle
    (
        char*                       pRequestID
    )
{
    PANSC_XML_DOM_NODE_OBJECT       pRootNode       = (PANSC_XML_DOM_NODE_OBJECT)NULL;
    CHAR                            pTempBuf[128];

    /* create the root node */
    pRootNode = (PANSC_XML_DOM_NODE_OBJECT)AnscCreateXmlDomNode(NULL);

    if( pRootNode == NULL)
    {
        return NULL;
    }

    pRootNode->bIgnoreXMLHeader = TRUE;

    _ansc_sprintf
        (
            pTempBuf,
            "%s:%s",
            SOAP_NAMESPACE_VALUE,
            SOAP_ENVELOPE
        );

    AnscXmlDomNodeSetName
        (
            pRootNode,
            pTempBuf
        );

    _ansc_sprintf
        (
            pTempBuf,
            "%s:%s",
            SOAP_NAMESPACE_NAME,
            SOAP_NAMESPACE_VALUE
        );

    AnscXmlDomNodeSetAttrString
        (
            pRootNode,
            pTempBuf,
            DEFAULT_SOAP_NAMESPACE,
            AnscSizeOfString(DEFAULT_SOAP_NAMESPACE)
        );

    if( bAddEncodingStyle)
    {
		snprintf
			(
				pTempBuf,
                		sizeof(pTempBuf),
				"%s:%s",
				SOAP_NAMESPACE_NAME,
				NODE_SOAP_ENCODING
			);

		AnscXmlDomNodeSetAttrString
            (
                pRootNode,
                pTempBuf,
                DEFAULT_SOAP_ENCODING,
                AnscSizeOfString(DEFAULT_SOAP_ENCODING)
            );
    }

    snprintf
        (
            pTempBuf,
            sizeof(pTempBuf),
            "%s:%s",
            SOAP_NAMESPACE_NAME,
            SOAP_XSI_NAMESPACE_NAME
        );

    AnscXmlDomNodeSetAttrString
        (
            pRootNode,
            pTempBuf,
            SOAP_XSI_NAMESPACE_VALUE,
            AnscSizeOfString(SOAP_XSI_NAMESPACE_VALUE)
        );

    snprintf
        (
            pTempBuf,
            sizeof(pTempBuf),
            "%s:%s",
            SOAP_NAMESPACE_NAME,
            SOAP_XSD_NAMESPACE_NAME
        );

    AnscXmlDomNodeSetAttrString
        (
            pRootNode,
            pTempBuf,
            SOAP_XSD_NAMESPACE_VALUE,
            AnscSizeOfString(SOAP_XSD_NAMESPACE_VALUE)
        );

    _ansc_sprintf
        (
            pTempBuf,
            "%s:%s",
            SOAP_NAMESPACE_NAME,
            CCSP_CWMP_NAMESPACE
        );

    AnscXmlDomNodeSetAttrString
        (
            pRootNode,
            pTempBuf,
            CCSP_CWMP_NAMESPACE_VALUE,
            AnscSizeOfString(CCSP_CWMP_NAMESPACE_VALUE)
        );

    CcspCwmpSoappoUtilAddSoapHeader
        (
            pRootNode,
            pRequestID
        );

    return (ANSC_HANDLE)pRootNode;
}

/**********************************************************************

    prototype:

        ANSC_STATUS
        CcspCwmpSoappoUtilAddSoapHeader
            (
                ANSC_HANDLE                 hSoapEnvelopeHandle,
                char*                       pRequestID
            );

    description:

        This function is called to add SoapHeader with ID included.

    argument:
                ANSC_HANDLE                 hSoapEnvelopeHandle,
                The Soap Envelope XML node;

                char*                       pRequestID
                The request ID value;

    return:     The status of the operation;

**********************************************************************/
ANSC_STATUS
CcspCwmpSoappoUtilAddSoapHeader
    (
        ANSC_HANDLE                 hSoapEnvelopeHandle,
        char*                       pRequestID
    )
{
    PANSC_XML_DOM_NODE_OBJECT       pRootNode      = (PANSC_XML_DOM_NODE_OBJECT)hSoapEnvelopeHandle;
    PANSC_XML_DOM_NODE_OBJECT       pHeaderNode    = (PANSC_XML_DOM_NODE_OBJECT)NULL;
    PANSC_XML_DOM_NODE_OBJECT       pChildNode     = (PANSC_XML_DOM_NODE_OBJECT)NULL;
    CHAR                            pNodeName[64];

    if( pRequestID == NULL || AnscSizeOfString(pRequestID) == 0 || hSoapEnvelopeHandle == NULL)
    {
        return ANSC_STATUS_FAILURE;
    }

    /*
     *  Create and add the header node
     */
    pHeaderNode = (PANSC_XML_DOM_NODE_OBJECT)AnscCreateXmlDomNode(NULL);

    if( pHeaderNode == NULL)
    {
        return ANSC_STATUS_RESOURCES;
    }

    snprintf
        (
            pNodeName,
            sizeof(pNodeName),
            "%s:%s",
            SOAP_NAMESPACE_VALUE,
            SOAP_HEADER_NODE
        );

    AnscXmlDomNodeSetName(pHeaderNode, pNodeName);
    AnscXmlDomNodeAddChild(pRootNode,pHeaderNode);

    /*
     *  Create and add the cwmp content
     */
    pChildNode = (PANSC_XML_DOM_NODE_OBJECT)AnscCreateXmlDomNode(NULL);

    if( pChildNode == NULL)
    {
        return ANSC_STATUS_RESOURCES;
    }

    snprintf
        (
            pNodeName,
            sizeof(pNodeName),
            "%s:%s",
            CCSP_CWMP_NAMESPACE,
            CCSP_CWMP_ID
        );

    AnscXmlDomNodeSetName(pChildNode, pNodeName);
    AnscXmlDomNodeAddChild(pHeaderNode,pChildNode);

    AnscXmlDomNodeSetDataString
        (
            pChildNode,
            NULL,
            pRequestID,
            AnscSizeOfString(pRequestID)
        );

    /*
     *  "mustUnderstand" attribute must be set to 1
     */
    snprintf
        (
            pNodeName,
            sizeof(pNodeName),
            "%s:%s",
            SOAP_NAMESPACE_VALUE,
            SOAP_MUSTUNDERSTAND
        );

    AnscXmlDomNodeSetAttrUlong
        (
            pChildNode,
            pNodeName,
            1
        );

    return ANSC_STATUS_SUCCESS;

}

/**********************************************************************

    prototype:

        ANSC_STATUS
        CcspCwmpSoappoUtilProcessErrorRequest
            (
                ANSC_HANDLE                 hCcspCwmpMcoIf,
                char*                       pRequestID,
                char*                       pMethodName
            )

    description:

        This function is called to process error requests..

    argument:
                ANSC_HANDLE                 hCcspCwmpMcoIf,
                The CCSP CWMP MCO interface handle;

                char*                       pRequestID,
                The request id;

                char*                       pMethodName,
                The request method name;

    return:     The status of the operation;

**********************************************************************/
ANSC_STATUS
CcspCwmpSoappoUtilProcessInvalidArgumentRequest
    (
        ANSC_HANDLE                 hCcspCwmpMcoIf,
        char*                       pRequestID,
        char*                       pMethodName
    )
{
    ANSC_STATUS                     returnStatus     = ANSC_STATUS_SUCCESS;
    PCCSP_CWMP_SOAP_FAULT           pCwmpSoapFault   = (PCCSP_CWMP_SOAP_FAULT)NULL;
    PCCSP_CWMP_MCO_INTERFACE             pCcspCwmpMcoIf       = (PCCSP_CWMP_MCO_INTERFACE)hCcspCwmpMcoIf;

    pCwmpSoapFault = (PCCSP_CWMP_SOAP_FAULT)AnscAllocateMemory(sizeof(CCSP_CWMP_SOAP_FAULT));

    if ( !pCwmpSoapFault )
    {
        return returnStatus;
    }
    else
    {
        pCwmpSoapFault->soap_faultcode           = AnscCloneString(CCSP_CWMP_CPE_SOAP_faultcode_invalidArgs);
        pCwmpSoapFault->soap_faultstring         = AnscCloneString(CCSP_CWMP_CPE_SOAP_faulttext_invalidArgs);
        pCwmpSoapFault->Fault.FaultCode          = CCSP_CWMP_CPE_CWMP_FaultCode_invalidArgs;
        pCwmpSoapFault->Fault.FaultString        = AnscCloneString(CCSP_CWMP_CPE_CWMP_FaultText_invalidArgs);
        pCwmpSoapFault->SetParamValuesFaultCount = 0;
    }

    if( pCcspCwmpMcoIf != NULL)
    {
        returnStatus =
            pCcspCwmpMcoIf->ProcessSoapError
                (
                    pCcspCwmpMcoIf->hOwnerContext,
                    pRequestID,
                    pMethodName,
                    (ANSC_HANDLE)pCwmpSoapFault
                );
    }

    if ( pCwmpSoapFault )
    {
        CcspCwmpFreeSoapFault(pCwmpSoapFault);
        pCwmpSoapFault = NULL;
    }

    return returnStatus;
}


ANSC_STATUS
CcspCwmpSoappoUtilProcessInvalidArgumentSPVRequest
    (
        ANSC_HANDLE                 hCcspCwmpMcoIf,
        char*                       pRequestID,
        char*                       pParamName,
        ULONG                       CwmpSpvFaultCode
    )
{
    ANSC_STATUS                     returnStatus     = ANSC_STATUS_SUCCESS;
    PCCSP_CWMP_SOAP_FAULT           pCwmpSoapFault   = (PCCSP_CWMP_SOAP_FAULT)NULL;
    PCCSP_CWMP_MCO_INTERFACE             pCcspCwmpMcoIf       = (PCCSP_CWMP_MCO_INTERFACE)hCcspCwmpMcoIf;

    pCwmpSoapFault = (PCCSP_CWMP_SOAP_FAULT)AnscAllocateMemory(sizeof(CCSP_CWMP_SOAP_FAULT) + sizeof(CCSP_CWMP_SET_PARAM_FAULT));

    if ( !pCwmpSoapFault )
    {
        return returnStatus;
    }
    else
    {
        pCwmpSoapFault->soap_faultcode           = AnscCloneString(CCSP_CWMP_CPE_SOAP_faultcode_invalidArgs);
        pCwmpSoapFault->soap_faultstring         = AnscCloneString(CCSP_CWMP_CPE_SOAP_faulttext_invalidArgs);
        pCwmpSoapFault->Fault.FaultCode          = CCSP_CWMP_CPE_CWMP_FaultCode_invalidArgs;
        pCwmpSoapFault->Fault.FaultString        = AnscCloneString(CCSP_CWMP_CPE_CWMP_FaultText_invalidArgs);
        pCwmpSoapFault->SetParamValuesFaultCount = 0;

        if ( pParamName && *pParamName != 0 )
        {
            pCwmpSoapFault->SetParamValuesFaultCount = 1;
            pCwmpSoapFault->SetParamValuesFaultArray[0].ParameterName = AnscCloneString(pParamName);
            pCwmpSoapFault->SetParamValuesFaultArray[0].FaultCode     = CwmpSpvFaultCode;

            switch ( CwmpSpvFaultCode )
            {
                case    CCSP_CWMP_CPE_CWMP_FaultCode_invalidParamName: 
                        pCwmpSoapFault->SetParamValuesFaultArray[0].FaultString = 
                            AnscCloneString(CCSP_CWMP_CPE_CWMP_FaultText_invalidParamName);
                        break;
                case    CCSP_CWMP_CPE_CWMP_FaultCode_invalidParamType: 
                        pCwmpSoapFault->SetParamValuesFaultArray[0].FaultString = 
                            AnscCloneString(CCSP_CWMP_CPE_CWMP_FaultText_invalidParamType);
                        break;
                case    CCSP_CWMP_CPE_CWMP_FaultCode_invalidParamValue: 
                        pCwmpSoapFault->SetParamValuesFaultArray[0].FaultString = 
                            AnscCloneString(CCSP_CWMP_CPE_CWMP_FaultText_invalidParamValue);
                        break;
                case    CCSP_CWMP_CPE_CWMP_FaultCode_notWritable: 
                        pCwmpSoapFault->SetParamValuesFaultArray[0].FaultString = 
                            AnscCloneString(CCSP_CWMP_CPE_CWMP_FaultText_notWritable);
                        break;
            }
        }
    }

    if( pCcspCwmpMcoIf != NULL)
    {
        returnStatus =
            pCcspCwmpMcoIf->ProcessSoapError
                (
                    pCcspCwmpMcoIf->hOwnerContext,
                    pRequestID,
                    "SetParameterValues",
                    (ANSC_HANDLE)pCwmpSoapFault
                );
    }

    if ( pCwmpSoapFault )
    {
        CcspCwmpFreeSoapFault(pCwmpSoapFault);
        pCwmpSoapFault = NULL;
    }

    return returnStatus;
}

/**********************************************************************

    prototype:

        ANSC_STATUS
        CcspCwmpSoappoUtilProcessTooManyEnvelopes
            (
                ANSC_HANDLE                 hCcspCwmpMcoIf
            )

    description:

        This function is called to process error requests..

    argument:
                ANSC_HANDLE                 hCcspCwmpMcoIf,
                The CCSP CWMP MCO interface handle;

    return:     The status of the operation;

**********************************************************************/
ANSC_STATUS
CcspCwmpSoappoUtilProcessTooManyEnvelopes
    (
        ANSC_HANDLE                 hCcspCwmpMcoIf
    )
{
    ANSC_STATUS                     returnStatus     = ANSC_STATUS_SUCCESS;
    PCCSP_CWMP_SOAP_FAULT           pCwmpSoapFault   = (PCCSP_CWMP_SOAP_FAULT)NULL;
    PCCSP_CWMP_MCO_INTERFACE             pCcspCwmpMcoIf       = (PCCSP_CWMP_MCO_INTERFACE)hCcspCwmpMcoIf;

    pCwmpSoapFault = (PCCSP_CWMP_SOAP_FAULT)AnscAllocateMemory(sizeof(CCSP_CWMP_SOAP_FAULT));

    if ( !pCwmpSoapFault )
    {
        return returnStatus;
    }
    else
    {
        pCwmpSoapFault->soap_faultcode           = AnscCloneString(CCSP_CWMP_CPE_SOAP_faultcode_maxEnvExceeded);
        pCwmpSoapFault->soap_faultstring         = AnscCloneString(CCSP_CWMP_CPE_SOAP_faulttext_maxEnvExceeded);
        pCwmpSoapFault->Fault.FaultCode          = CCSP_CWMP_CPE_CWMP_FaultCode_maxEnvExceeded;
        pCwmpSoapFault->Fault.FaultString        = AnscCloneString(CCSP_CWMP_CPE_CWMP_FaultText_maxEnvExceeded);
        pCwmpSoapFault->SetParamValuesFaultCount = 0;
    }

    if( pCcspCwmpMcoIf != NULL)
    {
        returnStatus =
            pCcspCwmpMcoIf->ProcessSoapError
                (
                    pCcspCwmpMcoIf->hOwnerContext,
                    NULL,
                    NULL,
                    (ANSC_HANDLE)pCwmpSoapFault
                );
    }

    if ( pCwmpSoapFault )
    {
        CcspCwmpFreeSoapFault(pCwmpSoapFault);
        pCwmpSoapFault = NULL;
    }

    return returnStatus;
}

