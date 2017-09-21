#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <QuickLook/QuickLook.h>

#include "hdf5OSXImpl.hpp"

OSStatus GeneratePreviewForURL(void *thisInterface, QLPreviewRequestRef preview, CFURLRef url, CFStringRef contentTypeUTI, CFDictionaryRef options);
void CancelPreviewGeneration(void *thisInterface, QLPreviewRequestRef preview);

/* -----------------------------------------------------------------------------
   Generate a preview for file

   This function's job is to create preview for designated file
   ----------------------------------------------------------------------------- */

OSStatus GeneratePreviewForURL(void *thisInterface, QLPreviewRequestRef preview, CFURLRef url, CFStringRef contentTypeUTI, CFDictionaryRef options)
{
    CFStringRef stringURL = CFURLGetString(url);
    const char* path = CFStringGetCStringPtr(stringURL, kCFStringEncodingUTF8);
    long length = CFStringGetLength(stringURL);
    
    CGImageRef image = getThumbnailOSX(path, length);
    if(image == NULL) {
        return -1;
    }
    CGFloat width = CGImageGetWidth(image);
    CGFloat height = CGImageGetHeight(image);
    CGSize size = {width, height};
    CGContextRef context = QLPreviewRequestCreateContext(preview, size, true, NULL);

    if(!context) {
        return -1;
    }
    CGContextDrawImage(context, CGRectMake(0, 0, width, height), image);
    QLPreviewRequestFlushContext(preview, context);
    CGImageRelease(image);
    CGContextRelease(context);

    return noErr;
}

void CancelPreviewGeneration(void *thisInterface, QLPreviewRequestRef preview)
{
    // Implement only if supported
}
