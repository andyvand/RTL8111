//
//  RTL8111Pref.m
//

#import "RTL8111Pref.h"

@implementation RTL8111Pref

//
//  main.m
//  anvmsr
//
//  Created by Andy Vandijck on 12/09/13.
//  Copyright (c) 2013 Andy Vandijck. All rights reserved.
//

void dialog (const char *message)
{
    NSAlert *alert = [[NSAlert alloc] init];
 
    [alert setInformativeText:[NSString stringWithUTF8String:message]];
    [alert setMessageText:@"RTL8111"];
    [alert addButtonWithTitle:@"OK"];
    [alert setAlertStyle:NSWarningAlertStyle];

    [alert runModal];
    [alert release];
}

io_service_t getService()
{
    io_service_t service = 0;
    mach_port_t masterPort;
    io_iterator_t iter;
    kern_return_t ret;
    io_string_t path;

    ret = IOMasterPort(MACH_PORT_NULL, &masterPort);
    if (ret != KERN_SUCCESS) {
        dialog("Can't get masterport\n");
        goto failure;
    }
    
    ret = IOServiceGetMatchingServices(masterPort, IOServiceMatching(kRTL8111ClassName), &iter);
    if (ret != KERN_SUCCESS) {
        dialog("RTL8111 is not running\n");
        goto failure;
    }
    
    service = IOIteratorNext(iter);
    IOObjectRelease(iter);
    
    ret = IORegistryEntryGetPath(service, kIOServicePlane, path);
    if (ret != KERN_SUCCESS) {
        dialog("Can't get registry-entry path\n");
        goto failure;
    }
    
failure:
    return service;
}

long long hex2int(const char *s)
{
    char charset[16] = "0123456789abcdef";
    char altcharset[16] = "0123456789ABCDEF";
    long long i = (int)strlen(s), len = i, j = 0;
    long long num = 0;
    while (i >= 0) {
        for (j = 0; j < 16; j++) {
            if ((charset[j] == s[i]) || (altcharset[j] == s[i])) {
                num += j * (int)pow(16, len-i-1);
                break;
            }
        }
        i--;
    }
    return num;
}

- (void) setMACAddr:(id)sender
{
    const char *macValue = [[MACAddrVal stringValue] cStringUsingEncoding:NSUTF8StringEncoding];
    unsigned long long macaddress = 0;
    unsigned long long macaddress2 = 0;
    unsigned int i = 0;
    char uppermac[5];
    char string[255];

    if (strlen(macValue) != 12)
    {
        dialog("Please enter a MAC address in the form of 1A3B3C4D5E6F (12 hex digits)");

        return;
    }

    while (i < strlen(macValue))
    {
        if ((macValue[i] <= '0') && (macValue[i] >= '9') &&
            (macValue[i] <= 'a') && (macValue[i] >= 'z') &&
            (macValue[i] <= 'A') && (macValue[i] >= 'Z'))
        {
            dialog("Please enter a MAC address in the form of 1A3B3C4D5E6F (12 hex digits)");

            return;
        }

        ++i;
    }

    uppermac[0] = macValue[0];
    uppermac[1] = macValue[1];
    uppermac[2] = macValue[2];
    uppermac[3] = macValue[3];
    uppermac[4] = 0x00;

    macaddress = hex2int(macValue);
    macaddress2 = hex2int(uppermac);

    service = getService();
    if(!service)
        goto failure;
    kern_return_t ret;
    io_connect_t connect = 0;
    ret = IOServiceOpen(service, mach_task_self(), 0, &connect);
    if (ret != KERN_SUCCESS)
    {
        dialog("Couldn't open IO Service\n");
        goto failure;
    }

    inout in;
    //size_t outsize = sizeof(in);
    
    in.action = SetMACAddr;

    memset(in.macaddr, 0, sizeof(in.macaddr));
    in.macaddr[5] = (UInt8)(macaddress & 0xFF);
    in.macaddr[4] = (UInt8)((macaddress & 0xFF00) >> 8);
    in.macaddr[3] = (UInt8)((macaddress & 0xFF0000) >> 16);
    in.macaddr[2] = (UInt8)((macaddress & 0xFF000000) >> 24);
    in.macaddr[1] = (UInt8)(macaddress2 & 0xFF);
    in.macaddr[0] = (UInt8)((macaddress2 & 0xFF00) >> 8);

#if MAC_OS_X_VERSION_MIN_REQUIRED <= MAC_OS_X_VERSION_10_4
    ret = IOConnectMethodStructureIStructureO(connect, SetMACAddr,
                                              sizeof(in),			/* structureInputSize */
                                              NULL,    /* structureOutputSize */
                                              &in,        /* inputStructure */
                                              NULL);       /* ouputStructure */
#else
    ret = IOConnectCallStructMethod(connect,
                                    SetMACAddr,
                                    &in,
                                    sizeof(in),
                                    NULL,
                                    NULL
                                    );
#endif
    
    if (ret != KERN_SUCCESS)
    {
        dialog("Can't connect to StructMethod to send commands\n");
    } else {
        snprintf(string, sizeof(string), "Set new MAC address: %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n", in.macaddr[0], in.macaddr[1], in.macaddr[2], in.macaddr[3], in.macaddr[4], in.macaddr[5]);
        dialog(string);
    }

failure:
    if(connect)
    {
        ret = IOServiceClose(connect);
        if (ret != KERN_SUCCESS)
        {
            dialog("IOServiceClose failed\n");
        }
    }
    
    if(service)
        IOObjectRelease(service);
}

- (void) setOrigMACAddr:(id)sender
{
    service = getService();
    if(!service)
        goto failure;
    kern_return_t ret;
    io_connect_t connect = 0;
    ret = IOServiceOpen(service, mach_task_self(), 0, &connect);
    if (ret != KERN_SUCCESS)
    {
        dialog("Couldn't open IO Service\n");
        goto failure;
    }
    
    inout in;
    //size_t outsize = sizeof(in);

    in.action = DefaultMACAddr;
    memset(in.macaddr, 0, sizeof(in.macaddr));
    
#if MAC_OS_X_VERSION_MIN_REQUIRED <= MAC_OS_X_VERSION_10_4
    ret = IOConnectMethodStructureIStructureO(connect, DefaultMACAddr,
                                              sizeof(in),			/* structureInputSize */
                                              NULL,    /* structureOutputSize */
                                              &in,        /* inputStructure */
                                              NULL);       /* ouputStructure */
#else
    ret = IOConnectCallStructMethod(connect,
                                    DefaultMACAddr,
                                    &in,
                                    sizeof(in),
                                    NULL,
                                    NULL
                                    );
#endif
    
    if (ret != KERN_SUCCESS)
    {
        dialog("Can't connect to StructMethod to send commands\n");
    } else {
        dialog("Set MAC address to original value\n");
    }
    
failure:
    if(connect)
    {
        ret = IOServiceClose(connect);
        if (ret != KERN_SUCCESS)
        {
            dialog("IOServiceClose failed\n");
        }
    }

    if(service)
        IOObjectRelease(service);
}

- (void) mainViewDidLoad
{
    [super mainViewDidLoad];
}

@end
