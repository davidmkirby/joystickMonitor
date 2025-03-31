#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QScrollArea>
#include <QMessageBox>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QTabWidget>
#include <QFileDialog>
#include <QLineEdit>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_joystickManager(new JoystickManager(this))
    , m_mirrorController(new FastSteeringMirror(this))
    , m_selectedJoystickIndex(-1)
    , m_mirrorOutputEnabled(false)
    , m_xAxisIndex(0)
    , m_yAxisIndex(1)
    , m_invertXAxis(false)
    , m_invertYAxis(false)
    , m_deadzone(0.05)  // 5% deadzone
    , m_updateTimer(new QTimer(this))
{
    ui->setupUi(this);

    // Set window title
    setWindowTitle("Joystick Mirror Controller");

    // Create a tab widget if not already in the UI
    QTabWidget *tabWidget = new QTabWidget(this);
    setCentralWidget(tabWidget);

    // Create joystick tab
    QWidget *joystickTab = new QWidget();
    QVBoxLayout *joystickLayout = new QVBoxLayout(joystickTab);

    // Add joystick selection controls
    QHBoxLayout *joystickSelectionLayout = new QHBoxLayout();
    joystickSelectionLayout->addWidget(new QLabel("Select Joystick:"));

    // Create joystick combo box
    QComboBox *joystickComboBox = new QComboBox();
    joystickSelectionLayout->addWidget(joystickComboBox);
    ui->joystickComboBox = joystickComboBox;

    // Create refresh button
    QPushButton *refreshButton = new QPushButton("Refresh");
    joystickSelectionLayout->addWidget(refreshButton);
    ui->refreshButton = refreshButton;

    // Create calibrate button
    QPushButton *calibrateButton = new QPushButton("Calibrate");
    joystickSelectionLayout->addWidget(calibrateButton);

    joystickLayout->addLayout(joystickSelectionLayout);

    // Add joystick info label
    QLabel *joystickInfoLabel = new QLabel("No joystick selected");
    joystickLayout->addWidget(joystickInfoLabel);
    ui->joystickInfoLabel = joystickInfoLabel;

    // Add joystick inputs scroll area
    QScrollArea *inputsScrollArea = new QScrollArea();
    inputsScrollArea->setWidgetResizable(true);
    QWidget *inputsScrollAreaContents = new QWidget();
    m_inputsLayout = new QVBoxLayout(inputsScrollAreaContents);
    inputsScrollArea->setWidget(inputsScrollAreaContents);
    joystickLayout->addWidget(inputsScrollArea);

    // Create mirror control tab
    QWidget *mirrorTab = new QWidget();
    QVBoxLayout *mirrorLayout = new QVBoxLayout(mirrorTab);

    // Add mirror device selection controls
    QHBoxLayout *mirrorSelectionLayout = new QHBoxLayout();
    mirrorSelectionLayout->addWidget(new QLabel("Select Mirror Device:"));

    // Create mirror device combo box
    m_mirrorDeviceComboBox = new QComboBox();
    mirrorSelectionLayout->addWidget(m_mirrorDeviceComboBox);

    // Create refresh mirror button
    m_refreshMirrorButton = new QPushButton("Refresh");
    mirrorSelectionLayout->addWidget(m_refreshMirrorButton);
    mirrorLayout->addLayout(mirrorSelectionLayout);

    // Add profile selection controls
    QGroupBox *profileGroup = new QGroupBox("Device Profile");
    QVBoxLayout *profileLayout = new QVBoxLayout(profileGroup);

    QHBoxLayout *fileSelectionLayout = new QHBoxLayout();

    // Add label
    fileSelectionLayout->addWidget(new QLabel("XML Profile:"));

    // Create profile path line edit
    m_profilePathEdit = new QLineEdit();
    m_profilePathEdit->setPlaceholderText("Select XML profile file...");
    fileSelectionLayout->addWidget(m_profilePathEdit);

    // Create browse button
    QPushButton *browseProfileButton = new QPushButton("Browse");
    fileSelectionLayout->addWidget(browseProfileButton);

    // Add the layout to the group
    profileLayout->addLayout(fileSelectionLayout);

    // Add a description label
    QLabel *descriptionLabel = new QLabel("XML profiles configure device-specific settings including voltage ranges.");
    descriptionLabel->setWordWrap(true);
    profileLayout->addWidget(descriptionLabel);

    // Add the group to the mirror controls
    mirrorLayout->addWidget(profileGroup);

    // Add mirror enable checkbox
    m_enableMirrorCheckbox = new QCheckBox("Enable Mirror Output");
    mirrorLayout->addWidget(m_enableMirrorCheckbox);

    // Create axis mapping controls
    QGroupBox *mappingGroup = new QGroupBox("Joystick to Mirror Mapping");
    QGridLayout *mappingLayout = new QGridLayout(mappingGroup);

    mappingLayout->addWidget(new QLabel("X-Axis:"), 0, 0);
    m_xAxisComboBox = new QComboBox();
    mappingLayout->addWidget(m_xAxisComboBox, 0, 1);
    m_invertXCheckbox = new QCheckBox("Invert");
    mappingLayout->addWidget(m_invertXCheckbox, 0, 2);

    mappingLayout->addWidget(new QLabel("Y-Axis:"), 1, 0);
    m_yAxisComboBox = new QComboBox();
    mappingLayout->addWidget(m_yAxisComboBox, 1, 1);
    m_invertYCheckbox = new QCheckBox("Invert");
    mappingLayout->addWidget(m_invertYCheckbox, 1, 2);

    mappingLayout->addWidget(new QLabel("Deadzone:"), 2, 0);
    m_deadzoneSpinBox = new QSpinBox();
    m_deadzoneSpinBox->setRange(0, 50);
    m_deadzoneSpinBox->setValue(5);
    m_deadzoneSpinBox->setSuffix("%");
    mappingLayout->addWidget(m_deadzoneSpinBox, 2, 1);

    mirrorLayout->addWidget(mappingGroup);

    // Create placeholder for mirror status UI
    QWidget *mirrorStatusWidget = new QWidget();
    m_mirrorStatusLayout = new QVBoxLayout(mirrorStatusWidget);
    mirrorLayout->addWidget(mirrorStatusWidget);

    // Add a stretch to keep UI elements at the top
    mirrorLayout->addStretch();

    // Add tabs to tab widget
    tabWidget->addTab(joystickTab, "Joystick Input");
    tabWidget->addTab(mirrorTab, "Mirror Control");

    // Initialize mirror controller
    if (!m_mirrorController->initialize()) {
        QMessageBox::warning(this, "Mirror Initialization Error",
            "Failed to initialize mirror controller: " + m_mirrorController->getLastError());
    }

    // Initialize joystick manager
    m_joystickManager->initialize();

    // Connect signals and slots
    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::onRefreshJoysticks);
    connect(joystickComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onJoystickSelected);
    connect(calibrateButton, &QPushButton::clicked, this, &MainWindow::onCalibrateJoystick);

    connect(m_refreshMirrorButton, &QPushButton::clicked, this, &MainWindow::onRefreshMirrorDevices);
    connect(m_mirrorDeviceComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onMirrorDeviceSelected);
    connect(m_enableMirrorCheckbox, &QCheckBox::toggled, this, &MainWindow::onEnableMirrorOutput);

    connect(m_xAxisComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onAxisMappingChanged);
    connect(m_yAxisComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onAxisMappingChanged);
    connect(m_invertXCheckbox, &QCheckBox::toggled, this, &MainWindow::onInvertAxisToggled);
    connect(m_invertYCheckbox, &QCheckBox::toggled, this, &MainWindow::onInvertAxisToggled);
    connect(m_deadzoneSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::onDeadzoneChanged);

    connect(m_joystickManager, &JoystickManager::joysticksChanged,
            this, &MainWindow::updateJoystickList);
    connect(m_joystickManager, &JoystickManager::buttonChanged,
            this, &MainWindow::onButtonStateChanged);
    connect(m_joystickManager, &JoystickManager::axisChanged,
            this, &MainWindow::onAxisValueChanged);
    connect(m_joystickManager, &JoystickManager::hatChanged,
            this, &MainWindow::onHatValueChanged);

    connect(m_mirrorController, &FastSteeringMirror::positionChanged,
            this, &MainWindow::onMirrorPositionChanged);
    connect(m_mirrorController, &FastSteeringMirror::deviceError,
            this, &MainWindow::onMirrorDeviceError);

    // Connect browse button for profile
    connect(browseProfileButton, &QPushButton::clicked, this, [this]() {
        QString filePath = QFileDialog::getOpenFileName(this,
            "Select XML Profile", "", "XML Files (*.xml)");

        if (!filePath.isEmpty()) {
            m_profilePathEdit->setText(filePath);

            // If device is already open, try to load the profile
            if (m_mirrorController->isDeviceOpen()) {
                if (m_mirrorController->loadProfile(filePath)) {
                    QMessageBox::information(this, "Profile Loaded",
                        "Device profile loaded successfully.");
                } else {
                    QMessageBox::warning(this, "Profile Error",
                        "Failed to load profile: " + m_mirrorController->getLastError());
                }
            } else {
                // Save for later when device is opened
                m_mirrorController->setProfilePath(filePath);
            }
        }
    });

    // Configure timer for mirror updates
    connect(m_updateTimer, &QTimer::timeout, this, &MainWindow::onUpdateMirrorPosition);
    m_updateTimer->setInterval(16);  // ~60Hz updates

    // Initial updates
    updateJoystickList();
    updateMirrorDeviceList();
    createMirrorControlUI();
}

