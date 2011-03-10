#include "CameraThread.h"

#include <FCam/Shot.h>

#include <vector>
#include <iostream>
#include <QMessageBox>

#include "SoundPlayer.h"
#include "LEDBlinker.h"
#include "UserDefaults.h"

#include "AppState.h"
#include "OverlayWidget.h"
#include "RecognitionEngine.h"

#include "arm_neon.h"

using namespace std;

void CameraThread::run() {
    exitLock.lock();
    if (!overlay) {
        printf("No overlay attached to camera thread, exiting!\n");
        return;
    }

    printf("Camera thread running...\n");
    

    // The viewfinder 
    viewfinder.exposure = 33333;
    viewfinder.gain = 1.0f;
    viewfinder.image = overlay->framebuffer();
    viewfinder.histogram.region = FCam::Rect(0, 0, 640, 480);
    viewfinder.histogram.enabled = true;
    viewfinder.sharpness.enabled = true;
    viewfinder.sharpness.size = FCam::Size(16, 12);
    viewfinder.whiteBalance = 5000;
    viewfinder.id = VIEWFINDER;

    // An HDR variant of the viewfinder that alternates between two
    // viewfinder-sized frames drawn off-screen
    hdrViewfinder[0] = hdrViewfinder[1] = viewfinder;
    hdrViewfinder[0].image = FCam::Image(overlay->framebuffer().size(), FCam::UYVY);
    hdrViewfinder[0].id = HDR_VIEWFINDER_HI;
    hdrViewfinder[1].image = FCam::Image(overlay->framebuffer().size(), FCam::UYVY);
    hdrViewfinder[1].id = HDR_VIEWFINDER_LO;

    bool takeSnapshot = false;
    bool halfDepress = false;
    bool fullDepress = false;
    bool hdrMode = false;
    bool takeViewFinderSnapshot= false;
    
    FCam::Frame frame;
    int lastShotId = -1;

    int luckyCount = 0;
    FCam::Frame sharpest;
    int sharpestScore = 0;

    printf("Initiating streaming...\n");
    // stream the viewfinder
    sensor.stream(viewfinder);

    printf("Making sound player device...\n");
    // Make a device that makes a shutter sound with the picture is taken
    SoundPlayer soundPlayer;
    SoundPlayer::SoundAction exposureStartSound(&soundPlayer);
    
    printf("Making LED blinker device...\n");
    // Make a device that blinks the front LED when a picture is taken
    LEDBlinker blinker;
    LEDBlinker::BlinkAction blink(&blinker);

    printf("Entering main loop...\n");
    while (keepGoing) {
        // deal with FCam events
        FCam::Event e;
        while (FCam::getNextEvent(&e)) {
            switch(e.type) {
            case FCam::Event::Error:
                printf("FCam Error: %s\n", e.description.c_str());                
                if (e.data == FCam::Event::DriverLockedError || e.data == FCam::Event::DriverMissingError) {                
                    emit panic(e);
                    sensor.stop();
                    exitLock.lock();
                    printf("Terminating\n");
                    return;
                }
                break;
            case FCam::Event::Warning:
                printf("FCam Warning: %s\n", e.description.c_str());
                break;
            case FCam::Event::FocusPressed:
                emit focusPressed();
                parameters.mutex.lock();

                if (autoFocus.idle()) {
                    if (parameters.focus.mode == CameraParameters::Focus::AUTO) {
                        autoFocus.setTarget(FCam::Rect(0, 0, 640, 480));
                        autoFocus.startSweep();       
                    } else if (parameters.focus.mode == CameraParameters::Focus::SPOT) {
                        int x = parameters.focus.spot.x();
                        int y = parameters.focus.spot.y();
                        printf("Setting target to %d %d\n", x, y);
                        autoFocus.setTarget(FCam::Rect(x-15, y-15, 30, 30));
                        autoFocus.startSweep();
                    }
                }

                parameters.mutex.unlock();
                halfDepress = true;
                break;
            case FCam::Event::FocusReleased:
                emit focusReleased();
                halfDepress = false;
                break;
            case FCam::Event::ShutterPressed:
                emit shutterPressed();
                //takeSnapshot = true;
                fullDepress = true;
                takeViewFinderSnapshot = true;
                break;
            case FCam::Event::ShutterReleased:
                emit shutterReleased();
                fullDepress = false;
                break;
            case FCam::Event::N900LensClosed:
                emit lensCoverClosed();
                break;
            case FCam::Event::N900LensOpened:
                emit lensCoverOpened();
                break;                
            };
        }//done dealing with events

        // Take a picture if appropriate    
        if (takeSnapshot && autoFocus.idle()) { 
            if (hdrMode) {
                // figure out how many shots to take
                float b0 = hdrViewfinder[0].exposure * hdrViewfinder[0].gain;
                float b1 = hdrViewfinder[1].exposure * hdrViewfinder[1].gain;
                int shots = (int)ceilf(log2f(b0/b1)/2 + 1); // each shot can be up to 2 stops apart
                if (shots < 2) shots = 2;
                hdrPhoto.resize(shots);
                for (int i = 0; i < shots; i++) {
                    float b = expf((i * logf(b0) + (shots-1-i) * logf(b1))/(shots-1));
                    float exposure = b;
                    float gain = 1;
                    if (exposure > 33333) {
                        gain = exposure/33333;
                        exposure = 33333;
                    }
                    if (gain > sensor.maxGain()) {
                        exposure *= gain/sensor.maxGain();
                        gain = sensor.maxGain();
                    }


                    // Give it the same id as the photo - for now we
                    // don't need to be able to tell it apart when we
                    // get the frame back.
                    hdrPhoto[i].id = HDR;
                    hdrPhoto[i].image = FCam::Image(2592, 1968, FCam::RAW);
                    hdrPhoto[i].exposure = exposure;
                    hdrPhoto[i].gain = gain;
                    hdrPhoto[i].whiteBalance = ((i * hdrViewfinder[0].whiteBalance + 
                                                 (shots-1-i) * hdrViewfinder[1].whiteBalance)/
                                                (shots-1));
                    hdrPhoto[i].frameTime = 200000;
                    hdrPhoto[i].clearActions();
                    if (UserDefaults::instance()["captureSound"].asInt()) {
                        hdrPhoto[i].addAction(exposureStartSound);                    
                    }
                    if (UserDefaults::instance()["captureBlink"].asInt()) {
                        hdrPhoto[i].addAction(blink);                    
                    }                    
                }
                sensor.capture(hdrPhoto);
                takeSnapshot = false;
            } else {
                // Configure photo for a 5MP raw frame, allocated on demand
                photo.image = FCam::Image(2592, 1968, FCam::RAW, FCam::Image::AutoAllocate);
                photo.exposure     = int(parameters.exposure.value * 1000000 + 0.5);
                photo.gain         = parameters.gain.value;
                photo.whiteBalance = parameters.whiteBalance.value;
                photo.frameTime = 0;
                
                // Make a camera sound at the start of the exposure
                photo.clearActions();
                if (UserDefaults::instance()["captureSound"].asInt()) {
                    photo.addAction(exposureStartSound);
                }
                if (UserDefaults::instance()["captureBlink"].asInt()) {
                    photo.addAction(blink);
                }
                
                if (parameters.burst.mode == CameraParameters::Burst::SINGLE) {
                    photo.id = SINGLE;
                    sensor.capture(photo);
                } else if (parameters.burst.mode == CameraParameters::Burst::CONTINUOUS) {
                    // take a quick burst of four
                    std::vector<FCam::Shot> burst;
                    burst.resize(4);
                    for (int i = 0; i < 4; i++) {
                        burst[i] = photo;            
                        burst[i].id = BURST;
                    }
                    sensor.capture(burst);
                } else { 
                    // Save the sharpest of 8
                    std::vector<FCam::Shot> burst;
                    burst.resize(8);
                    for (int i = 0; i < 8; i++) {
                        burst[i] = photo;
                        burst[i].frameTime = 250000;
                        burst[i].id = SHARPEST;
                    }
                    sensor.capture(burst);
                    luckyCount = 0;
                }

                takeSnapshot = false;
            }
        }

        
        bool imageSaved = false;
        bool imageSavedWasBurst = false;
        // Drain the frame queue
        do {
            // Do anything that must be done with the returned frame
            if (frame) lastShotId = frame.shot().id;
            frame = sensor.getFrame();

            switch (frame.shot().id) {
            case SINGLE:
            case BURST:
            case HDR:
                // We got a photo back
                if (!frame.image().valid()) {
                    printf("ERROR: Photo dropped!\n");
                    continue;
                }
                
                emit newImage(new ImageItem(frame));
                imageSaved = true;
                imageSavedWasBurst = frame.shot().id != SINGLE;
                break;
            case SHARPEST: {
                    // evaluate the sharpness of this frame
                    FCam::Image im = frame.image();
                    int sharpness = 0;
                    if (im.valid()) {
                        for (size_t y = 20; y < im.height()-20; y += 20) {
                            for (size_t x = 20; x < im.width()-20; x += 20) {
                                sharpness += abs(im(x, y)[1] - im(x-1, y)[1]);
                                sharpness += abs(im(x, y)[1] - im(x+1, y)[1]);
                                sharpness += abs(im(x, y)[1] - im(x, y-1)[1]);
                                sharpness += abs(im(x, y)[1] - im(x, y+1)[1]);
                            }
                        }
                    }
                    printf("Frame %d had sharpness %d\n", luckyCount, sharpness);

                    // Decide whether it's better than the best so far
                    if (luckyCount == 0 || sharpness > sharpestScore) {
                        printf("Accepting as sharper\n");
                        sharpest = frame;
                        sharpestScore = sharpness;
                    } else {
                        printf("Rejecting as blurrier\n");
                    }
                    // Save the best one in the burst
                    if (luckyCount == 7) {
                        emit newImage(new ImageItem(sharpest));
                        imageSaved = true;
                        sharpest = FCam::Frame();
                    }

                    luckyCount++;
                    break;
                }
            case VIEWFINDER:
            case HDR_VIEWFINDER_LO:
            case HDR_VIEWFINDER_HI:
                // Deal with a returned viewfinder frame
                updateState(frame);

                // Stream the updated shot if we're not in continuous mode
                if (parameters.exposure.mode == CameraParameters::Exposure::AUTO_HDR) {
                    if (!hdrMode) {
                        // it's our first frame of HDR, grab the settings
                        // from the regular viewfinder
                        hdrViewfinder[0].whiteBalance = hdrViewfinder[1].whiteBalance = viewfinder.whiteBalance;
                        hdrViewfinder[0].exposure = hdrViewfinder[1].exposure = viewfinder.exposure;
                        hdrViewfinder[0].gain = hdrViewfinder[1].gain = viewfinder.gain;
                        hdrMode = true;
                    }
                    sensor.stream(hdrViewfinder);
                } else {
                    if (hdrMode) {
                        // It's our first frame of non-hdr, grab the
                        // settings from the hdr mode
                        viewfinder.whiteBalance = hdrViewfinder[1].whiteBalance;
                        viewfinder.exposure = hdrViewfinder[1].exposure;
                        viewfinder.gain = hdrViewfinder[1].gain;
                        hdrMode = false;
                    }                    
                    sensor.stream(viewfinder);
                }
                break;
            default:
                printf("Got back a frame with unknown id: %d\n", frame.shot().id);
            }
        } while (sensor.framesPending());
        // Display the animation for the frame capture (box moving from viewfinder down to review screen)
        if (imageSaved) emit captureComplete(imageSavedWasBurst);
        
        // Now do any optional, CPU-intensive things with the returned frame
        switch (frame.shot().id) {
        case VIEWFINDER:
            // Don't need to do anything to make the frame appear -
            // the image has already been dumped into the viewfinder's
            // framebuffer memory
            break;
        case HDR_VIEWFINDER_LO:
            break;
        case HDR_VIEWFINDER_HI:
            if (lastShotId == HDR_VIEWFINDER_HI || lastShotId == HDR_VIEWFINDER_LO) {
                // Average the two frames and blit it to the viewfinder using arm neon instructions
                for (size_t y = 0; y < overlay->framebuffer().height(); y ++) {
                    uint8x16_t *outPtr = (uint8x16_t *)overlay->framebuffer()(0,y);
                    uint8x16_t *loPtr  = (uint8x16_t *)hdrViewfinder[0].image(0,y);
                    uint8x16_t *hiPtr  = (uint8x16_t *)hdrViewfinder[1].image(0,y);            
                    for (size_t x = 0; x < overlay->framebuffer().width()/8; x++) {
                        *outPtr++ = vrhaddq_u8(*loPtr++, *hiPtr++);
                    }
                } 
            }
            break;
        default:
            break;
        }
        //TO DO: Cannot lock the frame's image although this seems to be required according to the documentation
        //bool isLocked = frame.image().lock();
        //if(isLocked)
        //{
        if(appState->recEngine != NULL)
        {
            appState->recEngineMutex.lock();
            if(appState->recEngine->templateFeatures.size() > 0) //if have templates
            {
                // copy intensity channel
                try{
                    for (int y = 0; y < 480; ++y) {
                        unsigned char *row = frame.image()(0, y);
                        for (int x = 0; x < 640; ++x) {
                            int idx = (x << 1) + 1;
                            ((uchar*)(appState->recEngine->large_iplGray->imageData + appState->recEngine->large_iplGray->widthStep*y))[x] = row[idx];
                        }
                    }
                    cvResize( appState->recEngine->large_iplGray, appState->recEngine->iplGray, CV_INTER_CUBIC);
                }
                catch( cv::Exception& e )
                {
                    const char* err_msg = e.what();
                    printf("CV Exception in ProcessFrame: %s \n", err_msg );
                    return;
                }
                appState->recEngineMutex.unlock();
                //  frame.image().unlock();
                emit viewfinderFrame();
                if( takeViewFinderSnapshot ) {
                    emit newSnapshotFrame(frame);
                    takeViewFinderSnapshot = false;
                }
            }
            else
                appState->recEngineMutex.unlock();
            // }
        }
    } //end while keep going

    sensor.stop();
}

