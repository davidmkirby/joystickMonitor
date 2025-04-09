#ifndef LOGRECORD_H
#define LOGRECORD_H

struct LogRecord {
    qint64 elapsedTime;  // Time in nanoseconds since start
    double frequency;
    double amplitude;
    double xCommand;
    double yCommand;
    double xFeedback;
    double yFeedback;
};

#endif // LOGRECORD_H