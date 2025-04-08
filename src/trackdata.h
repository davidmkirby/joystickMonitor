#ifndef TRACKDATA_H
#define TRACKDATA_H

#include <cstdint>
#include <QString>

struct TrackData {
    // Raw track errors
    float rawErrorX;
    float rawErrorY;

    // Filtered track errors
    float filteredErrorX;
    float filteredErrorY;

    // Status information
    uint16_t targetPolarity;
    uint16_t trackState;
    uint16_t trackMode;
    uint16_t status;

    // Target information
    uint16_t targetSizeX;
    uint16_t targetSizeY;
    uint16_t targetLeft;
    uint16_t targetTop;
    uint16_t targetPixelCount;

    // Mount position
    int32_t azimuth;
    int32_t elevation;

    // Helper functions to get status as strings
    QString getPolarityString() const {
        switch (targetPolarity) {
            case 0: return "Gray";
            case 1: return "White";
            case 2: return "Black";
            case 3: return "Mix";
            case 4: return "Auto";
            default: return "Unknown";
        }
    }

    QString getStateString() const {
        switch (trackState) {
            case 0: return "Initialization";
            case 1: return "Acquire";
            case 2: return "Pending Track";
            case 3: return "On Track";
            case 4: return "Coast";
            case 5: return "Off Track";
            case 6: return "Auto Acquire";
            default: return "Unknown";
        }
    }

    QString getModeString() const {
        switch (trackMode) {
            case 0: return "Top edge";
            case 1: return "Bottom edge";
            case 2: return "Left edge";
            case 3: return "Right edge";
            case 4: return "Centroid";
            case 5: return "Intensity";
            case 6: return "Vector";
            case 7: return "Correlation";
            default: return "Unknown";
        }
    }

    QString getStatusString() const {
        QString result;
        if (status & 0x0002) result += "TOO FEW TARGET PIXELS, ";
        if (status & 0x0004) result += "TOO MANY TARGET PIXELS, ";
        if (status & 0x0008) result += "X POSITION FAIL, ";
        if (status & 0x0010) result += "Y POSITION FAIL, ";
        if (status & 0x0020) result += "NCOUNT TOO LARGE, ";
        if (status & 0x0040) result += "NCOUNT TOO SMALL, ";
        if (status & 0x0080) result += "X SIZE FAIL, ";
        if (status & 0x0100) result += "Y SIZE FAIL, ";
        if (status & 0x1000) result += "CORR MATCH FAIL, ";

        if (result.isEmpty()) return "OK";

        // Remove trailing comma and space
        result.chop(2);
        return result;
    }
};

#endif // TRACKDATA_H