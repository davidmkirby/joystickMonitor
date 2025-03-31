#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QPair>
#include <QTimer>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QProgressBar>
#include <QLabel>
#include <QVBoxLayout>
#include <QLineEdit>
#include "joystickmanager.h"
#include "faststeeringmirror.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QGroupBox;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Joystick related slots
    void onRefreshJoysticks();
    void onJoystickSelected(int index);
    void updateJoystickList();
    void updateJoystickInfo();
    void onButtonStateChanged(int button, bool pressed);
    void onAxisValueChanged(int axis, int value);
    void onHatValueChanged(int hat, int value);
    void onCalibrateJoystick();

    // Mirror related slots
    void onRefreshMirrorDevices();
    void onMirrorDeviceSelected(int index);
    void updateMirrorDeviceList();
    void onUpdateMirrorPosition();
    void onMirrorPositionChanged(double xPosition, double yPosition);
    void onMirrorDeviceError(const QString &errorMessage);
    void onAxisMappingChanged();
    void onDeadzoneChanged(int value);
    void onInvertAxisToggled(bool checked);
    void onEnableMirrorOutput(bool enabled);

private:
    Ui::MainWindow *ui;
    JoystickManager *m_joystickManager;
    FastSteeringMirror *m_mirrorController;

    // Joystick UI elements
    QVector<QLabel*> m_buttonLabels;
    QVector<QProgressBar*> m_axisProgressBars;
    QVector<QLabel*> m_hatLabels;
    QVBoxLayout *m_inputsLayout;

    // Mirror UI elements
    QComboBox *m_mirrorDeviceComboBox;
    QPushButton *m_refreshMirrorButton;
    QCheckBox *m_enableMirrorCheckbox;
    QComboBox *m_xAxisComboBox;
    QComboBox *m_yAxisComboBox;
    QCheckBox *m_invertXCheckbox;
    QCheckBox *m_invertYCheckbox;
    QSpinBox *m_deadzoneSpinBox;
    QVBoxLayout *m_mirrorStatusLayout;
    QLineEdit *m_profilePathEdit;  // Added for XML profile support

    // Mirror status UI
    QLabel *m_mirrorXLabel;
    QLabel *m_mirrorYLabel;
    QProgressBar *m_mirrorXBar;
    QProgressBar *m_mirrorYBar;

    // State variables
    int m_selectedJoystickIndex;
    bool m_mirrorOutputEnabled;
    int m_xAxisIndex;
    int m_yAxisIndex;
    bool m_invertXAxis;
    bool m_invertYAxis;
    double m_deadzone;
    QTimer *m_updateTimer;

    void createJoystickInputsUI();
    void clearJoystickInputsUI();
    void createMirrorControlUI();
    QString hatValueToString(int value);

    // Helper method to map joystick axis value to mirror position
    double mapAxisToPosition(int axisValue);
};
#endif // MAINWINDOW_H