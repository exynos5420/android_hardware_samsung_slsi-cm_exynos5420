/*
Copyright  © Trustonic Limited 2013

All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice, this
     list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution.

  3. Neither the name of the Trustonic Limited nor the names of its contributors
     may be used to endorse or promote products derived from this software
     without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include <libxml/parser.h>
#include <libxml/valid.h>
#include <libxml/xmlschemas.h>

#include <mcVersionInfo.h>

#include "logging.h"
#include "enrollmentservicexmlschema.h"
#include "xmlmessagehandler.h"
#include "contentmanager.h"
#include "provisioningengine.h"
#include "base64.h"

#define ENROLLMENT_SERVICE_NS_PREFIX 0 // "mces"
#define ENROLLMENT_SERVICE_NAMESPACE "http://www.mcore.gi-de.com/2012/04/schema/EnrollmentService"

#define PLATFORM_TYPES_NS_PREFIX "mcpt"
#define PLATFORM_TYPES_NAMESPACE "http://www.mcore.gi-de.com/2012/02/schema/MCPlatformTypes"

#define XSD_PATH_MAX_LEN 256

#define UNKNOWN_ID 0xFFFFFFFF

typedef enum
{
    CMP,
    SO_UPLOAD,
	TLT_UPLOAD,
    UNKNOWN_TYPE=0xFFFFFFFF
} commandtype_t;

static char enrollmentServiceFullPath_[XSD_PATH_MAX_LEN];
static char platformTypesFullPath_[XSD_PATH_MAX_LEN];
static xmlNsPtr nameSpace_=NULL;
static xmlNsPtr typesNameSpace_=NULL;

// file internal functions

xmlDocPtr createXmlResponse()
{
    xmlDocPtr docP = NULL;
    xmlNodePtr root_node = NULL;

    docP = xmlNewDoc(BAD_CAST "1.0");
    root_node = xmlNewNode(nameSpace_, BAD_CAST "ContentManagementResponse");
    nameSpace_=xmlNewNs(root_node, (const xmlChar*) ENROLLMENT_SERVICE_NAMESPACE, (const xmlChar*) ENROLLMENT_SERVICE_NS_PREFIX );
    typesNameSpace_=xmlNewNs(root_node, (const xmlChar*) PLATFORM_TYPES_NAMESPACE, (const xmlChar*) PLATFORM_TYPES_NS_PREFIX );
    xmlSetNs(root_node, nameSpace_);
    xmlDocSetRootElement(docP, root_node);
    docP->standalone=1;
    return docP;
}


bool addTrustletData(xmlNodePtr rootNode, bool tltBin, char* contentP)
{
    xmlNodePtr trustletDataNode=xmlNewChild(rootNode, nameSpace_, BAD_CAST "trustletData", NULL);
    if(NULL==trustletDataNode) return false;
    char* element="encryptedKey";
    if(tltBin)
    {
        element="tltBin";
    }

    if(xmlNewChild(trustletDataNode, nameSpace_, BAD_CAST element, BAD_CAST contentP)==NULL ) return false;
    return true;
}


char* errorCodeToString(rootpaerror_t errorCode)
{
    switch(errorCode)
    {
        case ROOTPA_COMMAND_NOT_SUPPORTED:
            return STRING_ROOTPA_COMMAND_NOT_SUPPORTED;

        case ROOTPA_ERROR_LOCK:
            return STRING_ROOTPA_ERROR_LOCK;

//
// this is not currently understood by SE
//
//        case ROOTPA_ERROR_COMMAND_EXECUTION:
//            return STRING_ROOTPA_ERROR_COMMAND_EXECUTION;

        case ROOTPA_ERROR_REGISTRY:
            return STRING_ROOTPA_ERROR_REGISTRY;

        case ROOTPA_ERROR_MOBICORE_CONNECTION:
            return STRING_ROOTPA_ERROR_MOBICORE_CONNECTION;

        case ROOTPA_ERROR_OUT_OF_MEMORY:
            return STRING_ROOTPA_ERROR_OUT_OF_MEMORY;

        case ROOTPA_ERROR_INTERNAL:
            return STRING_ROOTPA_ERROR_INTERNAL;

        case ROOTPA_ERROR_XML:
            return STRING_ROOTPA_ERROR_XML;

        case ROOTPA_ERROR_REGISTRY_OBJECT_NOT_AVAILABLE:
            return STRING_ROOTPA_ERROR_REGISTRY_OBJECT_NOT_AVAILABLE;

    }
    LOGD("errorCodeToString: unknown error code %d", errorCode);
    return STRING_ROOTPA_ERROR_INTERNAL;
}

bool addCommandResultData(xmlNodePtr resultListNode, int id,  char* commandResultP, rootpaerror_t errorCode, uint32_t errorDetail )
{
    xmlNodePtr commandResultNode=xmlNewChild(resultListNode, nameSpace_, BAD_CAST "commandResult", NULL);
    if(NULL==commandResultNode) return false;

    bool retValue=true;
    char intBuffer[11];

    sprintf(intBuffer,"%u",(uint32_t) id);
    if(xmlNewProp(commandResultNode, BAD_CAST "id", BAD_CAST intBuffer)==NULL) return false;

    if(commandResultP==NULL)
    {
        xmlNodePtr errorNode=xmlNewChild(commandResultNode, nameSpace_, BAD_CAST "resultError", NULL);
        if( NULL==errorNode ) return false;    // CommandExecutionError

        if(xmlNewProp(errorNode, BAD_CAST "errorCode", BAD_CAST errorCodeToString(errorCode))==NULL)
        {
            retValue=false;
        }
        else if(errorDetail!=0)
        {
            sprintf(intBuffer,"%u",errorDetail);
            if(xmlNewProp(errorNode, BAD_CAST "errorDetail", BAD_CAST intBuffer)==NULL)
            {
                retValue=false;
            }
        }
    }
    else
    {
        if(xmlNewChild(commandResultNode, nameSpace_, BAD_CAST "resultValue", BAD_CAST commandResultP)==NULL )
        {
            retValue=false;
        }
    }

    return retValue;
}

xmlNodePtr findFirstCommandNode(xmlDocPtr xmlDocP)
{
    xmlNodePtr rootElementP = xmlDocGetRootElement(xmlDocP);
    if(NULL==rootElementP) return NULL;

    xmlNodePtr commandsNodeP=rootElementP->children;
    for (; commandsNodeP; commandsNodeP = commandsNodeP->next)
    {
        if (commandsNodeP->type == XML_ELEMENT_NODE && strcmp((char*)commandsNodeP->name, "commands")==0)
        {
            break;
        }
    }
    if(NULL==commandsNodeP) return NULL;
    return commandsNodeP->children;
}

xmlNodePtr getNextCommand(xmlDocPtr xmlDocP, xmlNodePtr prevNode)
{
    LOGD(">> getNextCommand %ld %ld", (long int) xmlDocP, (long int) prevNode);
    xmlNodePtr firstNode;
    if(NULL==prevNode)
    {
        firstNode=findFirstCommandNode(xmlDocP);
    }
    else
    {
        firstNode=prevNode->next;
    }

    xmlNodePtr commandNode;
    for (commandNode = firstNode; commandNode; commandNode = commandNode->next)
    {
        if (commandNode->type == XML_ELEMENT_NODE && strcmp((char*)commandNode->name, "command")==0)
        {
            break;
        }
    }
    LOGD("<< getNextCommand %ld", (long int) commandNode);
    return commandNode;
}


int getCommandId(xmlNodePtr commandNode)
{
    xmlChar* idP=xmlGetProp(commandNode, BAD_CAST "id");
    if(NULL==idP)
    {
        return UNKNOWN_ID;
    }
    int id=atoi((char*)idP);
    xmlFree(idP);
    return id;
}

commandtype_t getCommandType(xmlNodePtr commandNode)
{
    if(NULL==commandNode) return UNKNOWN_TYPE;
    xmlChar* typeP=xmlGetProp(commandNode, BAD_CAST "type");
    commandtype_t type=UNKNOWN_TYPE;
    if(typeP!=NULL)
    {
        if(strcmp((char*)typeP,"CMP")==0) type=CMP;
        else if(strcmp((char*)typeP,"SO_UPLOAD")==0) type=SO_UPLOAD;
        else if(strcmp((char*)typeP,"TLT_UPLOAD")==0) type=TLT_UPLOAD;
        xmlFree(typeP);
    }
    else
    {
        LOGE("type property does not exist");
    }
    return type;
}

/**
Note, the caller has to release the memory with xmlFree
*/
char* getCommandValue(xmlNodePtr commandNode)
{
    xmlNodePtr commandValueNodeP=commandNode->children;

    for (; commandValueNodeP; commandValueNodeP = commandValueNodeP->next)
    {
        if (commandValueNodeP->type == XML_ELEMENT_NODE && strcmp((char*)commandValueNodeP->name, "commandValue")==0)
        {
            break;
        }
    }
    if(NULL==commandValueNodeP) return NULL;

    return (char*) xmlNodeGetContent(commandValueNodeP);
}


