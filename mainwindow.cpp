#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QString>
#include <QDir>
#include <QTextStream>
#include <iostream>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->selectPathBtn,SIGNAL(clicked()),this,SLOT(browseFolders()));
    connect(ui->closeAppBtn,SIGNAL(clicked()),this,SLOT(close()));

    readCsvFiles("selectPath");

}

void MainWindow::browseFolders(){

    QString selectedPath = QFileDialog::getExistingDirectory(nullptr, "Select Folder", QDir::homePath());

    ui->selectPathBtn->setText("Select Another Location");

    ui->pathLbl->setEnabled(true);
    ui->pathLbl->setText("Selected path:");

    ui->pathLine->setText("  " + selectedPath);

    ui->startPentalyserBtn->setEnabled(true);

    readCsvFiles(selectedPath);

}

void MainWindow::readCsvFiles(QString selectedPath){

    selectedPath = "/Users/emagabu/workspace/Pentacam-Code/Pentacam_Files";


    // List all CSV files in the directory
    QDir folder(selectedPath);
    QStringList csvFiles = folder.entryList(QStringList() << "*.csv", QDir::Files);

    int totalFiles = csvFiles.size();
    if (totalFiles == 0) {
        std::cout << "No CSV files found in the folder." << std::endl;
        return;
    }

    // Define the labels to search for
    QStringList labels = {
        "Cornea Front Rh",
        "Cornea Front Rv",
        "Pachy Min",
        "K Max (Front)"
    };

    // Process each file
    for (const QString& fileName : csvFiles) {
        QString filePath = folder.absoluteFilePath(fileName);
        QFile file(filePath);

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            std::cerr << "Failed to open file: " << filePath.toStdString() << std::endl;
            continue;
        }

        std::cout << "Processing file: " << filePath.toStdString() << std::endl;

        QTextStream in(&file);
        QMap<QString, QString> extractedValues;

        // Read file line by line
        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList parts = line.split(';');

            if (parts.size() >= 2) {
                QString label = parts[0].trimmed();
                QString value = parts[1].trimmed();

                // Check if the label matches any of the target labels
                if (labels.contains(label)) {
                    extractedValues[label] = value;
                }
            }
        }

        // Print extracted values for the current file
        for (const QString& label : labels) {
            if (extractedValues.contains(label)) {
                std::cout << label.toStdString() << ": " << extractedValues[label].toStdString() << std::endl;
            } else {
                std::cout << label.toStdString() << ": Not found" << std::endl;
            }
        }

        file.close();
        std::cout << "----------------------------------------" << std::endl;
    }

}


MainWindow::~MainWindow()
{
    delete ui;
}
