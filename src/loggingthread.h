#ifndef LOGGINGTHREAD_H
#define LOGGINGTHREAD_H

#include <QThread>
#include <QFile>
#include <QTextStream>
#include <QMutex>
#include <QWaitCondition>
#include "logrecord.h"

class LoggingThread : public QThread
{
    Q_OBJECT
public:
    explicit LoggingThread(QObject *parent = nullptr);
    ~LoggingThread();

    void startLogging(const QString& filename);
    void stopLogging();
    void addRecord(const LogRecord& record);
    bool isLogging() const { return m_isLogging; }

protected:
    void run() override;

private:
    QFile* m_logFile;
    QTextStream* m_logStream;
    QVector<LogRecord> m_buffer;
    QMutex m_mutex;
    QWaitCondition m_condition;
    bool m_isLogging;
    bool m_shouldStop;
};

#endif // LOGGINGTHREAD_H