bool getCommandIgnoreError(xmlNodePtr commandNode)
{
    xmlChar* attribute=xmlGetProp(commandNode, BAD_CAST "ignoreError");
    bool ignoreError=false; // default value is false
    if(NULL!=attribute)
    {
        if(strcmp((char*)attribute, "true")==0)
        {
            ignoreError=true;
        }
        xmlFree(attribute);
    }

    return ignoreError;
}

void getValues(xmlNodePtr commandNodeP, commandtype_t* commandTypeP, uint32_t* idP, char** commandValueP, bool* ignoreErrorP)
{
    *commandTypeP=getCommandType(commandNodeP);
    *idP=getCommandId(commandNodeP);
    *commandValueP=(char*) getCommandValue(commandNodeP);
    *ignoreErrorP=getCommandIgnoreError(commandNodeP);
}

uint32_t extractCmpCommand(CmpMessage** cmpCommandsP, uint32_t numberOfCmpCommands, uint32_t id, char* commandValueP, bool ignoreError)
{
    CmpMessage* localCommandsP=*cmpCommandsP; // localCommandsP is just to make the code a bit more readable
    CmpMessage* tmpCommandsP=realloc(localCommandsP, sizeof(CmpMessage)*(numberOfCmpCommands+1));

    if(tmpCommandsP!=NULL)
    {
        localCommandsP=tmpCommandsP;
        *cmpCommandsP=localCommandsP;

        memset(&(localCommandsP[numberOfCmpCommands]), 0,sizeof(CmpMessage));
        if(commandValueP)
        {
            localCommandsP[numberOfCmpCommands].length= base64DecodeStringRemoveEndZero(commandValueP,
	                                                              (char**) &(localCommandsP[numberOfCmpCommands].contentP));
            if(0==localCommandsP[numberOfCmpCommands].length)
            {
                LOGE("handleCmpCommand: base64 decoding failed");
            }
            localCommandsP[numberOfCmpCommands].hdr.id=id;
            localCommandsP[numberOfCmpCommands].hdr.ignoreError=ignoreError;
            numberOfCmpCommands++;
        }
        else
        {
            localCommandsP[numberOfCmpCommands].hdr.ret=ROOTPA_ERROR_XML;
        }
    }
    else
    {
        LOGE("handleCmpCommand: was not able to realloc");
        // In this case we can not return an error to SE unless we set some of the earlier errors.
        if(!ignoreError)
        {
            free(*cmpCommandsP);
            *cmpCommandsP=NULL;
            numberOfCmpCommands=0;
        }
    }
    return numberOfCmpCommands;
}

