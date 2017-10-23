#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <QuickLook/QuickLook.h>

#include "hdf5OSXImpl.hpp"

OSStatus GenerateThumbnailForURL(void *thisInterface, QLThumbnailRequestRef thumbnail, CFURLRef url, CFStringRef contentTypeUTI, CFDictionaryRef options, CGSize maxSize);
void CancelThumbnailGeneration(void *thisInterface, QLThumbnailRequestRef thumbnail);

/* -----------------------------------------------------------------------------
    Generate a thumbnail for file

   This function's job is to create thumbnail for designated file as fast as possible
   ----------------------------------------------------------------------------- */

OSStatus GenerateThumbnailForURL(void *thisInterface, QLThumbnailRequestRef thumbnail, CFURLRef url, CFStringRef contentTypeUTI, CFDictionaryRef options, CGSize maxSize)
{
    CFStringRef stringURL = CFURLGetString(url);
    const char* path = CFStringGetCStringPtr(stringURL, kCFStringEncodingUTF8);
    long length = CFStringGetLength(stringURL);
    
    getThumbnailOSX(path, length);
    CGDataProviderRef dataProvider = CGDataProviderCreateWithFilename("/tmp/thumbnail.png");
    CGImageRef image = CGImageCreateWithPNGDataProvider(dataProvider, NULL, false, kCGRenderingIntentDefault);
    QLThumbnailRequestSetImage(thumbnail, image, NULL);
    CGDataProviderRelease(dataProvider);
    CGImageRelease(image);
    // To complete your generator please implement the function GenerateThumbnailForURL in GenerateThumbnailForURL.c
    return noErr;
}

void CancelThumbnailGeneration(void *thisInterface, QLThumbnailRequestRef thumbnail)
{
    // Implement only if supported
}
