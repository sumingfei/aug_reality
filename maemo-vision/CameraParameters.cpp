#include "CameraParameters.h"

CameraParameters::CameraParameters() {
    // Set default modes and values
    exposure.mode = Exposure::AUTO;
    exposure.value = 1.0f/30;
    gain.mode = Gain::AUTO;
    gain.value = 1.0f;
    focus.mode = Focus::AUTO;
    focus.value = 0.0f; // infinity
    whiteBalance.mode = WhiteBalance::AUTO;
    whiteBalance.value = 5000;
    burst.mode = Burst::SINGLE;
}

CameraParameters::~CameraParameters() {}

QString CameraParameters::Exposure::toString(float val) {
    QString str;
    
    if (val < 0.001) {
        str.sprintf("1/%d000s", (int)(0.001/val+0.5));
    } else if (val < 0.01) {
        str.sprintf("1/%d00s", (int)(0.01/val+0.5));
    } else if (val < 0.1) {
        str.sprintf("1/%d0s", (int)(0.1/val+0.5));
    } else if (val < 0.2) {
        str.sprintf("1/%ds", (int)(1/val+0.5));
    } else if (val < 0.95) {
        str.sprintf("0.%ds", (int)(10*val+0.5));
    } else {
        str.sprintf("%ds", (int)(val+0.5));
    }
    return str;
}

QString CameraParameters::Gain::toString(float val) {
    QString str;
    str.sprintf("ISO %d0", (int)(val*10 + 0.5));
    return str;
}

QString CameraParameters::Focus::toString(float val) {
    QString str;
    if (val > 5.0) { // up to 20cm
        str.sprintf("%dcm", (int)(100/val + 0.5));
    } else if (val > 1.0) { // up to 1m
        str.sprintf("%d0cm", (int)(10/val + 0.5));
    } else if (val > 0.2) { // up to 5m
        str.sprintf("%dm", (int)(1/val + 0.5));
    } else {
        str.sprintf(">5m");
    }
    return str;
}

QString CameraParameters::WhiteBalance::toString(float val) {
    QString str;
    str.sprintf("%d00K", (int)(val/100 + 0.5));
    return str;
}