rootpaerror_t handleCmpResponses(uint32_t maxNumberOfCmpResponses, CmpMessage* cmpResponsesP, xmlNodePtr rspResultElementP)
{
    LOGD(">>handleCmpResponses %d", maxNumberOfCmpResponses);
    rootpaerror_t ret=ROOTPA_OK;
    uint32_t i;

    for(i=0; (i<maxNumberOfCmpResponses) && (ROOTPA_OK==ret); i++)
    {
        char* encodedResponseP=NULL;
        if((ROOTPA_ERROR_COMMAND_EXECUTION==cmpResponsesP[i].hdr.ret ||
            ROOTPA_OK==cmpResponsesP[i].hdr.ret) && cmpResponsesP[i].contentP!=NULL)
        {
            encodedResponseP=base64EncodeAddEndZero((char*) cmpResponsesP[i].contentP, cmpResponsesP[i].length);
            if(NULL==encodedResponseP)
            {
                LOGE("handleCmpResponses: base64 encoding failed %d", i);
                cmpResponsesP[i].hdr.ret=ROOTPA_ERROR_INTERNAL;
                if(cmpResponsesP[i].hdr.ignoreError==false) ret=ROOTPA_ERROR_INTERNAL;
            }
        }

        if( addCommandResultData(rspResultElementP, cmpResponsesP[i].hdr.id, encodedResponseP, cmpResponsesP[i].hdr.ret, cmpResponsesP[i].hdr.intRet )==false )
        {
            ret=ROOTPA_ERROR_XML;
        }
        free(encodedResponseP);

        if(cmpResponsesP[i].hdr.ret != ROOTPA_OK && false == cmpResponsesP[i].hdr.ignoreError)
        {
            LOGD("handleCmpResponses, stopping since the current message (%d) has error %d and ignoreError false", i, cmpResponsesP[i].hdr.ret);
            break;
        }
    }
    LOGD("<<handleCmpResponses %d", ret);
    return ret;
}

