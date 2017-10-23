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
    
    getThumbnailOSX(path, length);
    CGDataProviderRef dataProvider = CGDataProviderCreateWithFilename("/tmp/thumbnail.png");
    CGImageRef image = CGImageCreateWithPNGDataProvider(dataProvider, NULL, false, kCGRenderingIntentDefault);
    
    CGFloat width = CGImageGetWidth(image);
    CGFloat height = CGImageGetHeight(image);
    CGSize size = {width, height};
    CGContextRef context = QLPreviewRequestCreateContext(preview, size, true, NULL);

    if(!context) {
        CGDataProviderRelease(dataProvider);
        CGImageRelease(image);
        return -1;
    }
    CGContextDrawImage(context, CGRectMake(0, 0, width, height), image);
    QLPreviewRequestFlushContext(preview, context);

    CGContextRelease(context);
    CGImageRelease(image);
    CGDataProviderRelease(dataProvider);
    return noErr;
}

void CancelPreviewGeneration(void *thisInterface, QLPreviewRequestRef preview)
{
    // Implement only if supported
}