MainWindow::~MainWindow()
{
    m_updateTimer->stop();
    m_joystickManager->cleanup();
    m_mirrorController->cleanup();
    delete ui;
}

// Joystick related methods
void MainWindow::onRefreshJoysticks()
{
    m_joystickManager->refreshJoysticks();
}

void MainWindow::onJoystickSelected(int index)
{
    if (index < 0) {
        m_joystickManager->closeJoystick();
        m_selectedJoystickIndex = -1;
        clearJoystickInputsUI();

        // Update axis mapping comboboxes
        m_xAxisComboBox->clear();
        m_yAxisComboBox->clear();
        return;
    }

    m_selectedJoystickIndex = m_joystickManager->openJoystick(index);
    if (m_selectedJoystickIndex >= 0) {
        updateJoystickInfo();

        // Update axis mapping comboboxes
        m_xAxisComboBox->clear();
        m_yAxisComboBox->clear();

        int numAxes = m_joystickManager->getNumAxes();
        for (int i = 0; i < numAxes; ++i) {
            QString axisName = QString("Axis %1").arg(i);
            m_xAxisComboBox->addItem(axisName, i);
            m_yAxisComboBox->addItem(axisName, i);
        }

        // Set default mappings
        if (numAxes >= 2) {
            m_xAxisComboBox->setCurrentIndex(0);  // First axis (X)
            m_yAxisComboBox->setCurrentIndex(1);  // Second axis (Y)
        }
    }
}