/** Auto expose, making the yth percentile hit a brightness of x */
void CameraThread::meter(FCam::Shot *s, FCam::Frame f, float x, float y) {
    if (!f || !s || !f.histogram().valid()) return;   

    // How far along the histogram is the xth percentile?
    const FCam::Histogram &h = f.histogram();
    int total = 0; 
    size_t i;
    for (i = 0; i < h.buckets(); i++) {
        total += h(i);
    }

    int sum = 0;
    for (i = 0; i < h.buckets(); i++) {
        sum += h(i);
        if (sum > total*x) break;
    }    

    float v = (i+1)/(float)h.buckets();

    float adjustment = (v >= 1.0) ? 0.5 : (y/v);
    float brightness = f.exposure()*f.gain();
    float desiredBrightness = brightness*adjustment;

    int exposure;
    float gain;

    int exposureKnee = 40000;

    // make the change smooth when they are small
    float oldBrightness = s->exposure * s->gain;
    float smoothness = oldBrightness/desiredBrightness;
    if (smoothness < 1) smoothness = 1.0f/smoothness;
    if (smoothness > 1.75) smoothness = 1.75;
    smoothness = 2-smoothness;
    desiredBrightness = (1-smoothness)*desiredBrightness + smoothness*oldBrightness;

    // Set the exposure as high as possible without sacrificing frame rate
    if (desiredBrightness > exposureKnee) {
        exposure = exposureKnee;
        gain = desiredBrightness / exposureKnee;
    } else {
        gain = 1.0f;
        exposure = desiredBrightness;
    }

    // Clamp the gain at max, and try to make up for it with exposure (sacrificing frame rate)
    if (gain > sensor.maxGain()) {
        exposure = desiredBrightness/sensor.maxGain();
        gain = sensor.maxGain();
    } 

    // Finally, clamp the exposure at max
    if (exposure > 125000) {
        exposure = 125000;
    }

    s->exposure = exposure;
    s->gain = gain;
}