uint32_t handleUploadCommand(commandtype_t commandType,
                             CommonMessage** uploadCommandsP,
                             uint32_t numberOfUploadCommands,
                             uint32_t id,
                             char* commandValueP,
                             bool ignoreError)
{
    LOGD(">>handleUploadCommand %d %lx %lx", commandType, (long int) uploadCommandsP, (long int) *uploadCommandsP);
    CommonMessage* localCommandsP=*uploadCommandsP; // localCommandsP is just to make the code a bit more readable
    CommonMessage* tmpCommandsP=realloc(localCommandsP, sizeof(CommonMessage)*(numberOfUploadCommands+1));

    if(NULL == tmpCommandsP)
    {
        LOGE("handleUploadCommand: was not able to realloc, returning %d", ignoreError);
        if(!ignoreError)
        {
            free(*uploadCommandsP);
            *uploadCommandsP=NULL;
            numberOfUploadCommands=0;
        }
        return numberOfUploadCommands;
        // In this case we can not return an error to SE unless we set some of the earlier errors.
    }

    localCommandsP=tmpCommandsP;
    *uploadCommandsP=localCommandsP;
    memset(&(localCommandsP[numberOfUploadCommands]), 0,sizeof(CommonMessage));

    if(NULL == commandValueP)
    {
        localCommandsP[numberOfUploadCommands++].ret=ROOTPA_ERROR_XML;
        return numberOfUploadCommands;
    }

    localCommandsP[numberOfUploadCommands].ret=ROOTPA_OK;
    uint8_t* containerDataP = NULL;
    int containerLength= base64DecodeStringRemoveEndZero(commandValueP, (char**) &(containerDataP));

    if(0 == containerLength)
    {
        LOGE("handleUploadCommand: base64 decoding failed");
        localCommandsP[numberOfUploadCommands].ret=ROOTPA_ERROR_INTERNAL;
    }

    if(TLT_UPLOAD == commandType)
    {
        localCommandsP[numberOfUploadCommands].ret = uploadTrustlet(containerDataP, containerLength);
    }
    else if (SO_UPLOAD == commandType)
    {
        localCommandsP[numberOfUploadCommands].ret = uploadSo(containerDataP,
                                                              containerLength,
                                                              &localCommandsP[numberOfUploadCommands].intRet);
    }
    else
    {
        LOGE("handleUploadCommand: unknown command type %d this should not have happened", commandType);
        localCommandsP[numberOfUploadCommands].ret=ROOTPA_ERROR_INTERNAL;
    }
    free(containerDataP);
    localCommandsP[numberOfUploadCommands].id=id;
    localCommandsP[numberOfUploadCommands].ignoreError=ignoreError;
    numberOfUploadCommands++;

    LOGD("<<handleUploadCommand %d %lx %lx", numberOfUploadCommands, (long int) uploadCommandsP, (long int) *uploadCommandsP);
    return numberOfUploadCommands;
}

rootpaerror_t handleUploadResponses(uint32_t numberOfUploadResponses, CommonMessage* uploadResponsesP, xmlNodePtr rspResultElementP)
{
    LOGD(">>handleUploadResponses %d", numberOfUploadResponses);
    rootpaerror_t ret=ROOTPA_OK;

    uint32_t i;
    for(i=0; (i < numberOfUploadResponses) && (ROOTPA_OK==ret); i++)
    {
        char* encodedResponseP=NULL;
        if(ROOTPA_OK == uploadResponsesP[i].ret)
        {
            // in success case TLT_UPLOAD and SO_UPLOAD return "0" (encoded) in the resultValue
            // field (discussed and agreed with Dimi Jan 10, 2013 */
            encodedResponseP=base64EncodeAddEndZero("0", 1);
        }

        if( addCommandResultData(rspResultElementP, uploadResponsesP[i].id, encodedResponseP,  uploadResponsesP[i].ret, uploadResponsesP[i].intRet )==false)
        {
            ret=ROOTPA_ERROR_XML;
        }
        free(encodedResponseP);

        LOGD("handleUploadResponses, in loop idx %d ret %d ignore %d", i, uploadResponsesP[i].ret , uploadResponsesP[i].ignoreError);
        if(uploadResponsesP[i].ret != ROOTPA_OK && false == uploadResponsesP[i].ignoreError)
        {
            LOGD("handleUploadResponses, stopping since the current message has error and ignoreError false %d", i);
            break;
        }
    }
    LOGD("<<handleUploadResponses");
    return ret;
}