void MainWindow::updateJoystickList()
{
    ui->joystickComboBox->clear();

    QStringList joysticks = m_joystickManager->getAvailableJoysticks();
    if (joysticks.isEmpty()) {
        ui->joystickComboBox->addItem("No joysticks found");
        ui->joystickComboBox->setEnabled(false);
    } else {
        ui->joystickComboBox->addItems(joysticks);
        ui->joystickComboBox->setEnabled(true);
    }
}

void MainWindow::updateJoystickInfo()
{
    if (!m_joystickManager->isJoystickOpen()) {
        ui->joystickInfoLabel->setText("No joystick selected");
        clearJoystickInputsUI();
        return;
    }

    QString infoText = QString("%1\n")
                        .arg(m_joystickManager->getJoystickName());

    infoText += QString("Buttons: %1\n")
                .arg(m_joystickManager->getNumButtons());

    infoText += QString("Axes: %1\n")
                .arg(m_joystickManager->getNumAxes());

    infoText += QString("Hats: %1")
                .arg(m_joystickManager->getNumHats());

    ui->joystickInfoLabel->setText(infoText);

    createJoystickInputsUI();
}

void MainWindow::createJoystickInputsUI()
{
    clearJoystickInputsUI();

    // Create buttons UI
    int numButtons = m_joystickManager->getNumButtons();
    if (numButtons > 0) {
        QGroupBox *buttonGroup = new QGroupBox("Buttons");
        QGridLayout *buttonLayout = new QGridLayout(buttonGroup);

        const int buttonsPerRow = 8;

        for (int i = 0; i < numButtons; ++i) {
            QLabel *buttonLabel = new QLabel(QString::number(i));
            buttonLabel->setAlignment(Qt::AlignCenter);
            buttonLabel->setFixedSize(30, 30);

            // Special styling for button 3 (mirror toggle)
            if (i == 3) {
                buttonLabel->setStyleSheet("background-color: lightblue; border: 1px solid gray;");
                buttonLabel->setToolTip("Press to toggle mirror output");
            } else {
                buttonLabel->setStyleSheet("background-color: lightgray; border: 1px solid gray;");
            }

            buttonLayout->addWidget(buttonLabel, i / buttonsPerRow, i % buttonsPerRow);
            m_buttonLabels.append(buttonLabel);
        }

        m_inputsLayout->addWidget(buttonGroup);
    }

    // Create axes UI
    int numAxes = m_joystickManager->getNumAxes();
    if (numAxes > 0) {
        QGroupBox *axesGroup = new QGroupBox("Axes");
        QVBoxLayout *axesLayout = new QVBoxLayout(axesGroup);

        for (int i = 0; i < numAxes; ++i) {
            QHBoxLayout *axisLayout = new QHBoxLayout();
            QLabel *axisLabel = new QLabel(QString("Axis %1").arg(i));
            QProgressBar *axisBar = new QProgressBar();

            axisBar->setRange(-32768, 32767);
            axisBar->setValue(0);
            axisBar->setTextVisible(true);
            axisBar->setFormat("%v");

            axisLayout->addWidget(axisLabel);
            axisLayout->addWidget(axisBar);

            axesLayout->addLayout(axisLayout);
            m_axisProgressBars.append(axisBar);
        }

        m_inputsLayout->addWidget(axesGroup);
    }

    // Create hats UI
    int numHats = m_joystickManager->getNumHats();
    if (numHats > 0) {
        QGroupBox *hatsGroup = new QGroupBox("Hats (POV)");
        QVBoxLayout *hatsLayout = new QVBoxLayout(hatsGroup);

        for (int i = 0; i < numHats; ++i) {
            QHBoxLayout *hatLayout = new QHBoxLayout();
            QLabel *hatTitleLabel = new QLabel(QString("Hat %1").arg(i));
            QLabel *hatValueLabel = new QLabel("Centered");

            hatValueLabel->setAlignment(Qt::AlignCenter);
            hatValueLabel->setFixedWidth(100);
            hatValueLabel->setStyleSheet("border: 1px solid gray;");

            hatLayout->addWidget(hatTitleLabel);
            hatLayout->addWidget(hatValueLabel);

            hatsLayout->addLayout(hatLayout);
            m_hatLabels.append(hatValueLabel);
        }

        m_inputsLayout->addWidget(hatsGroup);
    }

    // Add a note about the special button functionality
    QLabel *specialButtonLabel = new QLabel("Button 3: Toggle mirror output");
    specialButtonLabel->setStyleSheet("color: blue; font-weight: bold;");
    m_inputsLayout->addWidget(specialButtonLabel);

    // Add a stretch to keep UI elements at the top
    m_inputsLayout->addStretch();
}

