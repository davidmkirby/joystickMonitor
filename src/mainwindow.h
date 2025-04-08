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
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>
#include <QtMath>
#include "joystickmanager.h"
#include "faststeeringmirror.h"
// Add tracker-related includes
#include "trackermemory.h"
#include "trackdata.h"
#include "logger.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QGroupBox;
class QTabWidget;

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

    // Sine wave testing slots
    void onStartStopSineWave();
    void onUpdateSineWave();
    void onBrowseLogFile();
    void onStartStopLogging();
    void onFrequencyChanged(int value);
    void onAmplitudeChanged(double value);
    void onXAxisToggled(bool checked);
    void onYAxisToggled(bool checked);
    void onPhaseOffsetChanged(int value);

    // Tracker related slots
    void onTrackerInitButtonClicked();
    void onTrackerPingButtonClicked();
    void onTrackerStartLoggingButtonClicked();
    void onTrackerStopLoggingButtonClicked();
    void onTrackerAutoPollToggled(bool checked);
    void pollTracker();
    void handleTrackerError(const QString& errorMsg);
    void handleLoggerError(const QString& errorMsg);

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

    // Sine wave test UI elements
    QSpinBox *m_frequencySpinBox;
    QDoubleSpinBox *m_amplitudeSpinBox;
    QSpinBox *m_phaseOffsetSpinBox;
    QPushButton *m_sineWaveButton;
    QCheckBox *m_xAxisCheckBox;
    QCheckBox *m_yAxisCheckBox;
    QPushButton *m_loggingButton;
    QLineEdit *m_logFileEdit;
    QPushButton *m_browseButton;
    QLabel *m_waveformLabel;
    QProgressBar *m_xOutputBar;
    QProgressBar *m_yOutputBar;

    // Sine wave generation
    QTimer *m_sineWaveTimer;
    bool m_sineWaveActive;
    double m_sinePhase;
    double m_sineFrequency;
    double m_sineAmplitude;
    int m_phaseOffset;    // Phase offset between X and Y (in degrees)
    QVector<double> m_xWaveformData;
    QVector<double> m_yWaveformData;
    QVector<QPointF> m_waveformPoints;
    QDateTime m_startTime;

    // Data logging
    QFile *m_logFile;
    QTextStream *m_logStream;
    bool m_loggingActive;

    // State variables
    int m_selectedJoystickIndex;
    bool m_mirrorOutputEnabled;
    int m_xAxisIndex;
    int m_yAxisIndex;
    bool m_invertXAxis;
    bool m_invertYAxis;
    double m_deadzone;
    QTimer *m_updateTimer;

    // Tracker-related members
    TrackerMemory *m_trackerMemory;
    Logger *m_trackerLogger;
    QTimer *m_trackerPollTimer;

    // Tracker UI elements
    QLineEdit *m_rawErrorXLineEdit;
    QLineEdit *m_rawErrorYLineEdit;
    QLineEdit *m_filteredErrorXLineEdit;
    QLineEdit *m_filteredErrorYLineEdit;
    QLineEdit *m_trackStateLineEdit;
    QLineEdit *m_trackModeLineEdit;
    QLineEdit *m_targetPolarityLineEdit;
    QLineEdit *m_statusLineEdit;
    QLabel *m_trackerStatusLabel;
    QPushButton *m_trackerInitButton;
    QPushButton *m_trackerPingButton;
    QPushButton *m_trackerStartLoggingButton;
    QPushButton *m_trackerStopLoggingButton;
    QCheckBox *m_trackerAutoPollCheckBox;

    void createJoystickInputsUI();
    void clearJoystickInputsUI();
    void createMirrorControlUI();
    void createSineWaveTab();
    void createTrackerTab();  // New method for creating tracker tab
    void updateWaveformDisplay();
    void updateTrackerUI(const TrackData& data);
    void setTrackerUIEnabled(bool enabled);
    QString hatValueToString(int value);

    // Helper method to map joystick axis value to mirror position
    double mapAxisToPosition(int axisValue);
};
#endif // MAINWINDOW_H