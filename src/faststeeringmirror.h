#ifndef FASTSTEERINGMIRROR_H
#define FASTSTEERINGMIRROR_H

#include <QObject>
#include <QString>
#include "bdaqctrl.h"

using namespace Automation::BDaq;

class FastSteeringMirror : public QObject
{
    Q_OBJECT

public:
    explicit FastSteeringMirror(QObject *parent = nullptr);
    ~FastSteeringMirror();

    // Initialize the device
    bool initialize();
    void cleanup();

    // Get available devices
    QStringList getAvailableDevices() const;

    // Open/close device
    bool openDevice(const QString &deviceName);
    void closeDevice();
    bool isDeviceOpen() const;

    // Set mirror position (-1.0 to 1.0 range for each axis)
    bool setPosition(double xPosition, double yPosition);

    // Get current output voltage values
    QPair<double, double> getCurrentVoltages() const;

    // Configure voltage range
    bool setVoltageRange(double minVoltage, double maxVoltage);

    // XML profile support
    void setProfilePath(const QString &profilePath);
    bool loadProfile(const QString &profilePath);

    // Error handling
    QString getLastError() const;

signals:
    void positionChanged(double xPosition, double yPosition);
    void deviceError(const QString &errorMessage);

private:
    InstantAoCtrl *m_aoCtrl;
    bool m_initialized;
    QString m_lastError;
    double m_minVoltage;
    double m_maxVoltage;
    double m_currentVoltages[2];
    QString m_profilePath;

    // Error handling helper
    void checkError(ErrorCode errorCode);

    // Convert normalized position (-1.0 to 1.0) to voltage
    double positionToVoltage(double position) const;
};

#endif // FASTSTEERINGMIRROR_H