rootpaerror_t handleCommandAndFillResponse(xmlDocPtr xmlCommandP, xmlDocPtr xmlResponseP)
{
    LOGD(">>handleCommandAndFillResponse");
    rootpaerror_t ret=ROOTPA_OK;
    rootpaerror_t tmpRet=ROOTPA_OK;

    xmlNodePtr rspRootElementP = xmlDocGetRootElement(xmlResponseP);
    if(NULL==rspRootElementP) return ROOTPA_ERROR_XML;

    CmpMessage* cmpCommandsP=NULL;
    CommonMessage* uploadCommandsP=NULL;

    int numberOfCmpCommands=0;
    int numberOfUploadCommands=0;

    commandtype_t commandType=UNKNOWN_TYPE;
    uint32_t id=0;
    char* commandValueP=NULL;
    bool ignoreError=0;
    xmlNodePtr commandNode=NULL;

    // parse command data out of xml, upload commands will also be executed

    while((commandNode=getNextCommand(xmlCommandP, commandNode))!=NULL)
    {
        getValues(commandNode, &commandType, &id, &commandValueP, &ignoreError);
        switch(commandType)
        {
            case CMP:
            {
                numberOfCmpCommands=extractCmpCommand(&cmpCommandsP, numberOfCmpCommands, id, commandValueP, ignoreError);
                if(0==numberOfCmpCommands)
                {
                    ret=ROOTPA_ERROR_OUT_OF_MEMORY;
                }
                break;
            }
            case SO_UPLOAD:
            // intentional fallthrough
            case TLT_UPLOAD:
                numberOfUploadCommands=handleUploadCommand(commandType, &uploadCommandsP, numberOfUploadCommands, id, commandValueP, ignoreError);
                if(0==numberOfUploadCommands)
                {
                    ret=ROOTPA_ERROR_OUT_OF_MEMORY;
                }
                break;
            default:
                LOGE("handleCommandAndFillResponse: received unknown command");
                // we will still work with the other commands in case there are any
                break;
        }
        xmlFree(commandValueP);

        if(ROOTPA_ERROR_OUT_OF_MEMORY == ret) break;

        if(commandType != CMP &&
          false == ignoreError &&
          uploadCommandsP[numberOfUploadCommands-1].ret != ROOTPA_OK) break; // since upload commands are already executed in this loop
    }

    // execute the actual content management protocol commands, if there are any

    CmpMessage* cmpResponsesP=NULL;
    if(cmpCommandsP)
    {
        uint32_t internalError;
        cmpResponsesP=malloc(sizeof(CmpMessage)*numberOfCmpCommands);
        memset(cmpResponsesP, 0, sizeof(CmpMessage)*numberOfCmpCommands);

        if(NULL==cmpResponsesP)
        {
            ret=ROOTPA_ERROR_OUT_OF_MEMORY;
        }
        else
        {
            tmpRet=executeContentManagementCommands(numberOfCmpCommands, cmpCommandsP, cmpResponsesP, &internalError);
            if(ROOTPA_OK!=tmpRet)
            {
                LOGE("call to executeContentManagementCommands failed with %d, continuing anyway", tmpRet);
                // return code from executeContentManagementCommands is here more informative than anything else
                // even in an error case we need to return response to SE, the errors are also included in the
                // actual CMP messages.
                ret=tmpRet;
            }
        }
    }

    // fill response
    if (ret!=ROOTPA_ERROR_OUT_OF_MEMORY)
    {
        xmlNodePtr resultListNodeP=xmlNewChild(rspRootElementP, nameSpace_, BAD_CAST "commandResultList", NULL);
        tmpRet=handleCmpResponses(numberOfCmpCommands, cmpResponsesP, resultListNodeP);
        if(ROOTPA_OK!=tmpRet)
        {
            LOGE("handleCommandAndFillResponse: not able to handle all Cmp responses, still continuing with UploadResponses %d", tmpRet);
            ret=tmpRet;
        }
        tmpRet=handleUploadResponses(numberOfUploadCommands, uploadCommandsP, resultListNodeP);
        if(ROOTPA_OK!=tmpRet)
        {
            LOGE("handleCommandAndFillResponse: not able to handle all Upload responses %d", tmpRet);
            ret=tmpRet;
        }
    }
    // cleanup what has not yet been cleaned

    int i;
    for(i=0; i<numberOfCmpCommands; i++)
    {
        free(cmpCommandsP[i].contentP);
        if(cmpResponsesP) free(cmpResponsesP[i].contentP);
    }
    free(cmpCommandsP);
    free(cmpResponsesP);
    free(uploadCommandsP);

    LOGD("<<handleCommandAndFillResponse %d", ret);
    return ret;
}

void handleError(void* ctx, const char *format, ...)
{
    char *errMsg;
    va_list args;
    va_start(args, format);
    vasprintf(&errMsg, format, args);
    va_end(args);
    LOGW("From libxml2: %s", errMsg);
    free(errMsg);
}

/*
This is for saving the required xml schema files so that the libxml2 code can read it,
to be called only if the files do not exist of can not be parsed
*/

void saveFile(char* filePath, char* fileContent)
{
    LOGD(">>saveFile %s", filePath);
    FILE* fh;

    if ((fh = fopen(filePath, "w")) != NULL)
	{
        fprintf(fh, "%s", fileContent);
        fclose(fh);
	}
    else
    {
        LOGE("saveFiles no handle %s", filePath);
    }
    LOGD("<<saveFile");
}


bool validXmlMessage(xmlDocPtr xmlDocP)
{
    LOGD(">>validXmlMessage %s", enrollmentServiceFullPath_);

    int result=-2;

#ifdef LIBXML_SCHEMAS_ENABLED

    xmlSchemaParserCtxtPtr parserCtxtP = NULL;
    xmlSchemaPtr schemaP = NULL;
    xmlSchemaValidCtxtPtr validCtxtP = NULL;

//    Here we store the schemas if they are not already on "disk". It seems
//    xmlSchemaNewParserCtxt succeeds even if the file does not exists and it is
//    xmlSchemaParse that requires the file to exists. That is why the files are
//    created if schemaP==NULL. Since we are using static library, this can be
//    easily controlled even if there are changes in the behavior

    parserCtxtP = xmlSchemaNewParserCtxt(enrollmentServiceFullPath_);
    schemaP = xmlSchemaParse(parserCtxtP);
    if (!schemaP)
    {
        LOGW("validXmlMessage, no schema ctxt, attempting to save xsd files");
        saveFile(platformTypesFullPath_, PLATFORM_TYPES_XSD);
        saveFile(enrollmentServiceFullPath_, ENROLLMENT_SERVICE_XSD);
        schemaP = xmlSchemaParse(parserCtxtP);
        if (!schemaP){
            LOGE("validXmlMessage, was not able to save xsd files");
            goto cleanup;
        }
    }

    validCtxtP = xmlSchemaNewValidCtxt(schemaP);
    if (!validCtxtP){
        LOGE("validXmlMessage, no validCtxtP");
        goto cleanup;
    }

    result=xmlSchemaValidateDoc(validCtxtP, xmlDocP);

cleanup:

    if (parserCtxtP) xmlSchemaFreeParserCtxt(parserCtxtP);
    if (schemaP) xmlSchemaFree(schemaP);
    if (validCtxtP) xmlSchemaFreeValidCtxt(validCtxtP);
 #else // !LIBXML_SCHEMAS_ENABLED
    result=0;
 #endif // LIBXML_SCHEMAS_ENABLED

    LOGD("<<validXmlMessage %d", result);
    return ((0==result)?true:false);
}

