#include "faststeeringmirror.h"
#include <QDebug>

using namespace Automation::BDaq;

FastSteeringMirror::FastSteeringMirror(QObject *parent)
    : QObject(parent)
    , m_aoCtrl(nullptr)
    , m_initialized(false)
    , m_minVoltage(-10.0)
    , m_maxVoltage(10.0)
{
    m_currentVoltages[0] = 0.0;
    m_currentVoltages[1] = 0.0;
}

FastSteeringMirror::~FastSteeringMirror()
{
    cleanup();
}

bool FastSteeringMirror::initialize()
{
    if (m_initialized) {
        return true;
    }

    try {
        // Create the AO control instance
        m_aoCtrl = InstantAoCtrl::Create();
        if (!m_aoCtrl) {
            m_lastError = "Failed to create InstantAoCtrl instance";
            qDebug() << "Initialization failed:" << m_lastError;
            return false;
        }

        m_initialized = true;
        qDebug() << "FastSteeringMirror initialized successfully";
        return true;
    }
    catch (const std::exception &e) {
        m_lastError = QString("Exception during initialization: %1").arg(e.what());
        qDebug() << "Initialization failed with exception:" << m_lastError;
        return false;
    }
}

void FastSteeringMirror::cleanup()
{
    closeDevice();

    if (m_aoCtrl) {
        m_aoCtrl->Dispose();
        m_aoCtrl = nullptr;
    }

    m_initialized = false;
    qDebug() << "FastSteeringMirror cleaned up";
}

QStringList FastSteeringMirror::getAvailableDevices() const
{
    QStringList devices;

    if (!m_initialized) {
        qWarning() << "FastSteeringMirror not initialized";
        return devices;
    }

    Array<DeviceTreeNode> *supportedDevices = m_aoCtrl->getSupportedDevices();
    qDebug() << "Found" << supportedDevices->getCount() << "supported devices";

    for (int i = 0; i < supportedDevices->getCount(); i++) {
        DeviceTreeNode const &node = supportedDevices->getItem(i);
        QString description = QString::fromWCharArray(node.Description);

        // Check for various Advantech card models with AO capability
        if (description.contains("PCIE-1824", Qt::CaseInsensitive) ||
            description.contains("PCIE-1816", Qt::CaseInsensitive) ||
            description.contains("PCIE-1810", Qt::CaseInsensitive) ||
            description.contains("PCIE-1802", Qt::CaseInsensitive) ||
            description.contains("USB-4702", Qt::CaseInsensitive) ||
            description.contains("USB-4704", Qt::CaseInsensitive) ||
            description.contains("USB-4750", Qt::CaseInsensitive) ||
            // Add any other model numbers that support AO
            // Or use a more generic check if possible:
            description.contains("AO", Qt::CaseInsensitive)) {

            devices.append(description);
            qDebug() << "Found compatible device:" << description;
        }
    }

    if (devices.isEmpty()) {
        qDebug() << "No compatible Advantech analog output devices found";
    }

    return devices;
}

void FastSteeringMirror::setProfilePath(const QString &profilePath)
{
    m_profilePath = profilePath;
    qDebug() << "Set profile path to:" << profilePath;
}

bool FastSteeringMirror::loadProfile(const QString &profilePath)
{
    if (!m_initialized || !m_aoCtrl) {
        m_lastError = "Mirror controller not initialized";
        qDebug() << "Cannot load profile:" << m_lastError;
        return false;
    }

    qDebug() << "Loading profile:" << profilePath;

    try {
        std::wstring wProfilePath = profilePath.toStdWString();
        ErrorCode errCode = m_aoCtrl->LoadProfile(wProfilePath.c_str());

        if (errCode != Success) {
            m_lastError = QString("Failed to load profile, error code: 0x%1").arg(
                QString::number(errCode, 16).right(8).toUpper());
            qDebug() << m_lastError;
            return false;
        }

        qDebug() << "Profile loaded successfully";
        m_profilePath = profilePath;
        return true;
    }
    catch (const std::exception &e) {
        m_lastError = QString("Exception when loading profile: %1").arg(e.what());
        qDebug() << m_lastError;
        return false;
    }
}

