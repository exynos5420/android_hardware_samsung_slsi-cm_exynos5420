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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>


#include "rootpaErrors.h"
#include "logging.h"
#include "pacmp3.h"  // for setCallbackP
#include "seclient.h"
#include "xmlmessagehandler.h"
#include "provisioningengine.h"



static const char* const SE_URL="https://se.cgbe.trustonic.com:8443/"; // note that there has to be slash at the end since we are adding suid to it next
//static const char* const SE_URL="http://se.cgbe.trustonic.com:8080/"; // note that there has to be slash at the end since we are adding suid to it next

static const char* const RELATION_SELF  =      "relation/self";
static const char* const RELATION_SYSTEMINFO = "relation/system_info";
static const char* const RELATION_RESULT   =   "relation/command_result";
static const char* const RELATION_NEXT    =    "relation/next";
static const uint8_t* const SLASH="/";

static const char* const RELATION_INITIAL_POST="initial_post"; // this will make us to send HTTP GET, which
                                      // is the right thing to do since we do not
                                      // have any data to send to SE, this will need to be different in RootPA initiated trustet installation
static const char* const RELATION_INITIAL_DELETE="initial_delete"; // this will make us to send HTTP DELETE

#define INITIAL_URL_BUFFER_LENGTH 255

static char initialUrl_[INITIAL_URL_BUFFER_LENGTH];
static CallbackFunctionP callbackP_=NULL;

void addSlashToUri(char* uriP)
{
    LOGD(">>addSlashToUri");
    int uriidx=strlen(uriP);
    uriP[uriidx]='/';
    LOGD("<<addSlashToUri %s", uriP);
}

void addBytesToUri(char* uriP, uint8_t* bytes, uint32_t length, bool uuid )
{
    LOGD(">>add bytes to URI %d", length);
    int uriidx=strlen(uriP);
    int i;
    uint8_t singleNumber=0;
    for(i=0; i<length; i++)
    {
        singleNumber=(bytes[i]>>4);
        singleNumber=((singleNumber<0xA)?(singleNumber+0x30):(singleNumber+0x57));
        uriP[uriidx++]=singleNumber;
        singleNumber=(bytes[i]&0x0F);
        singleNumber=((singleNumber<0xA)?(singleNumber+0x30):(singleNumber+0x57));
        uriP[uriidx++]=singleNumber;
        if(true==uuid && (3 == i || 5 == i || 7 == i || 9 == i))
        {
            uriP[uriidx++]='-';
        }
    }
    LOGD("<<add bytes to URI %s %d", uriP, uriidx);
}

void addIntToUri(char* uriP, uint32_t addThis)
{
    char intInString[10];
    memset(intInString, 0, 10);
    // using signed integer since this is how SE wants it
    sprintf(intInString, "/%d", addThis);
    strcpy((uriP+strlen(uriP)), intInString);
    LOGD("add int to URI %s %d", uriP, addThis);
}

void cleanup(char** linkP, char** relP, char** commandP)
{
    if(commandP!=NULL)
    {
        free(*commandP);
        *commandP=NULL;
    }

    if(relP!=NULL)
    {
        if((*relP!=RELATION_INITIAL_POST) &&
           (*relP!=RELATION_INITIAL_DELETE)) free(*relP);
        *relP=NULL;
    }

    if(linkP!=NULL)
    {
        free(*linkP);
        *linkP=NULL;
    }
}

rootpaerror_t setInitialAddress(const char* addrP, uint32_t length)
{
    if(NULL==addrP || 0==length)
    {
        return ROOTPA_ERROR_INTERNAL;
    }

    if(INITIAL_URL_BUFFER_LENGTH < (length + 1))
    {
        return ROOTPA_ERROR_ILLEGAL_ARGUMENT;
    }
    memset(initialUrl_, 0, INITIAL_URL_BUFFER_LENGTH);
    memcpy(initialUrl_, addrP, length);
    return ROOTPA_OK;
}

