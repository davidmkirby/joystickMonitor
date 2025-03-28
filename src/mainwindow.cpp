#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QScrollArea>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_joystickManager(new JoystickManager(this))
    , m_selectedJoystickIndex(-1)
{
    ui->setupUi(this);
    
    // Initialize joystick manager
    m_joystickManager->initialize();
    
    // Connect signals and slots
    connect(ui->refreshButton, &QPushButton::clicked, this, &MainWindow::onRefreshJoysticks);
    connect(ui->joystickComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &MainWindow::onJoystickSelected);
    
    // Add calibrate button
    QPushButton *calibrateButton = new QPushButton("Calibrate", this);
    connect(calibrateButton, &QPushButton::clicked, this, &MainWindow::onCalibrateJoystick);
    ui->horizontalLayout->addWidget(calibrateButton);
    
    connect(m_joystickManager, &JoystickManager::joysticksChanged, 
            this, &MainWindow::updateJoystickList);
    connect(m_joystickManager, &JoystickManager::buttonChanged, 
            this, &MainWindow::onButtonStateChanged);
    connect(m_joystickManager, &JoystickManager::axisChanged, 
            this, &MainWindow::onAxisValueChanged);
    connect(m_joystickManager, &JoystickManager::hatChanged, 
            this, &MainWindow::onHatValueChanged);
    
    // Initial update
    updateJoystickList();
}

MainWindow::~MainWindow()
{
    m_joystickManager->cleanup();
    delete ui;
}

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
        return;
    }
    
    m_selectedJoystickIndex = m_joystickManager->openJoystick(index);
    if (m_selectedJoystickIndex >= 0) {
        updateJoystickInfo();
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
        QGroupBox *buttonGroup = new QGroupBox("Buttons", ui->inputsScrollAreaContents);
        QGridLayout *buttonLayout = new QGridLayout(buttonGroup);
        
        const int buttonsPerRow = 8;
        
        for (int i = 0; i < numButtons; ++i) {
            QLabel *buttonLabel = new QLabel(QString::number(i));
            buttonLabel->setAlignment(Qt::AlignCenter);
            buttonLabel->setFixedSize(30, 30);
            buttonLabel->setStyleSheet("background-color: lightgray; border: 1px solid gray;");
            
            buttonLayout->addWidget(buttonLabel, i / buttonsPerRow, i % buttonsPerRow);
            m_buttonLabels.append(buttonLabel);
        }
        
        ui->inputsLayout->addWidget(buttonGroup);
    }
    
    // Create axes UI
    int numAxes = m_joystickManager->getNumAxes();
    if (numAxes > 0) {
        QGroupBox *axesGroup = new QGroupBox("Axes", ui->inputsScrollAreaContents);
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
        
        ui->inputsLayout->addWidget(axesGroup);
    }
    
    // Create hats UI
    int numHats = m_joystickManager->getNumHats();
    if (numHats > 0) {
        QGroupBox *hatsGroup = new QGroupBox("Hats (POV)", ui->inputsScrollAreaContents);
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
        
        ui->inputsLayout->addWidget(hatsGroup);
    }
    
    // Add a stretch to keep UI elements at the top
    ui->inputsLayout->addStretch();
}

void MainWindow::clearJoystickInputsUI()
{
    // Clear existing input widgets
    m_buttonLabels.clear();
    m_axisProgressBars.clear();
    m_hatLabels.clear();
    
    // Delete all widgets in the scroll area
    QLayoutItem *child;
    while ((child = ui->inputsLayout->takeAt(0)) != nullptr) {
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
    buttonLabel->setStyleSheet(pressed ? 
                              "background-color: green; border: 1px solid gray;" : 
                              "background-color: lightgray; border: 1px solid gray;");
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

// New calibration method
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