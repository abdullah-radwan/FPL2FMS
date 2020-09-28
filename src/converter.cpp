#include "converter.h"
#include "pugixml.hpp"

#include <QFile>
#include <QDate>
#include <QDebug>

void Converter::convert()
{
    emit convertingFile();

    waypoints.clear();

    qDebug() << "Reading" << data.inputPath;

    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(data.inputPath.toStdString().c_str());

    if (result.status != pugi::status_ok) {
        qDebug() << "Error:" << result.description();
        emit error(result.description());
        return;
    }

    for (const auto& fplWaypoint : doc.child("flight-plan").child("waypoint-table").children("waypoint")) {
        Waypoint waypoint;

        waypoint.type = convertType(fplWaypoint.child_value("type"));
        waypoint.ident = fplWaypoint.child_value("identifier");
        waypoint.lat = fplWaypoint.child_value("lat");
        waypoint.lon = fplWaypoint.child_value("lon");

        waypoints.append(waypoint);
    }

    setRoute();
}

QString Converter::convertType(QString type)
{
    if (type == "AIRPORT")
        return "1";

    if (type == "NDB")
        return "2";

    if (type == "VOR")
        return "3";

    if (type == "INT")
        return "11";

    if (type == "USER WAYPOINT")
        return "28";

    return "0";
}

void Converter::setRoute()
{
    QString departure = waypoints.first().ident;
    QString arrival = waypoints.last().ident;

    waypoints.first().airway = "ADEP";
    waypoints.last().airway = "ADES";

    QStringList route = data.route.split(" ");

    for (auto& waypoint : route) {
        if (waypoint.isEmpty())
            route.removeOne(waypoint);
    }

    qDebug() << "Route:" << route;

    for (auto& waypoint : waypoints) {
        if (waypoint.ident == departure || waypoint.ident == arrival)
            continue;

        if (route.empty()) {
            qDebug() << "Invalid route.";
            emit error("the flight route is invalid.");
            return;
        }

        if (waypoint.ident == route.first()) {
            waypoint.airway = "DRCT";

            route.removeFirst();

            continue;
        }

        if (route.size() < 2) {
            qDebug() << "Invalid route" << route;
            emit error("the flight route is invalid.");
            return;
        }

        if (waypoint.ident == route.at(1)) {
            waypoint.airway = route.first();

            route.removeFirst();
            route.removeFirst();

            continue;
        }

        waypoint.airway = route.first();
    }

    writeFMS();
}

void Converter::writeFMS()
{
    qDebug() << "Writing into" << data.outputPath;
    QFile file(data.outputPath);

    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Couldn't open the FMS file";
        emit error("couldn't create the FMS file.");
        return;
    }

    QTextStream out(&file);
    out << "I" << Qt::endl;
    out << "1100 Version" << Qt::endl;
    out << "CYCLE " << getCycle() << Qt::endl;
    out << "ADEP " << waypoints.first().ident << Qt::endl;
    out << "ADES " << waypoints.last().ident << Qt::endl;
    out << "NUMENR " << waypoints.size() << Qt::endl;

    for (auto& waypoint : waypoints)
        out << waypoint.type << " " << waypoint.ident << " " << waypoint.airway << " 0.000000 " << waypoint.lat << " " << waypoint.lon << Qt::endl;

    file.close();

    qDebug() << "File converted successfully";

    emit done();
}

QString Converter::getCycle()
{
    QDate currentDate = QDate::currentDate();
    QDate date = QDate::fromString("2020/09/10", "yyyy/MM/dd");

    int cycle = 10;
    int year = date.year();

    QMap<QDate, int> cycleMap;
    cycleMap.insert(date, cycle);

    while (true) {
        date = date.addDays(28);

        if (currentDate.daysTo(date) >= 0) {
            QString lastCycle;

            if (cycleMap.last() < 10)
                lastCycle = "0" + QString::number(cycleMap.last());
            else
                lastCycle = QString::number(cycleMap.last());

            QString sCycle = QString::number(cycleMap.lastKey().year()).remove(0,2) + lastCycle;
            qDebug() << "Cycle:" << sCycle;
            return sCycle;
        }

        if (date.year() > year) {
            cycleMap.clear();
            cycle = 0;
            year = date.year();
        }

        cycle++;
        cycleMap.insert(date, cycle);
    }
}