uint8_t* validateDumpAndFree(xmlDocPtr xmlResponseP)
{
    uint8_t*  dumpedP=NULL;
    if(!validXmlMessage(xmlResponseP))
    {
        LOGE("validateDumpAndFree, invalid response");
    }
    int size=0;
    xmlChar* dumpP;
    xmlDocDumpMemory(xmlResponseP, &dumpP, &size);
    if(dumpP!=NULL)
    {
        // doing this copy only because libxml2 documentation tells to
        // release the memory with xmlFree, not free and we want to keep
        // libxml use strictly in this file. It is likely that xmlFree is
        // compatible with free but since I have not verified it, this is to
        // be on the safe side

        dumpedP=malloc(size+1);
        strcpy((char*) dumpedP, (char*) dumpP);
        xmlFree(dumpP);
    }
    xmlFreeDoc(xmlResponseP);

    return dumpedP;
}

// functions used from outside of this file

/**
    in case an error is returned *responseP is set to NULL
*/
rootpaerror_t handleXmlMessage(const char* messageP, const char** responseP)
{
    LOGD(">>handleXmlMessage");
    rootpaerror_t ret=ROOTPA_OK;
    rootpaerror_t tmpRet=ROOTPA_OK;
    *responseP=NULL;

    if (NULL==messageP)
    {
        LOGE("handleXmlMessage, no messageP");
        return ROOTPA_ERROR_ILLEGAL_ARGUMENT;
    }

    xmlSetStructuredErrorFunc(NULL, NULL);
    xmlSetGenericErrorFunc(NULL, handleError);
    xmlThrDefSetStructuredErrorFunc(NULL, NULL);
    xmlThrDefSetGenericErrorFunc(NULL, handleError);

    xmlDocPtr xmlDocP= xmlParseMemory(messageP, strlen(messageP));
    if(NULL==xmlDocP)
    {
        LOGE("handleXmlMessage, can not parse xmlMessageP %s", messageP);
        return ROOTPA_ERROR_XML;
    }

    if(!validXmlMessage(xmlDocP))
    {
        LOGE("handleXmlMessage, invalid message %s", messageP);
        ret=ROOTPA_ERROR_XML;
        // attempting to parse the message anyway.
    }

    xmlDocPtr xmlResponseP=createXmlResponse(ret);

// parse received command

    if(xmlResponseP)
    {
        tmpRet=handleCommandAndFillResponse(xmlDocP, xmlResponseP);
        if(tmpRet!=ROOTPA_OK) ret=tmpRet;
    }
    else
    {
        ret=ROOTPA_ERROR_XML;
    }

    if(xmlResponseP && xmlResponseP->children) // if there is something to return to SE, return it.
    {
        *responseP = (char*)validateDumpAndFree(xmlResponseP);
    }
    else
    {
        if(xmlResponseP) xmlFreeDoc(xmlResponseP);
    }

    if(xmlDocP) xmlFreeDoc(xmlDocP);
    xmlCleanupParser();

    LOGD("<<handleXmlMessage %d %s", ret, ((NULL==*responseP)?"no *responseP":*responseP));
    return ret;
}

