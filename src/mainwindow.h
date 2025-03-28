#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QPair>
#include "joystickmanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QLabel;
class QProgressBar;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onRefreshJoysticks();
    void onJoystickSelected(int index);
    
    void updateJoystickList();
    void updateJoystickInfo();
    
    void onButtonStateChanged(int button, bool pressed);
    void onAxisValueChanged(int axis, int value);
    void onHatValueChanged(int hat, int value);

    void onCalibrateJoystick();

private:
    Ui::MainWindow *ui;
    JoystickManager *m_joystickManager;
    
    QVector<QLabel*> m_buttonLabels;
    QVector<QProgressBar*> m_axisProgressBars;
    QVector<QLabel*> m_hatLabels;
    
    int m_selectedJoystickIndex;
    
    void createJoystickInputsUI();
    void clearJoystickInputsUI();
    QString hatValueToString(int value);
};
#endif // MAINWINDOW_H
