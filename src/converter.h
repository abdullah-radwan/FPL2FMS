#ifndef FPLCONVERTER_H
#define FPLCONVERTER_H

#include <QObject>

class Converter : public QObject
{
    Q_OBJECT
public:
    struct data {
        QString inputPath;
        QString route;
        QString outputPath;
    } data;

signals:
    void convertingFile();
    void done();
    void error(QString error);

public slots:
    void convert();

private:
    struct Waypoint {
        QString type;
        QString ident;
        QString airway;
        QString lat;
        QString lon;
    };

    QList<Waypoint> waypoints;

    QString convertType(QString type);
    void setRoute();
    void writeFMS();
    QString getCycle();
};

#endif // FPLCONVERTER_H
