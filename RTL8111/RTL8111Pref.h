//
//  RTL8111Pref.h
//

#ifndef RTL8111PREF_H
#define RTL8111PREF_H

#import <PreferencePanes/PreferencePanes.h>
#import <IOKit/IOKitKeys.h>
#import <IOKit/IOKitLib.h>

#define kRTL8111ClassName	"RTL8111"
enum {
    DefaultMACAddr = 0,
    SetMACAddr = 1,
    AnVMSRNumMethods
};

#ifndef kIOEthernetAddressSize
#define kIOEthernetAddressSize 6
#endif

typedef struct {
    UInt32 action;
    UInt8 macaddr[kIOEthernetAddressSize];
} inout;

@interface RTL8111Pref : NSPreferencePane
{

    IBOutlet NSButton *setMACAddr;
	IBOutlet NSButton *setOrigMACAddr;
    IBOutlet NSTextField *MACAddrVal;

    io_service_t service;
}

- (IBAction)setOrigMACAddr:(id)sender;
- (IBAction)setMACAddr:(id)sender;
@end

#endif