void MainWindow::clearJoystickInputsUI()
{
    // Clear existing input widgets
    m_buttonLabels.clear();
    m_axisProgressBars.clear();
    m_hatLabels.clear();

    // Delete all widgets in the inputs layout
    QLayoutItem *child;
    while ((child = m_inputsLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            delete child->widget();
        }
        delete child;
    }
}

void MainWindow::onButtonStateChanged(int button, bool pressed)
{
    if (button < 0 || button >= m_buttonLabels.size()) return;

    QLabel *buttonLabel = m_buttonLabels[button];

    // Change the button appearance based on its state
    if (pressed) {
        buttonLabel->setStyleSheet("background-color: green; border: 1px solid gray;");

        // Toggle mirror output when button 3 is pressed (only on press, not release)
        if (button == 3) {
            // Get current state and toggle it
            bool currentState = m_enableMirrorCheckbox->isChecked();
            m_enableMirrorCheckbox->setChecked(!currentState);

            // The checkbox's toggled signal will automatically call onEnableMirrorOutput
        }
    } else {
        // For button 3, use a special color to indicate it's the toggle button
        if (button == 3) {
            buttonLabel->setStyleSheet("background-color: lightblue; border: 1px solid gray;");
        } else {
            buttonLabel->setStyleSheet("background-color: lightgray; border: 1px solid gray;");
        }
    }
}