bool FastSteeringMirror::openDevice(const QString &deviceName)
{
    if (!m_initialized) {
        m_lastError = "FastSteeringMirror not initialized";
        qDebug() << "Cannot open device:" << m_lastError;
        return false;
    }

    // Clean up existing device first
    closeDevice();

    qDebug() << "Attempting to open device:" << deviceName;

    try {
        // Simple approach similar to Advantech examples
        std::wstring wDeviceName = deviceName.toStdWString();
        DeviceInformation devInfo(wDeviceName.c_str());

        // Select the device
        ErrorCode errCode = m_aoCtrl->setSelectedDevice(devInfo);
        qDebug() << "setSelectedDevice returned code:" << errCode;

        if (errCode != Success) {
            m_lastError = QString("Failed to select device, error code: 0x%1").arg(
                QString::number(errCode, 16).right(8).toUpper());
            qDebug() << m_lastError;
            return false;
        }

        qDebug() << "Device selected successfully";

        // If a profile path was provided, try to load it
        if (!m_profilePath.isEmpty()) {
            qDebug() << "Applying device profile from:" << m_profilePath;
            std::wstring wProfilePath = m_profilePath.toStdWString();
            errCode = m_aoCtrl->LoadProfile(wProfilePath.c_str());

            if (errCode != Success) {
                qDebug() << "Warning: Failed to load profile, error code:" << errCode;
                // Continue - non-fatal error
            } else {
                qDebug() << "Profile loaded successfully";
            }
        }

        // Check if we have at least 2 channels for X and Y control
        int channelCount = m_aoCtrl->getChannelCount();
        qDebug() << "Device reports" << channelCount << "channels";

        if (channelCount < 2) {
            m_lastError = "Device doesn't have enough channels (need at least 2)";
            qDebug() << m_lastError;
            return false;
        }

        // Try to get device info to verify connection
        DeviceInformation currentDevice;
        m_aoCtrl->getSelectedDevice(currentDevice);
        qDebug() << "Connected to device:" << QString::fromWCharArray(currentDevice.Description);

        // Set the voltage range for both channels
        for (int i = 0; i < 2; i++) {
            errCode = m_aoCtrl->getChannels()->getItem(i).setValueRange(V_Neg10To10);
            if (errCode != Success) {
                m_lastError = QString("Failed to set voltage range for channel %1").arg(i);
                qDebug() << m_lastError;
                return false;
            }
        }

        qDebug() << "Voltage ranges set successfully";

        // Initialize to zero position
        double zeroValues[2] = {0.0, 0.0};
        errCode = m_aoCtrl->Write(0, 2, zeroValues);
        if (errCode != Success) {
            m_lastError = QString("Failed to write initial values");
            qDebug() << m_lastError;
            return false;
        }

        qDebug() << "Wrote initial values successfully";

        m_currentVoltages[0] = 0.0;
        m_currentVoltages[1] = 0.0;

        // If we got this far, everything seems good
        qDebug() << "Device opened successfully!";
        return true;
    }
    catch (const std::exception &e) {
        m_lastError = QString("Exception when opening device: %1").arg(e.what());
        qDebug() << m_lastError;
        return false;
    }
}

void FastSteeringMirror::closeDevice()
{
    if (m_aoCtrl && m_aoCtrl->getState() != Idle) {
        qDebug() << "Closing device, setting outputs to zero";
        // Set outputs to zero before closing
        double zeroValues[2] = {0.0, 0.0};
        m_aoCtrl->Write(0, 2, zeroValues);
    }
}

bool FastSteeringMirror::isDeviceOpen() const
{
    if (!m_initialized || !m_aoCtrl) {
        return false;
    }

    try {
        // Just try to get the channel count - if this works, the device is likely connected
        int channelCount = m_aoCtrl->getChannelCount();
        return channelCount > 0;
    }
    catch (...) {
        return false;
    }
}

bool FastSteeringMirror::setPosition(double xPosition, double yPosition)
{
    if (!isDeviceOpen()) {
        m_lastError = "Device not open";
        qDebug() << "Cannot set position:" << m_lastError;
        return false;
    }

    // Clamp values to range -1.0 to 1.0
    xPosition = qBound(-1.0, xPosition, 1.0);
    yPosition = qBound(-1.0, yPosition, 1.0);

    // Convert to voltage
    double voltages[2];
    voltages[0] = positionToVoltage(xPosition);
    voltages[1] = positionToVoltage(yPosition);

    qDebug() << "Setting position:" << xPosition << yPosition;
    qDebug() << "Corresponding voltages:" << voltages[0] << voltages[1];

    // Write to device
    ErrorCode errCode = m_aoCtrl->Write(0, 2, voltages);
    if (errCode != Success) {
        checkError(errCode);
        return false;
    }

    // Update current voltages
    m_currentVoltages[0] = voltages[0];
    m_currentVoltages[1] = voltages[1];

    // Emit signal
    emit positionChanged(xPosition, yPosition);

    return true;
}

QPair<double, double> FastSteeringMirror::getCurrentVoltages() const
{
    return QPair<double, double>(m_currentVoltages[0], m_currentVoltages[1]);
}

bool FastSteeringMirror::setVoltageRange(double minVoltage, double maxVoltage)
{
    if (minVoltage >= maxVoltage) {
        m_lastError = "Invalid voltage range, min must be less than max";
        return false;
    }

    m_minVoltage = minVoltage;
    m_maxVoltage = maxVoltage;
    return true;
}

QString FastSteeringMirror::getLastError() const
{
    return m_lastError;
}

void FastSteeringMirror::checkError(ErrorCode errorCode)
{
    if (errorCode >= 0xE0000000 && errorCode != Success) {
        QString errorMessage = QString("Error Code: 0x%1").arg(
            QString::number(errorCode, 16).right(8).toUpper());

        m_lastError = errorMessage;
        qDebug() << "Device error:" << errorMessage;
        emit deviceError(errorMessage);
    }
}

double FastSteeringMirror::positionToVoltage(double position) const
{
    // Map position (-1.0 to 1.0) to voltage range
    double range = m_maxVoltage - m_minVoltage;
    double center = (m_maxVoltage + m_minVoltage) / 2.0;

    return center + (position * range / 2.0);
}