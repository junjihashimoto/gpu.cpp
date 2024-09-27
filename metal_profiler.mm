#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

extern "C" {
    void startCapture() {
        id<MTLDevice> device = MTLCreateSystemDefaultDevice();
        if (@available(macOS 10.15, *)) {
            MTLCaptureManager *captureManager = [MTLCaptureManager sharedCaptureManager];
            [captureManager startCaptureWithDevice:device];
        }
    }

    void stopCapture() {
        if (@available(macOS 10.15, *)) {
            MTLCaptureManager *captureManager = [MTLCaptureManager sharedCaptureManager];
            [captureManager stopCapture];
        }
    }
}
