//
//  RealtekRTL8111Client.h
//  RealtekRTL8111
//
//  Created by Andy Vandijck on 28/12/14.
//  Copyright (c) 2014 Laura Müller. All rights reserved.
//

#ifndef RealtekRTL8111UserclientH
#define RealtekRTL8111UserclientH

#include <mach/mach_types.h>
#include <mach/machine.h>
#include <pexpert/pexpert.h>
#include <string.h>
#include <IOKit/IOLib.h>
#include <IOKit/IOService.h>
#include <IOKit/IOUserClient.h>
#include <IOKit/IOBufferMemoryDescriptor.h>
#include <IOKit/IOSharedDataQueue.h>

#include "RealtekRTL8111.h"

#define BUFSIZE 	512 	//bytes
#define MAXENTRIES	500
#define MAXUSERS 	5

#define kMethodObjectUserClient ((IOService*)0)

enum {
    DefaultMACAddr = 0,
    SetMACAddr = 1,
    AnVMSRNumMethods
};

typedef struct {
    UInt32 action;
    UInt8 macaddr[kIOEthernetAddressSize];
} inout;

class RTL8111;

class RTL8111Client : public IOUserClient
{
    OSDeclareDefaultStructors(RTL8111Client);
    
private:
    RTL8111 *mDevice;
    
public:
    void messageHandler(UInt32 type, const char *format, ...) __attribute__ ((format (printf, 3, 4)));

    static const RTL8111Client *withTask(task_t owningTask);
    
    virtual void free();
    virtual bool start(IOService *provider);
    virtual void stop(IOService *provider);
    
    virtual bool initWithTask(task_t owningTask, void *securityID, UInt32 type, OSDictionary *properties);
    virtual IOReturn clientClose();
    virtual IOReturn clientDied();
    virtual bool set_Q_Size(UInt32 capacity);
    
    virtual bool willTerminate(IOService *provider, IOOptionBits options);
    virtual bool didTerminate(IOService *provider, IOOptionBits options, bool *defer);
    virtual bool terminate(IOOptionBits options = 0);
    
    virtual IOExternalMethod *getTargetAndMethodForIndex(IOService **targetP, UInt32 index);
    
    virtual IOReturn clientMemoryForType(UInt32 type, IOOptionBits *options, IOMemoryDescriptor **memory);
    
    virtual IOReturn actionMethodDefaultMAC(UInt32 *dataIn, UInt32 *dataOut, IOByteCount inputSize, IOByteCount *outputSize);
    virtual IOReturn actionMethodSetMAC(UInt32 *dataIn, UInt32 *dataOut, IOByteCount inputSize, IOByteCount *outputSize);
    
    task_t fTask;
    IOSharedDataQueue * myLogQueue;
    int Q_Err;
};

#endif /* RealtekRTL8111UserclientH */