void MainWindow::onAxisValueChanged(int axis, int value)
{
    if (axis < 0 || axis >= m_axisProgressBars.size()) return;

    QProgressBar *axisBar = m_axisProgressBars[axis];
    axisBar->setValue(value);
}

void MainWindow::onHatValueChanged(int hat, int value)
{
    if (hat < 0 || hat >= m_hatLabels.size()) return;

    QLabel *hatLabel = m_hatLabels[hat];
    hatLabel->setText(hatValueToString(value));
}

void MainWindow::onCalibrateJoystick()
{
    if (m_joystickManager->isJoystickOpen()) {
        m_joystickManager->calibrateAxes();
        QMessageBox::information(this, "Calibration",
            "Joystick has been calibrated. Current position is now the center.");
    } else {
        QMessageBox::warning(this, "Calibration Error",
            "No joystick is currently selected. Please select a joystick first.");
    }
}

QString MainWindow::hatValueToString(int value)
{
    // Convert SDL hat values to readable strings
    switch (value) {
        case SDL_HAT_CENTERED: return "Centered";
        case SDL_HAT_UP: return "Up";
        case SDL_HAT_RIGHT: return "Right";
        case SDL_HAT_DOWN: return "Down";
        case SDL_HAT_LEFT: return "Left";
        case SDL_HAT_RIGHTUP: return "Right+Up";
        case SDL_HAT_RIGHTDOWN: return "Right+Down";
        case SDL_HAT_LEFTUP: return "Left+Up";
        case SDL_HAT_LEFTDOWN: return "Left+Down";
        default: return "Unknown";
    }
}

// Mirror related methods
void MainWindow::onRefreshMirrorDevices()
{
    updateMirrorDeviceList();
}

void MainWindow::updateMirrorDeviceList()
{
    m_mirrorDeviceComboBox->clear();

    QStringList devices = m_mirrorController->getAvailableDevices();
    if (devices.isEmpty()) {
        m_mirrorDeviceComboBox->addItem("No compatible devices found");
        m_mirrorDeviceComboBox->setEnabled(false);
        m_enableMirrorCheckbox->setEnabled(false);
    } else {
        m_mirrorDeviceComboBox->addItems(devices);
        m_mirrorDeviceComboBox->setEnabled(true);
    }
}

void MainWindow::onMirrorDeviceSelected(int index)
{
    if (index < 0 || !m_mirrorDeviceComboBox->isEnabled()) {
        m_enableMirrorCheckbox->setEnabled(false);
        m_enableMirrorCheckbox->setChecked(false);
        m_mirrorOutputEnabled = false;
        m_updateTimer->stop();
        m_mirrorController->closeDevice();
        return;
    }

    QString deviceName = m_mirrorDeviceComboBox->currentText();

    // Check if we have a profile path set
    QString profilePath = m_profilePathEdit->text();
    if (!profilePath.isEmpty()) {
        m_mirrorController->setProfilePath(profilePath);
    }

    if (m_mirrorController->openDevice(deviceName)) {
        m_enableMirrorCheckbox->setEnabled(true);
    } else {
        m_enableMirrorCheckbox->setEnabled(false);
        m_enableMirrorCheckbox->setChecked(false);
        m_mirrorOutputEnabled = false;
        m_updateTimer->stop();
        QMessageBox::warning(this, "Device Error",
            "Failed to open mirror device: " + m_mirrorController->getLastError());
    }
}

