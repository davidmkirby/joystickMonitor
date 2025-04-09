#include "loggingthread.h"
#include <QDebug>

LoggingThread::LoggingThread(QObject *parent)
    : QThread(parent)
    , m_logFile(nullptr)
    , m_logStream(nullptr)
    , m_isLogging(false)
    , m_shouldStop(false)
{
}

LoggingThread::~LoggingThread()
{
    stopLogging();
}

void LoggingThread::startLogging(const QString& filename)
{
    if (m_isLogging) {
        stopLogging();
    }

    m_logFile = new QFile(filename);
    if (!m_logFile->open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Failed to open log file:" << m_logFile->errorString();
        delete m_logFile;
        m_logFile = nullptr;
        return;
    }

    m_logStream = new QTextStream(m_logFile);
    m_logStream->setRealNumberPrecision(5);
    m_logStream->setRealNumberNotation(QTextStream::FixedNotation);

    // Write header
    *m_logStream << "ElapsedTime(s),Frequency(Hz),Amplitude,X-Command,Y-Command,X-Feedback(V),Y-Feedback(V)" << Qt::endl;

    m_isLogging = true;
    m_shouldStop = false;
    start();
}

void LoggingThread::stopLogging()
{
    if (!m_isLogging) {
        return;
    }

    m_shouldStop = true;
    m_condition.wakeOne();
    wait();

    if (m_logStream) {
        m_logStream->flush();
        delete m_logStream;
        m_logStream = nullptr;
    }

    if (m_logFile) {
        m_logFile->close();
        delete m_logFile;
        m_logFile = nullptr;
    }

    m_isLogging = false;
}

void LoggingThread::addRecord(const LogRecord& record)
{
    if (!m_isLogging) {
        return;
    }

    QMutexLocker locker(&m_mutex);
    m_buffer.append(record);
    m_condition.wakeOne();
}

void LoggingThread::run()
{
    while (!m_shouldStop) {
        QMutexLocker locker(&m_mutex);

        if (m_buffer.isEmpty()) {
            m_condition.wait(&m_mutex);
            if (m_shouldStop) {
                break;
            }
        }

        // Write all records in the buffer
        for (const LogRecord& record : m_buffer) {
            double elapsedSeconds = record.elapsedTime / 1.0e9;
            *m_logStream << QString::number(elapsedSeconds, 'f', 9) << ","
                        << QString::number(record.frequency, 'f', 1) << ","
                        << QString::number(record.amplitude, 'f', 1) << ","
                        << QString::number(record.xCommand, 'f', 5) << ","
                        << QString::number(record.yCommand, 'f', 5) << ","
                        << QString::number(record.xFeedback, 'f', 5) << ","
                        << QString::number(record.yFeedback, 'f', 5) << "\n";
        }

        m_buffer.clear();
        m_logStream->flush();
    }
}