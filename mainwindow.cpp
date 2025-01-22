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
    connect(ui->useDefulatPathBtn,SIGNAL(clicked()),this,SLOT(useDefaultPath()));
    connect(ui->closeAppBtn,SIGNAL(clicked()),this,SLOT(close()));

    //connect(ui->setPathAsDefaultBtn,SIGNAL(clicked()),this,SLOT(setPathAsDefault()));

    connect(ui->startPentalyserBtn,SIGNAL(clicked()),this,SLOT(startAnalysis()));

}

void MainWindow::startAnalysis(){

    readCsvFiles(defaultPath);

}

void MainWindow::useDefaultPath(){

    setFilesPath(defaultPath);

}

void MainWindow::browseFolders(){

    ui->defaultPathLine->clear();

    QString selectedPath = QFileDialog::getExistingDirectory(nullptr, "Select Folder", QDir::homePath());

    ui->selectedPathLine->setText(" "+ selectedPath);

    setFilesPath(selectedPath);

}


void MainWindow::setFilesPath(QString path){

    filesPath = path;
    ui->startPentalyserBtn->setEnabled(true);

}



QString MainWindow::getFilesPath(){
    return filesPath;
}

void MainWindow::setPathAsDefault(){

    filesPath = getFilesPath();

}


/*
void MainWindow::readCsvFiles(QString selectedPath) {

    // Ensure a QTextBrowser widget exists in your GUI and is accessible as `ui->textBrowser`

    // Clear the QTextBrowser before starting
    ui->textBrowser->clear();

    // List all CSV files in the directory
    QDir folder(selectedPath);
    QStringList csvFiles = folder.entryList(QStringList() << "*.csv", QDir::Files);

    int totalFiles = csvFiles.size();
    if (totalFiles == 0) {
        ui->textBrowser->append("No CSV files found in the folder.");
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
            ui->textBrowser->append("Failed to open file: " + filePath);
            continue;
        }

        ui->textBrowser->append("Processing file: " + filePath);

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
                ui->textBrowser->append(label + ": " + extractedValues[label]);
            } else {
                ui->textBrowser->append(label + ": Not found");
            }
        }

        file.close();
        ui->textBrowser->append("----------------------------------------");
    }
}
*/

void MainWindow::readCsvFiles(QString selectedPath) {
    // Clear the tableWidget before populating new data
    ui->tableWidget->setRowCount(0);

    // List all CSV files in the directory
    QDir folder(selectedPath);
    QStringList csvFiles = folder.entryList(QStringList() << "*.csv", QDir::Files);

    int totalFiles = csvFiles.size();
    if (totalFiles == 0) {
        // If no CSV files found, add a row to indicate this
        int currentRow = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(currentRow);
        ui->tableWidget->setItem(currentRow, 0, new QTableWidgetItem("No CSV files found"));
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
        QFile file(folder.absoluteFilePath(fileName));

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            // Skip files that can't be opened
            continue;
        }

        // Extract information from the file name
        QStringList fileParts = fileName.split('_');
        QString firstname, surname, date, time, eye;

        if (fileParts.size() >= 5) {
            surname = fileParts[0];
            firstname = fileParts[1];

            // Map eye information to S (Right Eye) or D (Left Eye)
            eye = (fileParts[2].toLower() == "od") ? "D" :
                      (fileParts[2].toLower() == "os") ? "S" :
                      "Unknown";

            // Format the date
            QDate parsedDate = QDate::fromString(fileParts[3], "ddMMyyyy");
            date = parsedDate.isValid() ? parsedDate.toString("dd-MM-yyyy") : "Invalid Date";

            // Format the time
            QTime parsedTime = QTime::fromString(fileParts[4].split('.').first(), "hhmmss");
            time = parsedTime.isValid() ? parsedTime.toString("hh:mm:ss") : "Invalid Time";
        } else {
            firstname = "Unknown";
            surname = "Unknown";
            date = "Unknown";
            time = "Unknown";
            eye = "Unknown";
        }

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

        // Add a new row to the table
        int currentRow = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(currentRow);

        // Fill the table with extracted data
        ui->tableWidget->setItem(currentRow, 0, new QTableWidgetItem(firstname));
        ui->tableWidget->setItem(currentRow, 1, new QTableWidgetItem(surname));
        ui->tableWidget->setItem(currentRow, 2, new QTableWidgetItem(date));
        ui->tableWidget->setItem(currentRow, 3, new QTableWidgetItem(time));
        ui->tableWidget->setItem(currentRow, 4, new QTableWidgetItem(eye));
        ui->tableWidget->setItem(currentRow, 5, new QTableWidgetItem(extractedValues.value("Cornea Front Rh", "Not found")));
        ui->tableWidget->setItem(currentRow, 6, new QTableWidgetItem(extractedValues.value("Cornea Front Rv", "Not found")));
        ui->tableWidget->setItem(currentRow, 7, new QTableWidgetItem(extractedValues.value("Pachy Min", "Not found")));
        ui->tableWidget->setItem(currentRow, 8, new QTableWidgetItem(extractedValues.value("K Max (Front)", "Not found")));

        file.close();
    }
}



MainWindow::~MainWindow()
{
    delete ui;
}
