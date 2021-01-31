#include "mainwindow.h"

#include <QApplication>
#include <QDateTime>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

static QFile* logFile;

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context) Q_UNUSED(type)

    QTextStream out(logFile);

    out << QDateTime::currentDateTime().toString("hh:mm:ss.zzz ");

    out << msg << Qt::endl;

    out.flush();
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("FPL2FMS");
    app.setApplicationDisplayName("FPL2FMS");
    app.setApplicationVersion("1.1");

    QString dirPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir(dirPath).mkpath(".");

    logFile = new QFile(dirPath + "/log.txt");
    logFile->open(QFile::WriteOnly | QFile::Text);

    qInstallMessageHandler(messageHandler);

    qDebug() << "FPL2FMS 1.1, 31 January 2021";
    qDebug() << "Current date:" << QDateTime::currentDateTime().toString("yyyy-MM-dd");

    MainWindow mainWindow(dirPath);

    int result = app.exec();

    logFile->close();
    delete logFile;

    return result;
}
