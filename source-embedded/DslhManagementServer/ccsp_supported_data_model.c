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

    module: ccsp_supported_data_model.c

        For CCSP Device.DeviceInfo.SupportedDataModel.

    ---------------------------------------------------------------

    description:

        This file implements all internal functions for supporting
        Device.DeviceInfo.SupportedDataModel.

    ---------------------------------------------------------------

    environment:

        platform independent

    ---------------------------------------------------------------

    author:

        Hui Ma 

    ---------------------------------------------------------------

    revision:

        07/19/2011    initial revision.

**********************************************************************/
#include "ansc_platform.h"
#include "ccsp_types.h"
#include "ccsp_trace.h"
#include "ccsp_supported_data_model.h"
#include "ccsp_management_server.h"
#include "ccsp_management_server_pa_api.h"
#include "ansc_string.h"
#include "ccsp_tr069pa_wrapper_api.h"


char* _SupportedDataModelConfigFile = NULL;
extern char *CcspManagementServer_ComponentName;
extern msObjectInfo *objectInfo;
int                 sdmObjectNumber = 0;
/* The node name cannot start with digits. So add 'm' before instance number. */
/*  <SupportedDataModel>
        <m1>
            <URL>aa</URL> 
            <URN>bb</URN> 
            <Features>bb</Features> 
        </m1>
        <m2>
            <URL>xx</URL> 
            <URN>yy</URN> 
            <Features>bb</Features> 
        </m2>
    </SupportedDataModel>
 */

static
VOID
LoadSDMObject
    (
        PANSC_XML_DOM_NODE_OBJECT   pArgNode,
        int                         sdmId,
        CCSP_STRING                 sdmIdStr
    )
{
    if(pArgNode == NULL) return;
    PANSC_XML_DOM_NODE_OBJECT       pChildNode     = NULL;
    CCSP_CHAR                       sdmObjectName[100];
    CCSP_CHAR                       paramterValue[CCSP_SUPPORTED_DATA_MODEL_PARAMETER_VALUE_LENGTH + 1];
    size_t                          len = strlen(sdmIdStr);
    unsigned int                    i;
    int                             id = SupportedDataModelID + sdmId;
    unsigned long                   ulSize;
    errno_t     rc  = -1;
    int ind  = -1;

    for(i=0; i<len; i++)
        sdmObjectName[i] = sdmIdStr[i];
    sdmObjectName[len] = '.';
    sdmObjectName[len+1] = '\0';

    objectInfo[id].name = CcspManagementServer_MergeString(_SupportedDataModelObjectName, sdmObjectName); /* full name end with '.' */
    objectInfo[id].numberOfChildObjects = 0;
    objectInfo[id].childObjectIDs = NULL;
    objectInfo[id].numberOfParameters = 3;
    objectInfo[id].parameters = (msParameterInfo *) AnscAllocateMemory(3 * sizeof(msParameterInfo));
    objectInfo[id].access = CCSP_RO;
    objectInfo[id].parameters[SupportedDataModelURLID].name = AnscCloneString("URL");
    objectInfo[id].parameters[SupportedDataModelURNID].name = AnscCloneString("URN");
    objectInfo[id].parameters[SupportedDataModelFeaturesID].name = AnscCloneString("Features");
    for(i = 0; i<3; i++)
    {
        objectInfo[id].parameters[i].notification = 0;
        objectInfo[id].parameters[i].access = CCSP_RO;
        objectInfo[id].parameters[i].accessControlBitmask = ~((unsigned int)0);
        objectInfo[id].parameters[i].type = ccsp_string;
        objectInfo[id].parameters[i].value = NULL;
    }

    pChildNode = (PANSC_XML_DOM_NODE_OBJECT)pArgNode->GetHeadChild(pArgNode);
    if( pChildNode == NULL) return;
    do
    {
        memset(paramterValue, 0, CCSP_SUPPORTED_DATA_MODEL_PARAMETER_VALUE_LENGTH + 1);
        ulSize = CCSP_SUPPORTED_DATA_MODEL_PARAMETER_VALUE_LENGTH + 1;
        rc = strcmp_s("URL", strlen("URL"),pChildNode->Name, &ind);
        ERR_CHK(rc);
         if((rc == EOK) && (ind == 0))
        {
            pChildNode->GetDataString(pChildNode, "URL", paramterValue, &ulSize);
            objectInfo[id].parameters[SupportedDataModelURLID].value = AnscCloneString(paramterValue);
        }
        else
        {
           rc = strcmp_s("URN", strlen("URN"),pChildNode->Name, &ind);
           ERR_CHK(rc);
           if ((rc == EOK) && (ind == 0))
           {
            pChildNode->GetDataString(pChildNode, "URN", paramterValue, &ulSize);
            objectInfo[id].parameters[SupportedDataModelURNID].value = AnscCloneString(paramterValue);
            }
          else
          {
           rc = strcmp_s("Features", strlen("Features"),pChildNode->Name, &ind);
           ERR_CHK(rc);

            if ((rc == EOK) && (ind == 0))
           {
            pChildNode->GetDataString(pChildNode, "Features", paramterValue, &ulSize);
            objectInfo[id].parameters[SupportedDataModelFeaturesID].value = AnscCloneString(paramterValue);
           }
          }

       }
    }while((pChildNode = (PANSC_XML_DOM_NODE_OBJECT)pArgNode->GetNextChild(pArgNode, pChildNode)) != NULL);
}

