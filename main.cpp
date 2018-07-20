/*
 ISC License

 Copyright (c) 2016-2017, Autonomous Vehicle Systems Lab, University of Colorado at Boulder

 Permission to use, copy, modify, and/or distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.

 THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

 */
#include <stdio.h>
#include <iostream>
#include <QApplication>
#include <QCommandLineParser>
#include <QDesktopWidget>
#include <QSurfaceFormat>
#include <QLibraryInfo>
#include "mainwindow.h"
#include "simdatamanager.h"

int main(int argc, char *argv[])
{
    
    // To redirect stdout to a log file uncomment the line below
    //freopen("visualization.log", "w", stdout);
    std::cout << QLibraryInfo::location(QLibraryInfo::PluginsPath).toStdString() <<std::endl;
    std::cout << QLibraryInfo::location(QLibraryInfo::LibrariesPath).toStdString() <<std::endl;
    std::cout << QLibraryInfo::location(QLibraryInfo::BinariesPath).toStdString() <<std::endl;
    QApplication::setAttribute(Qt::AA_DontUseNativeMenuBar);
    QApplication::setAttribute(Qt::AA_MacPluginApplication);
    QApplication app(argc, argv); // object needed for QtWidget
    /*QApplication provides command-line arguments*/
        
    QCommandLineParser parser;
    /*Define set of options, parse command-line arg and store opt values and which opt have been used*/
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("dataPath", "spice data path"); //
    QCommandLineOption dataDirectoryOption("d", "data path to spice library", "dataPath", ".data/");
    parser.addOption(dataDirectoryOption);
    parser.process(app); // process the actual command-line arg given by user
    QString dataPath = parser.value(dataDirectoryOption);
    
    QSurfaceFormat format;
    format.setDepthBufferSize(24); // set the minimum depth buffer size to size
    QSurfaceFormat::setDefaultFormat(format); // default surface format nxPix*nyPix(*bitsInPix)
    
    MainWindow mainWindow; // window for building the application's user interface
    mainWindow.setDataPath(dataPath);
    if (!mainWindow.loadSpiceFiles()) {
        std::cout << "Successfully loaded spice libraries at path: " << dataPath.toStdString() << std::endl;
    } else {
        std::cout << "Failed to load spice libraries at path: " << dataPath.toStdString() << std::endl;
        return EXIT_FAILURE;
    }
//    if (!mainWindow.loadStarCatalog()) {
//        std::cout << "Successfully loaded star catalog at path: " << dataPath.toStdString() << std::endl;
//    } else {
//        std::cout << "Failed to load star catalog at path: " << dataPath.toStdString() << std::endl;
//        return EXIT_FAILURE;
//    }

    mainWindow.setWindowIcon(QIcon(":/resources/images/AVSlog5-Large.png"));
    mainWindow.setWindowTitle("AVSLab Visualization");
    int desktopArea = QApplication::desktop()->width() *
                      QApplication::desktop()->height();
    int widgetArea = mainWindow.width() * mainWindow.height();
    if(((float)widgetArea / (float)desktopArea) < 0.75f) {
        mainWindow.show();
    } else {
        mainWindow.showMaximized();
    }
    return app.exec();
}
