#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "converter.h"

#include <QMainWindow>
#include <QSettings>
#include <QThread>
#include <QtAutoUpdaterWidgets/UpdateController>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QString dirPath);
    ~MainWindow();

signals:
    void convert();

public slots:
    void onConvertingFile();
    void onDone();
    void onError(QString error);

private slots:
    void on_fplButton_clicked();
    void on_convertButton_clicked();
    void on_aboutAction_triggered();

    void on_checkAction_triggered();

private:
    Ui::MainWindow *ui;
    QSettings settings;
    QtAutoUpdater::Updater *updater;

    QString inputFolder;
    QString outputFolder;
    bool checkUpdates;

    Converter *converter;
    QThread converterThread;

    void readConfig();
    void writeConfig();

    void setConverter();
    void setUpdater();
};
#endif // MAINWINDOW_H