static
CCSP_BOOL 
LoadFromXMLFile(void*  pXMLHandle)
{
    PANSC_XML_DOM_NODE_OBJECT       pHandle        = (PANSC_XML_DOM_NODE_OBJECT)pXMLHandle;
    PANSC_XML_DOM_NODE_OBJECT       pChildNode     = NULL;
    CCSP_INT                        i;

    //CcspTraceWarning("supportedDataModel", ( "LoadFromXMLFile 0: %p\n", pXMLHandle));
    if( pXMLHandle != NULL) sdmObjectNumber = pHandle->ChildNodeQueue.Depth;
    else sdmObjectNumber = 0;
    if(sdmObjectNumber < 0) sdmObjectNumber = 0;

    CcspTraceDebug(( "LoadFromXMLFile 0: %p, smbObjNum=%d\n", pXMLHandle, sdmObjectNumber));

    objectInfo = (msObjectInfo *)AnscAllocateMemory((SupportedDataModelID + sdmObjectNumber + 1) * sizeof(msObjectInfo));

    objectInfo[SupportedDataModelID].name = AnscCloneString(_SupportedDataModelObjectName);
    objectInfo[SupportedDataModelID].numberOfChildObjects = sdmObjectNumber;
    if(sdmObjectNumber > 0)
    {
        objectInfo[SupportedDataModelID].childObjectIDs = (unsigned int *) AnscAllocateMemory(sdmObjectNumber * sizeof(unsigned int));
        for(i = 0; i<sdmObjectNumber; i++)
        {
            objectInfo[SupportedDataModelID].childObjectIDs[i] = SupportedDataModelID + i + 1;
        }
    }
    else
    {
        objectInfo[SupportedDataModelID].childObjectIDs = NULL;
    }
    objectInfo[SupportedDataModelID].numberOfParameters = 0;
    objectInfo[SupportedDataModelID].parameters = NULL;
    objectInfo[SupportedDataModelID].access = CCSP_RO;

    if(sdmObjectNumber <= 0) return TRUE;

    pChildNode = (PANSC_XML_DOM_NODE_OBJECT)pHandle->GetHeadChild(pHandle);

    do{
        /* Since xml node name cannot start with digits, add 'm' before instance number. */
        i = atoi(&(pChildNode->Name[1]));
        if(i > 0)  /* Instance number cannot be 0. */
        {
            LoadSDMObject(pChildNode, i, &(pChildNode->Name[1]));
        }
    } while((pChildNode = (PANSC_XML_DOM_NODE_OBJECT)pHandle->GetNextChild(pHandle, pChildNode)) != NULL);
    return TRUE;
}

/* Check for the existence of a config file path */
static CCSP_BOOL CheckFileExists( const char *path )
{
    FILE *file;

    /* Try and open the config file */  
    if( ( file = fopen( path, "rb" ) ) == NULL )
    {
        CcspTraceWarning(("Cannot open configuration file '%s'\n", path));
        return FALSE;
    }

    fclose( file );

    return TRUE;
}

