#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include "trackdata.h"

class Logger : public QObject
{
    Q_OBJECT
public:
    explicit Logger(QObject *parent = nullptr);
    ~Logger();

    bool startLogging(const QString& filename);
    void stopLogging();
    bool isLogging() const { return m_isLogging; }
    void logData(const TrackData& data);

signals:
    void errorOccurred(const QString& errorMsg);

private:
    QFile m_logFile;
    QTextStream m_textStream;
    bool m_isLogging;
    bool m_headerWritten;

    void writeHeader();
};

#endif // LOGGER_H