void CameraThread::updateState(const FCam::Frame &frame) {

    parameters.mutex.lock();
    
    // FOCUS
    if (parameters.focus.mode == CameraParameters::Focus::MANUAL) {
        if (!autoFocus.idle()) autoFocus.update(frame);        
        else if (lens.getFocus() != parameters.focus.value && 
                 !lens.focusChanging())
            lens.setFocus(parameters.focus.value, lens.maxFocusSpeed());

    } else { // spot focus or auto focus
        autoFocus.update(frame);
        parameters.focus.value = lens.getFocus();
    }
    
    // EXPOSURE TIME AND GAIN
    if (parameters.exposure.mode != CameraParameters::Exposure::MANUAL ||
        parameters.gain.mode != CameraParameters::Gain::MANUAL) {
        
        switch (parameters.exposure.mode) {
        case CameraParameters::Exposure::HIGHLIGHTS:
            if (frame.shot().id == viewfinder.id)
                meter(&viewfinder, frame, 0.995, 0.9);
            break;                    
        case CameraParameters::Exposure::SHADOWS:
            if (frame.shot().id == viewfinder.id)
                meter(&viewfinder, frame, 0.1, 0.1);
            break;
        case CameraParameters::Exposure::AUTO_HDR:
            if (frame.shot().id == HDR_VIEWFINDER_HI) {
                meter(&(hdrViewfinder[0]), frame, 0.1, 0.1);
            } else if (frame.shot().id == HDR_VIEWFINDER_LO) {
                meter(&(hdrViewfinder[1]), frame, 0.98, 0.8);
            }
            break;
        default: // auto exposure mode
            if (frame.shot().id == viewfinder.id)
                meter(&viewfinder, frame, 0.9, 0.4);
            break;
        }

        if (parameters.exposure.mode == CameraParameters::Exposure::MANUAL) {
            int newExp = int(1000000*parameters.exposure.value+0.5);
            viewfinder.gain *= viewfinder.exposure;
            viewfinder.gain /= newExp;
            viewfinder.exposure = newExp;
            // clamp to possible gains
            if (viewfinder.gain < 1.0) viewfinder.gain = 1.0;
            if (viewfinder.gain > 32.0) viewfinder.gain = 32.0;
        } else {
            if (parameters.exposure.mode == CameraParameters::Exposure::AUTO_HDR) {
                parameters.exposure.value = hdrViewfinder[0].exposure / 1000000.0f;
            } else {
                parameters.exposure.value = viewfinder.exposure / 1000000.0f;
                parameters.exposure.value *= viewfinder.gain/parameters.gain.value;
            }
        }

        if (parameters.gain.mode == CameraParameters::Gain::MANUAL) {
            int newGain = parameters.gain.value;
            viewfinder.exposure *= viewfinder.gain / newGain;
            viewfinder.gain = newGain;
        } else {
            if (parameters.exposure.mode == CameraParameters::Exposure::AUTO_HDR) {
                parameters.gain.value = hdrViewfinder[0].gain;
            } else {
                parameters.gain.value = viewfinder.gain;
            }
        }            

        // clamp to possible exposures
        if (parameters.exposure.value > 1.0f) {
            parameters.exposure.value = 1.0f;
            viewfinder.exposure = 1000000;
        }


    } else {

        viewfinder.exposure = int(1000000*parameters.exposure.value+0.5);
        viewfinder.gain = parameters.gain.value;
    }

    // We don't want the exposure time for the viewfinder to get too
    // long, or the framerate drops
    if (viewfinder.exposure > 125000) {
        viewfinder.gain *= viewfinder.exposure;
        viewfinder.gain /= 125000;
        viewfinder.exposure = 125000;
        if (viewfinder.gain > sensor.maxGain()) {
            // If we've hit maximum gain, we have no choice but to lengthen the exposure time
            viewfinder.exposure /= sensor.maxGain();
            viewfinder.exposure *= viewfinder.gain;
            viewfinder.gain = sensor.maxGain();            
        }
    }

    // WHITE BALANCE
    switch (parameters.whiteBalance.mode) {
    case CameraParameters::WhiteBalance::AUTO:
        if (frame.shot().id == HDR_VIEWFINDER_HI) {
            autoWhiteBalance(&hdrViewfinder[0], frame);
        } else if (frame.shot().id == HDR_VIEWFINDER_LO) {
            autoWhiteBalance(&hdrViewfinder[1], frame);
            parameters.whiteBalance.value = hdrViewfinder[0].whiteBalance;
        } else if (frame.shot().id == VIEWFINDER) {
            autoWhiteBalance(&viewfinder, frame);
            parameters.whiteBalance.value = viewfinder.whiteBalance;
        }
        break;
    case CameraParameters::WhiteBalance::MANUAL:
        viewfinder.whiteBalance = (int)(parameters.whiteBalance.value + 0.5);
        hdrViewfinder[0].whiteBalance = viewfinder.whiteBalance;
        hdrViewfinder[1].whiteBalance = viewfinder.whiteBalance;
        break;
    }    
    
    parameters.mutex.unlock();

    parameters.notify();
}

bool CameraThread::lensCovered() {
    FILE * file = fopen("/sys/devices/platform/gpio-switch/cam_shutter/state", "r");
    char state = fgetc(file); // file contains either "open" or "closed"
    fclose(file);
    return state == 'c';
}



