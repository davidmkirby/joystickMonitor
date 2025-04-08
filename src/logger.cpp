#include "logger.h"
#include <QDebug>

Logger::Logger(QObject *parent)
    : QObject(parent)
    , m_isLogging(false)
    , m_headerWritten(false)
{
}

Logger::~Logger()
{
    stopLogging();
}

bool Logger::startLogging(const QString& filename)
{
    if (m_isLogging) {
        stopLogging();
    }

    m_logFile.setFileName(filename);
    if (!m_logFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit errorOccurred("Failed to open log file: " + filename);
        return false;
    }

    m_textStream.setDevice(&m_logFile);

    // Set high precision for floating point values
    m_textStream.setRealNumberPrecision(5); // 5 decimal places for track errors
    m_textStream.setRealNumberNotation(QTextStream::FixedNotation);

    m_isLogging = true;
    m_headerWritten = false;

    return true;
}

void Logger::stopLogging()
{
    if (m_isLogging) {
        m_logFile.close();
        m_isLogging = false;
    }
}

void Logger::writeHeader()
{
    // Simplified header with only time, x raw error, and y raw error
    m_textStream << "Timestamp,RawErrorX,RawErrorY" << Qt::endl;
    m_headerWritten = true;
}

void Logger::logData(const TrackData& data)
{
    if (!m_isLogging) {
        return;
    }

    if (!m_headerWritten) {
        writeHeader();
    }

    // Get current timestamp with millisecond precision
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");

    // Write timestamp and raw errors with 5 decimal places (full card precision)
    m_textStream << timestamp << ","
                 << QString::number(data.rawErrorX, 'f', 5) << ","
                 << QString::number(data.rawErrorY, 'f', 5) << Qt::endl;

    // Ensure data is written to disk immediately
    m_textStream.flush();
}