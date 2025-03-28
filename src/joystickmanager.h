#ifndef JOYSTICKMANAGER_H
#define JOYSTICKMANAGER_H

#include <QObject>
#include <QTimer>
#include <QMap>
#include <QVector>
#include <SDL2/SDL.h>

class JoystickManager : public QObject
{
    Q_OBJECT
public:
    explicit JoystickManager(QObject *parent = nullptr);
    ~JoystickManager();

    void initialize();
    void cleanup();
    QStringList getAvailableJoysticks() const;
    int openJoystick(int index);
    void closeJoystick();
    bool isJoystickOpen() const;
    
    int getNumAxes() const;
    int getNumButtons() const;
    int getNumHats() const;
    
    QString getJoystickName() const;
    
    // Add calibration method that can be called externally if needed
    void calibrateAxes();

signals:
    void joysticksChanged();
    void buttonChanged(int button, bool pressed);
    void axisChanged(int axis, int value);
    void hatChanged(int hat, int value);

public slots:
    void refreshJoysticks();
    void pollEvents();

private:
    QTimer *m_pollTimer;
    SDL_Joystick *m_currentJoystick;
    QMap<int, QString> m_availableJoysticks;
    
    bool m_sdlInitialized;
    
    // Add calibration structure
    struct AxisCalibration {
        int min;
        int max;
        int center;
        bool calibrated;
    };
    QVector<AxisCalibration> m_axisCalibration;
    
    void scanJoysticks();
};

#endif // JOYSTICKMANAGER_H