rootpaerror_t fillSystemInfo(xmlNodePtr systemInfoNode, const osInfo_t* osSpecificInfoP)
{
    LOGD(">>fillSystemInfo %ld", (long int) osSpecificInfoP);
    if(osSpecificInfoP->imeiEsnP)
    {
        LOGD("imei %s", osSpecificInfoP->imeiEsnP);
        if(xmlNewProp(systemInfoNode, BAD_CAST "imei", BAD_CAST osSpecificInfoP->imeiEsnP)==NULL) return ROOTPA_ERROR_XML;
    }

    if(osSpecificInfoP->mnoP)
    {
        LOGD("mno %s", osSpecificInfoP->mnoP);
        if(xmlNewProp(systemInfoNode, BAD_CAST "mno", BAD_CAST osSpecificInfoP->mnoP)==NULL) return ROOTPA_ERROR_XML;
    }

    if(osSpecificInfoP->brandP)
    {
        LOGD("brand %s", osSpecificInfoP->brandP);
        if(xmlNewProp(systemInfoNode, BAD_CAST "brand", BAD_CAST osSpecificInfoP->brandP)==NULL) return ROOTPA_ERROR_XML;
    }

    if(osSpecificInfoP->manufacturerP)
    {
        LOGD("manufacturer %s", osSpecificInfoP->manufacturerP);
        if(xmlNewProp(systemInfoNode, BAD_CAST "manufacturer", BAD_CAST osSpecificInfoP->manufacturerP)==NULL) return ROOTPA_ERROR_XML;
    }

    if(osSpecificInfoP->hardwareP)
    {
        LOGD("hardware %s", osSpecificInfoP->hardwareP);
        if(xmlNewProp(systemInfoNode, BAD_CAST "hardware", BAD_CAST osSpecificInfoP->hardwareP)==NULL) return ROOTPA_ERROR_XML;
    }

    if(osSpecificInfoP->modelP)
    {
        LOGD("model %s", osSpecificInfoP->modelP);
        if(xmlNewProp(systemInfoNode, BAD_CAST "model", BAD_CAST osSpecificInfoP->modelP)==NULL) return ROOTPA_ERROR_XML;
    }

    if(osSpecificInfoP->versionP)
    {
        LOGD("version %s", osSpecificInfoP->versionP);
        if(xmlNewProp(systemInfoNode, BAD_CAST "version", BAD_CAST osSpecificInfoP->versionP)==NULL) return ROOTPA_ERROR_XML;
    }

    LOGD("<<fillSystemInfo");
    return ROOTPA_OK;
}

rootpaerror_t fillMcVersion(xmlNodePtr mcVersionNode, int mcVersionTag, const mcVersionInfo_t* mcVersionP)
{
    LOGD(">>fillMcVersion");
    char intBuffer[11];

    xmlSetStructuredErrorFunc(NULL, NULL);
    xmlSetGenericErrorFunc(NULL, handleError);
    xmlThrDefSetStructuredErrorFunc(NULL, NULL);
    xmlThrDefSetGenericErrorFunc(NULL, handleError);

    if(xmlNewProp(mcVersionNode, BAD_CAST "productId", BAD_CAST mcVersionP->productId)==NULL) return ROOTPA_ERROR_XML;

    sprintf(intBuffer,"%u",mcVersionP->versionMci);
    if(xmlNewProp(mcVersionNode, BAD_CAST "versionMci", BAD_CAST intBuffer)==NULL) return ROOTPA_ERROR_XML;

    sprintf(intBuffer,"%u",mcVersionP->versionSo);
    if(xmlNewProp(mcVersionNode, BAD_CAST "versionSo", BAD_CAST intBuffer)==NULL) return ROOTPA_ERROR_XML;

    sprintf(intBuffer,"%u",mcVersionP->versionMclf);
    if(xmlNewProp(mcVersionNode, BAD_CAST "versionMclf", BAD_CAST intBuffer)==NULL) return ROOTPA_ERROR_XML;

    sprintf(intBuffer,"%u",mcVersionP->versionContainer);
    if(xmlNewProp(mcVersionNode, BAD_CAST "versionContainer", BAD_CAST intBuffer)==NULL) return ROOTPA_ERROR_XML;

    sprintf(intBuffer,"%u",mcVersionP->versionMcConfig);
    if(xmlNewProp(mcVersionNode, BAD_CAST "versionMcConfig", BAD_CAST intBuffer)==NULL) return ROOTPA_ERROR_XML;

    sprintf(intBuffer,"%u",mcVersionP->versionTlApi);
    if(xmlNewProp(mcVersionNode, BAD_CAST "versionTlApi", BAD_CAST intBuffer)==NULL) return ROOTPA_ERROR_XML;

    sprintf(intBuffer,"%u",mcVersionP->versionDrApi);
    if(xmlNewProp(mcVersionNode, BAD_CAST "versionDrApi", BAD_CAST intBuffer)==NULL) return ROOTPA_ERROR_XML;

    sprintf(intBuffer,"%u",mcVersionP->versionCmp);
    if(xmlNewProp(mcVersionNode, BAD_CAST "versionCmp", BAD_CAST intBuffer)==NULL) return ROOTPA_ERROR_XML;

    LOGD("<<fillMcVersion");
    return ROOTPA_OK;
}