CCSP_VOID
CcspManagementServer_FillInSDMObjectInfo()
{
//    CCSP_BOOL     bSucc         = TRUE;
    struct stat   statBuf       = {0};

    /* load from XML file */
    PANSC_XML_DOM_NODE_OBJECT       pRootNode   = NULL;

    if( CheckFileExists( _SupportedDataModelConfigFile ) )
    {
        CCSP_INT fileHandle   = open(_SupportedDataModelConfigFile,  O_RDONLY);
        CCSP_INT iContentSize = 0;

        /*RDKB-7334, CID-33035, validate file open*/
        if(fileHandle != -1)
        {
            if (fstat(fileHandle, &statBuf) != -1)
            {
                iContentSize = statBuf.st_size;

                if( iContentSize < 500000)
                {
                    char * pOrigFileContent = AnscAllocateMemory(iContentSize + 1);
                    /*RDKB-7334, CID-33035, null check and use*/
                    if(pOrigFileContent)
                    {
                        char * pFileContent = pOrigFileContent;  /*Duplicate the pointer, to prevant leak*/
                        memset(pFileContent, 0, iContentSize + 1);

                        ssize_t bytesRead = read(fileHandle, pFileContent, iContentSize);
                        if( bytesRead > 0)
                        {
                            pRootNode = (PANSC_XML_DOM_NODE_OBJECT)
                                AnscXmlDomParseString((ANSC_HANDLE)NULL, (PCHAR*)&pFileContent, bytesRead); /*"pFileContent" may get udpated*/
                        }

                        /* loca from the node */
                        if( pRootNode != NULL)
                        {
    //                        bSucc = 
                            LoadFromXMLFile((void*)pRootNode);
                            pRootNode->Remove(pRootNode);
                        }
                        /*RDKB-7334, CID-33035, free memory after use*/
                        AnscFreeMemory(pOrigFileContent);
                        pOrigFileContent = pFileContent = NULL;
                    }

                }
            }
            close(fileHandle);
        }
    }
    else
    {
        sdmObjectNumber = 0;
        objectInfo = (msObjectInfo *)AnscAllocateMemory((SupportedDataModelID + sdmObjectNumber + 1) * sizeof(msObjectInfo));

        objectInfo[SupportedDataModelID].name = AnscCloneString(_SupportedDataModelObjectName);
        objectInfo[SupportedDataModelID].numberOfChildObjects = sdmObjectNumber;
        objectInfo[SupportedDataModelID].childObjectIDs = NULL;
        objectInfo[SupportedDataModelID].numberOfParameters = 0;
        objectInfo[SupportedDataModelID].parameters = NULL;
        objectInfo[SupportedDataModelID].access = CCSP_RO;
    }
}



/* CcspManagementServer_GetSupportedDataModel_URL is called to get
 * Device.DeviceInfo.SupportedDataModel.URL.
 * Return value - the parameter value.
 */
CCSP_STRING
CcspManagementServer_GetSupportedDataModel_URL
    (
        CCSP_STRING                 ComponentName,
        int                         ObjectID
    )
{
    UNREFERENCED_PARAMETER(ComponentName);
    return AnscCloneString(objectInfo[ObjectID].parameters[SupportedDataModelURLID].value);
}

/* CcspManagementServer_GetSupportedDataModel_URN is called to get
 * Device.DeviceInfo.SupportedDataModel.URN.
 * Return value - the parameter value.
 */
CCSP_STRING
CcspManagementServer_GetSupportedDataModel_URN
    (
        CCSP_STRING                 ComponentName,
        int                         ObjectID
    )
{
    UNREFERENCED_PARAMETER(ComponentName);
    return AnscCloneString(objectInfo[ObjectID].parameters[SupportedDataModelURNID].value);
}

/* CcspManagementServer_GetSupportedDataModel_Features is called to get
 * Device.DeviceInfo.SupportedDataModel.Features.
 * Return value - the parameter value.
 */
CCSP_STRING
CcspManagementServer_GetSupportedDataModel_Features
    (
        CCSP_STRING                 ComponentName,
        int                         ObjectID
    )
{
    UNREFERENCED_PARAMETER(ComponentName);
    return AnscCloneString(objectInfo[ObjectID].parameters[SupportedDataModelFeaturesID].value);
}
