//
//  RealtekRTL8111Client.cpp
//  RealtekRTL8111
//
//  Created by Andy Vandijck on 28/12/14.
//  Copyright (c) 2014 Laura Müller. All rights reserved.
//

#include "RealtekRTL8111Client.h"

#undef  super
#define super IOUserClient
OSDefineMetaClassAndStructors(RTL8111Client, IOUserClient);

const RTL8111Client *RTL8111Client::withTask(task_t owningTask)
{
    RTL8111Client *client;
    
    client = new RTL8111Client;
    if (client != NULL)
    {
        if (client->init() == false)
        {
            client->release();
            client = NULL;
        }
    }
    if (client != NULL)
    {
        client->fTask = owningTask;
    }
    return (client);
}


bool RTL8111Client::set_Q_Size(UInt32 capacity)
{
    bool res;
    
    if (capacity == 0)
    {
        return true;
    }
    
#ifdef  DEBUG
    IOLog("AnVMSR: Reseting size of data queue, all data in queue is lost");
#endif
    
    myLogQueue->release();
    
    //Get mem for new queue of calcuated size
    myLogQueue = new IOSharedDataQueue;
    if (myLogQueue == 0)
    {
        IOLog("AnVMSR: [ERR]  Failed to allocate memory for buffer\n");
        Q_Err++;
        return false;
    }
    
    res = myLogQueue->initWithCapacity(capacity);
    if (res == false)
    {
        IOLog("AnVMSR: [ERR] Could not initWithEntries\n");
        Q_Err++;
        return false;
    }
    
    return true;
}

void RTL8111Client::messageHandler(UInt32 type, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

bool RTL8111Client::initWithTask(task_t owningTask, void *securityID, UInt32 type,
                                    OSDictionary *properties)
{
    return super::initWithTask(owningTask, securityID, type, properties);
}

bool RTL8111Client::start(IOService *provider)
{
    if (!super::start(provider))
        return false;
    
    mDevice = OSDynamicCast(RTL8111, provider);
    mDevice->retain();
    
    return true;
}

bool RTL8111Client::willTerminate(IOService *provider, IOOptionBits options)
{
    return super::willTerminate(provider, options);
}

bool RTL8111Client::didTerminate(IOService *provider, IOOptionBits options, bool *defer)
{
    // if defer is true, stop will not be called on the user client
    *defer = false;
    
    return super::didTerminate(provider, options, defer);
}

bool RTL8111Client::terminate(IOOptionBits options)
{
    return super::terminate(options);
}

// clientClose is called when the user process calls IOServiceClose
IOReturn RTL8111Client::clientClose()
{
    if (mDevice != NULL)
    {
        mDevice->closeChild(this);
    }
    
    if (!isInactive())
        terminate();
    
    return kIOReturnSuccess;
}

// clientDied is called when the user process terminates unexpectedly, the default
// implementation simply calls clientClose
IOReturn RTL8111Client::clientDied()
{
    return clientClose();
}

void RTL8111Client::free(void)
{
    mDevice->release();
    
    super::free();
}

// stop will be called during the termination process, and should free all resources
// associated with this client
void RTL8111Client::stop(IOService *provider)
{
    super::stop(provider);
}

// getTargetAndMethodForIndex looks up the external methods - supply a description of the parameters
// available to be called
IOExternalMethod * RTL8111Client::getTargetAndMethodForIndex(IOService **target, UInt32 index)
{
    static const IOExternalMethod methodDescs[3] = {
        { NULL, (IOMethod) &RTL8111Client::actionMethodDefaultMAC, kIOUCStructIStructO,
            kIOUCVariableStructureSize, kIOUCVariableStructureSize },
        { NULL, (IOMethod) &RTL8111Client::actionMethodSetMAC, kIOUCStructIStructO,
            kIOUCVariableStructureSize, kIOUCVariableStructureSize },
    };
    
    *target = this;
    if (index < 3)
        return (IOExternalMethod *) (methodDescs + index);
    
    return NULL;
}

IOReturn RTL8111Client::actionMethodDefaultMAC(UInt32 *dataIn, UInt32 *dataOut, IOByteCount inputSize,
                                             IOByteCount *outputSize)
{
    IOEthernetAddress defaultmac;
    
    DebugLog("RTL8111Client: Set default MAC address\n", msrdata->msr, msrdata->param);

    defaultmac = mDevice->GetOrigMACAddr();

    mDevice->IOEthernetController::setPromiscuousMode(false);
    mDevice->setHardwareAddress(&defaultmac);
    mDevice->SetIsCustomMACAddr(false);

    return kIOReturnSuccess;
}

IOReturn RTL8111Client::actionMethodSetMAC(UInt32 *dataIn, UInt32 *dataOut, IOByteCount inputSize,
                                             IOByteCount *outputSize)
{
    inout * macdata = (inout *)dataIn;
    IOEthernetAddress MACAddr;

    DebugLog("RTL8111Client: Set MAC address called\n");
    
    if (!dataIn)
    {
        return kIOReturnUnsupported;
    }

    bcopy(macdata->macaddr, MACAddr.bytes, sizeof(MACAddr.bytes));

    mDevice->setPromiscuousMode(true);
    mDevice->setHardwareAddress(&MACAddr);
    mDevice->SetIsCustomMACAddr(true);

    return kIOReturnSuccess;
}

IOReturn RTL8111Client::clientMemoryForType(UInt32 type, IOOptionBits *options,
                                               IOMemoryDescriptor **memory)
{
    IOBufferMemoryDescriptor *memDesc;
    char *msgBuffer;
    
    *options = 0;
    *memory = NULL;
    
    memDesc = IOBufferMemoryDescriptor::withOptions(kIOMemoryKernelUserShared, mDevice->mPrefPanelMemoryBufSize);
    
    if (!memDesc)
    {
        return kIOReturnUnsupported;
    }
    
    msgBuffer = (char *) memDesc->getBytesNoCopy();
    bcopy(mDevice->mPrefPanelMemoryBuf, msgBuffer, mDevice->mPrefPanelMemoryBufSize);
    *memory = memDesc; // automatically released after memory is mapped into task
    
    return(kIOReturnSuccess);
}
