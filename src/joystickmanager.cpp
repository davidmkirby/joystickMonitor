#include "joystickmanager.h"
#include <QDebug>

JoystickManager::JoystickManager(QObject *parent)
    : QObject(parent)
    , m_pollTimer(new QTimer(this))
    , m_currentJoystick(nullptr)
    , m_sdlInitialized(false)
{
    m_pollTimer->setInterval(16); // ~60Hz polling
    connect(m_pollTimer, &QTimer::timeout, this, &JoystickManager::pollEvents);
}

JoystickManager::~JoystickManager()
{
    cleanup();
}

void JoystickManager::initialize()
{
    if (SDL_Init(SDL_INIT_JOYSTICK) < 0) {
        qWarning() << "SDL could not initialize! SDL Error:" << SDL_GetError();
        return;
    }
    
    m_sdlInitialized = true;
    
    // Enable joystick events
    SDL_JoystickEventState(SDL_ENABLE);
    
    scanJoysticks();
    m_pollTimer->start();
}

void JoystickManager::cleanup()
{
    m_pollTimer->stop();
    
    closeJoystick();
    
    if (m_sdlInitialized) {
        SDL_Quit();
        m_sdlInitialized = false;
    }
}

void JoystickManager::scanJoysticks()
{
    m_availableJoysticks.clear();
    
    int numJoysticks = SDL_NumJoysticks();
    for (int i = 0; i < numJoysticks; i++) {
        const char* name = SDL_JoystickNameForIndex(i);
        if (name) {
            m_availableJoysticks[i] = QString::fromUtf8(name);
        } else {
            m_availableJoysticks[i] = QString("Joystick %1").arg(i);
        }
    }
    
    emit joysticksChanged();
}

QStringList JoystickManager::getAvailableJoysticks() const
{
    QStringList result;
    for (auto it = m_availableJoysticks.constBegin(); it != m_availableJoysticks.constEnd(); ++it) {
        result << it.value();
    }
    return result;
}

int JoystickManager::openJoystick(int index)
{
    closeJoystick();
    
    m_currentJoystick = SDL_JoystickOpen(index);
    if (!m_currentJoystick) {
        qWarning() << "Couldn't open joystick" << index << ":" << SDL_GetError();
        return -1;
    }
    
    // Initialize calibration when opening joystick
    calibrateAxes();
    
    return index;
}

void JoystickManager::closeJoystick()
{
    if (m_currentJoystick) {
        SDL_JoystickClose(m_currentJoystick);
        m_currentJoystick = nullptr;
    }
    
    // Clear calibration data
    m_axisCalibration.clear();
}

bool JoystickManager::isJoystickOpen() const
{
    return m_currentJoystick != nullptr;
}

int JoystickManager::getNumAxes() const
{
    if (!m_currentJoystick) return 0;
    return SDL_JoystickNumAxes(m_currentJoystick);
}

int JoystickManager::getNumButtons() const
{
    if (!m_currentJoystick) return 0;
    return SDL_JoystickNumButtons(m_currentJoystick);
}

int JoystickManager::getNumHats() const
{
    if (!m_currentJoystick) return 0;
    return SDL_JoystickNumHats(m_currentJoystick);
}

QString JoystickManager::getJoystickName() const
{
    if (!m_currentJoystick) return QString();
    const char* name = SDL_JoystickName(m_currentJoystick);
    if (name) {
        return QString::fromUtf8(name);
    }
    return QString("Unknown Joystick");
}

void JoystickManager::refreshJoysticks()
{
    scanJoysticks();
}

// New calibration method
void JoystickManager::calibrateAxes()
{
    if (!m_currentJoystick) return;
    
    int numAxes = SDL_JoystickNumAxes(m_currentJoystick);
    m_axisCalibration.clear();
    
    // Initialize calibration with current values as center
    for (int i = 0; i < numAxes; ++i) {
        int currentValue = SDL_JoystickGetAxis(m_currentJoystick, i);
        AxisCalibration cal;
        cal.min = -32768;
        cal.max = 32767;
        cal.center = currentValue; // Use current position as center
        cal.calibrated = true;
        m_axisCalibration.append(cal);
        
        qDebug() << "Calibrated axis" << i << "center:" << cal.center;
    }
}

void JoystickManager::pollEvents()
{
    if (!m_sdlInitialized) return;
    
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_JOYAXISMOTION:
                {
                    int axis = event.jaxis.axis;
                    int rawValue = event.jaxis.value;
                    
                    // Apply calibration if available
                    int adjustedValue = rawValue;
                    if (axis < m_axisCalibration.size() && m_axisCalibration[axis].calibrated) {
                        adjustedValue = rawValue - m_axisCalibration[axis].center;
                        
                        // Clamp to valid range
                        if (adjustedValue < -32768) adjustedValue = -32768;
                        if (adjustedValue > 32767) adjustedValue = 32767;
                    }
                    
                    emit axisChanged(axis, adjustedValue);
                }
                break;
            
            case SDL_JOYBUTTONDOWN:
                emit buttonChanged(event.jbutton.button, true);
                break;
            
            case SDL_JOYBUTTONUP:
                emit buttonChanged(event.jbutton.button, false);
                break;
            
            case SDL_JOYHATMOTION:
                emit hatChanged(event.jhat.hat, event.jhat.value);
                break;
            
            case SDL_JOYDEVICEADDED:
            case SDL_JOYDEVICEREMOVED:
                scanJoysticks();
                break;
        }
    }
}