bool empty(const char* zeroTerminatedArray)
{
    return(strlen(zeroTerminatedArray)==0);
}

char* createBasicLink(mcSuid_t suid)
{
    char* tmpLinkP=NULL;
    size_t urlLength=0;

    urlLength=strlen(initialUrl_) + (sizeof(mcSuid_t)*2) + (sizeof(mcSpid_t)*2) + (sizeof(mcUuid_t)*2)+6; //possible slash and end zero and four dashes
    tmpLinkP=malloc(urlLength);
    memset(tmpLinkP,0,urlLength);
    strcpy(tmpLinkP, initialUrl_);
    addBytesToUri(tmpLinkP, (uint8_t*) &suid, sizeof(suid), false);
    return tmpLinkP;
}

void doProvisioningWithSe(
    mcSpid_t spid,
    mcSuid_t suid,
    CallbackFunctionP callbackP,
    SystemInfoCallbackFunctionP getSysInfoP,
    GetVersionFunctionP getVersionP,
    initialRel_t initialRel,
    trustletInstallationData_t* tltDataP)
{
    LOGD(">>doProvisioningWithSe");

    rootpaerror_t ret=ROOTPA_OK;
    rootpaerror_t tmpRet=ROOTPA_OK;
    bool workToDo = true;
    const char* linkP=NULL;
    const char* relP=NULL;
    const char* pendingLinkP=NULL;
    const char* pendingRelP=NULL;
    const char* commandP=NULL;  // "command" received from SE
    const char* responseP=NULL; // "response" to be sent to SE

    const char* usedLinkP=NULL;
    const char* usedRelP=NULL;
    const char* usedCommandP=NULL;

    callbackP_=callbackP;
    if(NULL==callbackP)
    {
        LOGE("No callbackP, can not respond to caller, this should not happen!");
    }

    if(empty(initialUrl_))
    {
        memset(initialUrl_, 0, INITIAL_URL_BUFFER_LENGTH);
        strncpy(initialUrl_, SE_URL, strlen(SE_URL));
    }

    linkP=createBasicLink(suid);

    if (initialRel == initialRel_DELETE)
    {
	relP = RELATION_INITIAL_DELETE;
    }
    else
    {
	relP = RELATION_INITIAL_POST;
        if(spid!=0) // SPID 0 is not legal. We use it for requesting root container creation only (no sp)
        {
            addIntToUri((char*)linkP, (uint32_t) spid);
        }
    }

    LOGD("calling first callback %ld", (long int) callbackP);
    callbackP(CONNECTING_SERVICE_ENABLER, ROOTPA_OK, NULL);

    ret=openSeClientAndInit();
    if(ROOTPA_OK!=ret)
    {
        callbackP(ERROR, ret, NULL);
        workToDo=false;
    }

    if(tltDataP != NULL) // we are installing trustlet
    {
        ret=buildXmlTrustletInstallationRequest(&responseP, *tltDataP );
        if(ROOTPA_OK!=ret || NULL==responseP)
        {
            if(ROOTPA_OK==ret) ret=ROOTPA_ERROR_XML;
            callbackP(ERROR, ret, NULL);
            workToDo=false;
        }
        else
        {
            addSlashToUri((char*) linkP);
            addBytesToUri((char*) linkP, (uint8_t*) tltDataP->uuid.value, UUID_LENGTH, true);
        }
    }

// recovery from factory reset
    if(factoryResetAssumed() && relP != RELATION_INITIAL_DELETE)
    {
        pendingLinkP=linkP;
        pendingRelP=relP;
        relP=RELATION_INITIAL_DELETE;
        linkP=createBasicLink(suid);
    }
// recovery from factory reset

    while(workToDo)
    {
        LOGD("in loop link: %s\nrel: %s\ncommand: %s\nresponse: %s\n", (linkP==NULL)?"null":linkP,
                                                                       (relP==NULL)?"null":relP,
                                                                       (commandP==NULL)?"null":commandP,
                                                                       (responseP==NULL)?"null":responseP);

        if(NULL==relP)
        {
// recovery from factory reset
            if(pendingLinkP!=NULL && pendingRelP!=NULL)
            {
                free((char*)linkP);
                linkP=pendingLinkP;
                relP=pendingRelP;
                pendingLinkP=NULL;
                pendingRelP=NULL;
                workToDo=true;
                continue;
            }
// recovery from factory reset


            callbackP(FINISHED_PROVISIONING, ROOTPA_OK, NULL); // this is the only place where we can be sure
                                                         // SE does not want to send any more data to us
                                                         // the other option would be to keep track on the
                                                         // commands received from SE but since we want
                                                         // SE to have option to execute also other commands
                                                         // and also allow modification in provisioning sequence
                                                         // without modifying RootPA we use this simpler way.
            workToDo=false;
        }
        else if(strstr(relP, RELATION_SELF))  // do it again. So we need to restore pointer to previous stuff.
        {
            cleanup((char**) &linkP, (char**) &relP, (char**) &commandP);

            relP=usedRelP;
            linkP=usedLinkP;
            commandP=usedCommandP;
        }
        else
        {
            // store the current pointers to "used" pointers just before using them
            usedLinkP=linkP;            // originally linkP
            usedRelP=relP;              // originally NULL
            usedCommandP=commandP;      // originally NULL

            if(strstr(relP, RELATION_SYSTEMINFO))
            {
                osInfo_t osSpecificInfo;
                int mcVersionTag=0;
                mcVersionInfo_t mcVersion;

                tmpRet=getSysInfoP(&osSpecificInfo);
                if(tmpRet!=ROOTPA_OK) ret=tmpRet;

                tmpRet=getVersionP(&mcVersionTag, &mcVersion);
                if(tmpRet!=ROOTPA_OK) ret=tmpRet;

                tmpRet=buildXmlSystemInfo(&responseP, mcVersionTag, &mcVersion, &osSpecificInfo);
                if(tmpRet!=ROOTPA_OK) ret=tmpRet;

                free(osSpecificInfo.imeiEsnP);
                free(osSpecificInfo.mnoP);
                free(osSpecificInfo.brandP);
                free(osSpecificInfo.manufacturerP);
                free(osSpecificInfo.hardwareP);
                free(osSpecificInfo.modelP);
                free(osSpecificInfo.versionP);

                tmpRet=httpPutAndReceiveCommand(responseP, &linkP, &relP, &commandP);
                if(tmpRet!=ROOTPA_OK) ret=tmpRet;
                if(ret!=ROOTPA_OK)
                {
                    LOGE("getSysInfoP, getVersionP or buildXmlSystemInfo or httpPutAndReceiveCommand returned an error %d", ret);
                    callbackP(ERROR, ret, NULL);
                    if(tmpRet!=ROOTPA_OK) workToDo=false; // if sending response succeeded, we rely on "relP" to tell whether we should continue or not
                }
            }
            else if(strstr(relP, RELATION_INITIAL_DELETE))
            {
                ret=httpDeleteAndReceiveCommand(&linkP, &relP, &commandP);

                if(ret!=ROOTPA_OK)
                {
                    LOGE("httpDeleteAndReceiveCommand returned an error %d", ret);
                    callbackP(ERROR, ret, NULL);
                    workToDo=false;
                }
            }
            else if(strstr(relP, RELATION_INITIAL_POST))
            {
                // response may be NULL or trustlet installation request
                ret=httpPostAndReceiveCommand(responseP, &linkP, &relP, &commandP);

                if(ret!=ROOTPA_OK)
                {
                    LOGE("httpPostAndReceiveCommand returned an error %d", ret);
                    callbackP(ERROR, ret, NULL);
                    workToDo=false;
                }
            }
            else if(strstr(relP, RELATION_RESULT))
            {
                setCallbackP(callbackP);
                ret=handleXmlMessage(commandP, &responseP);
                setCallbackP(NULL);

                if(NULL==responseP)
                {
                    if(ROOTPA_OK==ret) ret=ROOTPA_ERROR_XML;
                    // have to set these to NULL since we are not even trying to get them from SE now
                    linkP=NULL;
                    relP=NULL;
                    commandP=NULL;
                    LOGE("no responseP");
                }
                else
                {
                    // attempting to return response to SE even if there was something wrong in handleXmlMessage
                    tmpRet=httpPostAndReceiveCommand(responseP, &linkP, &relP, &commandP);
                    if(tmpRet!=ROOTPA_OK) ret=tmpRet;
                }

                if(ret!=ROOTPA_OK && ret!=ROOTPA_ERROR_REGISTRY_OBJECT_NOT_AVAILABLE) // if container is not found, not sending error intent to SP.PA since it is possible that SE can recover.
                {                                                                     // If it can not, it will return an error code anyway.
                    LOGE("httpPostAndReceiveCommand or handleXmlMessage returned an error %d %d", ret, tmpRet);
                    callbackP(ERROR, ret, NULL);
                    if(tmpRet!=ROOTPA_OK) workToDo=false; // if sending response succeeded, we rely on "relP" to tell whether we should continue or not
                }

            }
            else if(strstr(relP, RELATION_NEXT))
            {
                ret=httpGetAndReceiveCommand(&linkP, &relP, &commandP);
                if(ret!=ROOTPA_OK)
                {
                    LOGE("httpGetAndReceiveCommand returned an error %d", ret);
                    callbackP(ERROR, ret, NULL);
                    workToDo=false;
                }
            }
            else
            {
                LOGE("DO NOT UNDERSTAND REL %s", relP);
                ret=ROOTPA_ERROR_ILLEGAL_ARGUMENT;
                callbackP(ERROR, ret, NULL);
                workToDo=false;
            }

            LOGD("end of provisioning loop work to do: %d, responseP %ld", workToDo, responseP);
        }

        // last round cleaning in order to make sure both original and user pointers are released, but only once

        if(!workToDo)
        {
            LOGD("no more work to do %ld - %ld %ld - %ld %ld - %ld", (long int) linkP, (long int) usedLinkP,
                                                                     (long int) relP, (long int) usedRelP,
                                                                     (long int) commandP, (long int) usedCommandP);
            // final cleanup

            // ensure that we do not clean up twice in case used pointers opint to the original one
            if(linkP==usedLinkP) usedLinkP=NULL;
            if(relP==usedRelP) usedRelP=NULL;
            if(commandP==usedCommandP) usedCommandP=NULL;

            cleanup((char**) &linkP, (char**) &relP, (char**) &commandP);
        }

        // free the used pointers since all the necessary pointers point to new direction.
        // when relation is self we need to give the previous command again and so we keep the
        // data

        if(relP==NULL || strstr(relP, RELATION_SELF)==NULL)
        {
            cleanup((char**) &usedLinkP, (char**) &usedRelP, (char**) &usedCommandP);
        }

        // responseP can be freed at every round
        free((char*)responseP);
        responseP=NULL;

    } // while
    closeSeClientAndCleanup();


    if(ROOTPA_OK != ret)  LOGE("doProvisioningWithSe had some problems: %d",ret );
    LOGD("<<doProvisioningWithSe ");
    return;
}

rootpaerror_t uploadTrustlet(uint8_t* containerDataP, uint32_t containerLength)
{
    if(callbackP_)
    {
        tltInfo_t tltInfo;
        tltInfo.trustletP = containerDataP;
        tltInfo.trustletSize = containerLength;
        callbackP_(PROVISIONING_STATE_INSTALL_TRUSTLET, ROOTPA_OK, &tltInfo);
        return ROOTPA_OK;
    }
    LOGE("uploadTrustlet, no callbackP_");
    return ROOTPA_COMMAND_NOT_SUPPORTED;
}