void MainWindow::createMirrorControlUI()
{
    // Clear existing mirror status widgets
    QLayoutItem *child;
    while ((child = m_mirrorStatusLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            delete child->widget();
        }
        delete child;
    }

    // Create mirror status UI
    QGroupBox *statusGroup = new QGroupBox("Mirror Status");
    QVBoxLayout *statusLayout = new QVBoxLayout(statusGroup);

    // X position
    QHBoxLayout *xLayout = new QHBoxLayout();
    QLabel *xTitleLabel = new QLabel("X Position:");
    m_mirrorXBar = new QProgressBar();
    m_mirrorXBar->setRange(-100, 100);
    m_mirrorXBar->setValue(0);
    m_mirrorXBar->setTextVisible(false);
    m_mirrorXLabel = new QLabel("0.00 V");

    xLayout->addWidget(xTitleLabel);
    xLayout->addWidget(m_mirrorXBar);
    xLayout->addWidget(m_mirrorXLabel);
    statusLayout->addLayout(xLayout);

    // Y position
    QHBoxLayout *yLayout = new QHBoxLayout();
    QLabel *yTitleLabel = new QLabel("Y Position:");
    m_mirrorYBar = new QProgressBar();
    m_mirrorYBar->setRange(-100, 100);
    m_mirrorYBar->setValue(0);
    m_mirrorYBar->setTextVisible(false);
    m_mirrorYLabel = new QLabel("0.00 V");

    yLayout->addWidget(yTitleLabel);
    yLayout->addWidget(m_mirrorYBar);
    yLayout->addWidget(m_mirrorYLabel);
    statusLayout->addLayout(yLayout);

    m_mirrorStatusLayout->addWidget(statusGroup);
}

void MainWindow::onEnableMirrorOutput(bool enabled)
{
    if (enabled) {
        // Try to verify if the device is really connected before enabling output
        if (!m_mirrorController->isDeviceOpen()) {
            // Try to reconnect the device
            QString deviceName = m_mirrorDeviceComboBox->currentText();
            if (!deviceName.isEmpty()) {
                qDebug() << "Attempting to reconnect to device:" << deviceName;
                if (m_mirrorController->openDevice(deviceName)) {
                    qDebug() << "Successfully reconnected to device!";
                } else {
                    qDebug() << "Failed to reconnect to device:" << m_mirrorController->getLastError();
                    m_enableMirrorCheckbox->setChecked(false);
                    m_mirrorOutputEnabled = false;
                    QMessageBox::warning(this, "Device Connection Error",
                        "Could not connect to mirror device: " + m_mirrorController->getLastError());
                    return;
                }
            }
        }

        // Double-check that we have a connection before continuing
        if (!m_mirrorController->isDeviceOpen()) {
            m_enableMirrorCheckbox->setChecked(false);
            m_mirrorOutputEnabled = false;
            QMessageBox::warning(this, "Enable Error",
                "Cannot enable mirror output: No mirror device connected. Please check hardware connections and permissions.");
            return;
        }

        // Also check the joystick
        if (!m_joystickManager->isJoystickOpen()) {
            m_enableMirrorCheckbox->setChecked(false);
            m_mirrorOutputEnabled = false;
            QMessageBox::warning(this, "Enable Error",
                "Cannot enable mirror output: No joystick connected.");
            return;
        }

        // All checks passed, enable output
        m_mirrorOutputEnabled = true;
        m_updateTimer->start();
        qDebug() << "Mirror output enabled successfully!";
    } else {
        // Stop the update timer when mirror output is disabled
        m_updateTimer->stop();
        m_mirrorOutputEnabled = false;

        // Reset mirror position to center
        if (m_mirrorController->isDeviceOpen()) {
            m_mirrorController->setPosition(0.0, 0.0);
        }

        qDebug() << "Mirror output disabled.";
    }
}