rootpaerror_t buildXmlTrustletInstallationRequest(const char** responseP, trustletInstallationData_t data )
{
    LOGD(">>buildXmlTrustletInstallationRequest %ld (%ld %d %d)", (long int) responseP, (long int) data.dataP, data.dataLength, data.dataType);
    rootpaerror_t ret=ROOTPA_OK;
    if(NULL ==  responseP) return ROOTPA_ERROR_ILLEGAL_ARGUMENT; // data content checked earlier in commandhandler.c

    xmlDocPtr xmlResponseDocP=createXmlResponse(ret);
    xmlNodePtr rspRootElementP = xmlDocGetRootElement(xmlResponseDocP);
    if(NULL==rspRootElementP) return ROOTPA_ERROR_XML;

    xmlNodePtr systemInfoNode=xmlNewChild(rspRootElementP, nameSpace_, BAD_CAST "tltInstallationRequest", NULL);
    if(NULL==systemInfoNode) return ROOTPA_ERROR_XML;

    xmlNodePtr mcDataNode=NULL;
    char* encodedDataP=base64EncodeAddEndZero((char*) data.dataP, data.dataLength);
    if(NULL==encodedDataP)
    {
        LOGE("buildXmlTrustletInstallationRequest: base64 encoding failed");
        return ROOTPA_ERROR_INTERNAL;
    }

    if(data.dataType == REQUEST_DATA_TLT)
    {
        mcDataNode = xmlNewChild(systemInfoNode, nameSpace_, BAD_CAST "tltBinary", BAD_CAST encodedDataP);
    }
    else
    {
        mcDataNode = xmlNewChild(systemInfoNode, nameSpace_, BAD_CAST "tltEncryptionKey", BAD_CAST encodedDataP);
    }
    free(encodedDataP);
    if(NULL==mcDataNode) return ROOTPA_ERROR_XML;

    if(ROOTPA_OK==ret)
    {
        *responseP=(char*)validateDumpAndFree(xmlResponseDocP);
    }

    xmlCleanupParser();
    return ret;
    LOGD("<<buildXmlTrustletInstallationRequest");
}

/**
in case an error is returned *responseP is set to NULL
*/
rootpaerror_t buildXmlSystemInfo(const char** responseP, int mcVersionTag, const mcVersionInfo_t* mcVersionP, const osInfo_t* osSpecificInfoP)
{
    LOGD(">>buildXmlSystemInfo %ld %ld %ld", ( long int ) responseP, ( long int ) mcVersionP, ( long int ) osSpecificInfoP);
    rootpaerror_t ret=ROOTPA_OK;
    if(NULL == responseP || NULL == mcVersionP || NULL == osSpecificInfoP) return ROOTPA_ERROR_INTERNAL;

    xmlSetStructuredErrorFunc(NULL, NULL);
    xmlSetGenericErrorFunc(NULL, handleError);
    xmlThrDefSetStructuredErrorFunc(NULL, NULL);
    xmlThrDefSetGenericErrorFunc(NULL, handleError);

    xmlDocPtr xmlResponseDocP=createXmlResponse(ret);
    xmlNodePtr rspRootElementP = xmlDocGetRootElement(xmlResponseDocP);
    if(NULL==rspRootElementP) return ROOTPA_ERROR_XML;

    xmlNodePtr systemInfoNode=xmlNewChild(rspRootElementP, nameSpace_, BAD_CAST "systemInformation", NULL);
    if(NULL==systemInfoNode) return ROOTPA_ERROR_XML;

    xmlNodePtr mcVersionNode=xmlNewChild(systemInfoNode, typesNameSpace_, BAD_CAST "mcVersion", NULL);
    if(NULL==mcVersionNode) return ROOTPA_ERROR_XML;

    ret=fillSystemInfo(systemInfoNode, osSpecificInfoP);
    if(ROOTPA_OK!=ret)
    {
        LOGE("buildXmlSystemInfo: could not fill system info %d, continuing anyway", ret);
    }

    ret=fillMcVersion(mcVersionNode, mcVersionTag, mcVersionP);
    if(ROOTPA_OK!=ret)
    {
        LOGE("buildXmlSystemInfo: could not fill Mc version %d, continuing anyway", ret);
    }


    if(ROOTPA_OK==ret)
    {
        *responseP=(char*)validateDumpAndFree(xmlResponseDocP);
    }

    xmlCleanupParser();
    return ret;
    LOGD("<<buildXmlSystemInfo");
}

/**
set the path where to look for and store the xsd files
*/
void setXsdPaths(const char* xsdpathP)
{
    memset(enrollmentServiceFullPath_, 0, XSD_PATH_MAX_LEN);
    memset(platformTypesFullPath_, 0, XSD_PATH_MAX_LEN);

    if (xsdpathP!=NULL && strlen(xsdpathP)+1+sizeof(ENROLLMENT_SERVICE_XSD_NAME)<XSD_PATH_MAX_LEN) // ENROLLMENT_SERVICE_XSD_NAME is the longer of the two
    {
        strcpy(enrollmentServiceFullPath_, xsdpathP);
        strcpy(platformTypesFullPath_, xsdpathP);

        strcat(enrollmentServiceFullPath_, "/");
        strcat(platformTypesFullPath_, "/");
    }

    strcat(enrollmentServiceFullPath_, ENROLLMENT_SERVICE_XSD_NAME);
    strcat(platformTypesFullPath_, PLATFORM_TYPES_XSD_NAME);
}
