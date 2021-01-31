#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>

MainWindow::MainWindow(QString dirPath) : ui(new Ui::MainWindow), settings(dirPath + "/config.cfg", QSettings::IniFormat), converterThread(this)
{
    ui->setupUi(this);

    settings.setParent(this);
    readConfig();

    setConverter();
    setUpdater();

    show();

    if (checkUpdates) on_checkAction_triggered();
}

void MainWindow::readConfig()
{
    settings.beginGroup("Settings");

    inputFolder = settings.value("InputFolder").toString();
    outputFolder = settings.value("OutputFolder").toString();
    ui->deleteCheck->setChecked(settings.value("DeleteFPL", true).toBool());
    checkUpdates = settings.value("CheckDate", 0).toDate().addDays(14) <= QDate::currentDate();

    settings.endGroup();

    settings.beginGroup("MainWindow");

    QSize size = settings.value("Size", QSize(-1, -1)).toSize();
    if (size != QSize(-1, -1)) resize(size);

    QPoint pos = settings.value("Pos", QPoint(-1, -1)).toPoint();
    if (pos != QPoint(-1, -1)) move(pos);

    settings.endGroup();
}

void MainWindow::setConverter()
{
    converter = new Converter;

    converter->moveToThread(&converterThread);

    connect(this, &MainWindow::convert, converter, &Converter::convert);

    connect(converter, &Converter::convertingFile, this, &MainWindow::onConvertingFile);
    connect(converter, &Converter::done, this, &MainWindow::onDone);
    connect(converter, &Converter::error, this, &MainWindow::onError);

    connect(&converterThread, &QThread::finished, converter, &QObject::deleteLater);

    converterThread.start();
}

void MainWindow::setUpdater()
{
    updater = QtAutoUpdater::Updater::create("qtifw", {{"path", qApp->applicationDirPath() + "/maintenancetool"}}, qApp);

    if (updater)
    {
        connect(updater, &QtAutoUpdater::Updater::checkUpdatesDone, [&](QtAutoUpdater::Updater::State state)
        {
            switch (state)
            {
            case QtAutoUpdater::Updater::State::Error:
                qDebug() << "An error occured";
                checkUpdates = false;
                break;

            case QtAutoUpdater::Updater::State::NoUpdates:
                qDebug() << "No updates were found";
                break;

            case QtAutoUpdater::Updater::State::NewUpdates:
                qDebug() << "An update was found" << updater->updateInfo();
                break;

            default:
                break;
            }
        });
    }
    else qDebug() << "Couldn't create the updater";
}

void MainWindow::on_fplButton_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Select FPL File", inputFolder, "FPL Files (*.fpl)");

    if (path.isEmpty()) return;

    inputFolder = QFileInfo(path).absolutePath();

    converter->data.inputPath = path;

    ui->fplLabel->setText("Selected file: " + path);
}

void MainWindow::on_convertButton_clicked()
{
    if (converter->data.inputPath.isEmpty())
    {
        statusBar()->showMessage("You must choose an FPL file.");
        return;
    }

    converter->data.route = ui->routeEdit->text();

    if (converter->data.route.isEmpty())
    {
        statusBar()->showMessage("The flight route can't be empty.");
        return;
    }

    QString savePath;

    if (outputFolder.isEmpty())
    {
        savePath = converter->data.inputPath;
        savePath.remove(".fpl");
        savePath.append(".fms");
    }
    else savePath = outputFolder + "/" + QFileInfo(converter->data.inputPath).baseName().append(".fms");

    QString path = QFileDialog::getSaveFileName(this, "Save FMS File", savePath, "FMS Files (*.fms)");

    if (path.isEmpty()) return;

    if (!path.endsWith(".fms", Qt::CaseInsensitive)) path.append(".fms");

    ui->convertButton->setEnabled(false);

    outputFolder = QFileInfo(path).absolutePath();

    converter->data.outputPath = path;

    converter->data.deleteFPL = ui->deleteCheck->isChecked();

    emit convert();
}

void MainWindow::onConvertingFile()
{
    statusBar()->showMessage("Converting the FPL file...");
}

void MainWindow::onDone()
{
    statusBar()->showMessage("The FPL file was converted successfully.");
    ui->convertButton->setEnabled(true);
}

void MainWindow::onError(QString error)
{
    statusBar()->showMessage("Error: " + error);
    ui->convertButton->setEnabled(true);
}

void MainWindow::on_checkAction_triggered()
{
    if (!updater)
    {
        QMessageBox::warning(this, "Updater initialization", "Couldn't initialize the updater.");
        return;
    }

    qDebug() << "Starting updater";

    (new QtAutoUpdater::UpdateController(updater, this))->start(QtAutoUpdater::UpdateController::DisplayLevel::ExtendedInfo);
}


void MainWindow::on_aboutAction_triggered()
{
    QMessageBox::about(this, "FPL2FMS", "FPL2FMS\n"
                                        "An app to convert Garmin FPL files to X-Plane 11 FMS files.\n\n"

                                        "Version: 1.1\n"
                                        "Release date: 31 January 2021\n\n"

                                        "Copyright © Abdullah Radwan");
}

void MainWindow::writeConfig()
{
    settings.beginGroup("Settings");

    settings.setValue("InputFolder", inputFolder);
    settings.setValue("OutputFolder", outputFolder);
    settings.setValue("DeleteFPL", ui->deleteCheck->isChecked());

    if (checkUpdates || !settings.value("CheckDate", 0).toDate().isValid()) settings.setValue("CheckDate", QDate::currentDate());

    settings.endGroup();

    settings.beginGroup("MainWindow");

    settings.setValue("Size", size());
    settings.setValue("Pos", pos());

    settings.endGroup();
}

MainWindow::~MainWindow()
{
    converterThread.quit();
    converterThread.wait();

    writeConfig();

    delete ui;
}