void MainWindow::onUpdateMirrorPosition()
{
    if (!m_mirrorOutputEnabled || !m_joystickManager->isJoystickOpen() || !m_mirrorController->isDeviceOpen()) {
        m_updateTimer->stop();
        m_enableMirrorCheckbox->setChecked(false);
        m_mirrorOutputEnabled = false;
        return;
    }

    // Get axis values from mapping
    int numAxes = m_joystickManager->getNumAxes();
    if (m_xAxisIndex >= numAxes || m_yAxisIndex >= numAxes) {
        return;
    }

    // Get the current joystick positions for X and Y axes
    int xAxisValue = 0;
    int yAxisValue = 0;

    // Find the appropriate axis values from the UI
    for (int i = 0; i < m_axisProgressBars.size(); i++) {
        if (i == m_xAxisIndex) {
            xAxisValue = m_axisProgressBars[i]->value();
        }
        if (i == m_yAxisIndex) {
            yAxisValue = m_axisProgressBars[i]->value();
        }
    }

    // Map joystick values to mirror positions
    double xPosition = mapAxisToPosition(xAxisValue);
    double yPosition = mapAxisToPosition(yAxisValue);

    // Apply inversion if needed
    if (m_invertXAxis) xPosition = -xPosition;
    if (m_invertYAxis) yPosition = -yPosition;

    // Update the mirror position
    m_mirrorController->setPosition(xPosition, yPosition);
}

double MainWindow::mapAxisToPosition(int axisValue)
{
    // Convert joystick axis value (-32768 to 32767) to position (-1.0 to 1.0)
    double normalizedValue = axisValue / 32768.0;

    // Apply deadzone
    if (qAbs(normalizedValue) < m_deadzone) {
        return 0.0;
    }

    // Rescale the remaining range to still use the full -1 to 1 output range
    if (normalizedValue > 0) {
        normalizedValue = (normalizedValue - m_deadzone) / (1.0 - m_deadzone);
    } else {
        normalizedValue = (normalizedValue + m_deadzone) / (1.0 - m_deadzone);
    }

    return qBound(-1.0, normalizedValue, 1.0);
}

void MainWindow::onMirrorPositionChanged(double xPosition, double yPosition)
{
    // Get the actual voltage values
    QPair<double, double> voltages = m_mirrorController->getCurrentVoltages();

    // Update UI
    m_mirrorXBar->setValue(int(xPosition * 100));
    m_mirrorYBar->setValue(int(yPosition * 100));

    m_mirrorXLabel->setText(QString("%1 V").arg(voltages.first, 0, 'f', 2));
    m_mirrorYLabel->setText(QString("%1 V").arg(voltages.second, 0, 'f', 2));
}

void MainWindow::onMirrorDeviceError(const QString &errorMessage)
{
    QMessageBox::critical(this, "Mirror Device Error", errorMessage);

    // Disable mirror output on error
    m_enableMirrorCheckbox->setChecked(false);
    m_mirrorOutputEnabled = false;
    m_updateTimer->stop();
}

void MainWindow::onAxisMappingChanged()
{
    if (m_xAxisComboBox->count() == 0 || m_yAxisComboBox->count() == 0) {
        return;
    }

    m_xAxisIndex = m_xAxisComboBox->currentData().toInt();
    m_yAxisIndex = m_yAxisComboBox->currentData().toInt();
}

void MainWindow::onDeadzoneChanged(int value)
{
    m_deadzone = value / 100.0;
}

void MainWindow::onInvertAxisToggled(bool checked)
{
    QObject *sender = QObject::sender();

    if (sender == m_invertXCheckbox) {
        m_invertXAxis = checked;
    } else if (sender == m_invertYCheckbox) {
        m_invertYAxis = checked;
    }
}