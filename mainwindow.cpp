#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QString>
#include <QDir>
#include <QTextStream>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QMap>
#include <QTableWidgetItem>
#include <QDate>
#include <QTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Pentalyser");

    connect(ui->selectPathBtn, &QPushButton::clicked, this, &MainWindow::browseFolders);
    connect(ui->startExctractingBtn, &QPushButton::clicked, this, &MainWindow::startExtracting);
    connect(ui->useDefaultPathBtn, &QPushButton::clicked, this, &MainWindow::useDefaultPath);
    connect(ui->closeAppBtn, &QPushButton::clicked, this, &MainWindow::close);
}

void MainWindow::browseFolders() {
    QString selectedPath = QFileDialog::getExistingDirectory(nullptr, "Select Folder", QDir::homePath());
    if (selectedPath.isEmpty()) {
        ui->statusTL->setText("WARNING: No path is selected! (default path will be used: " + getFilesPath() + ")");
        return;
    }
    setFilesPath(selectedPath);
    ui->statusTL->setText("Files Path: " + getFilesPath());
}

void MainWindow::useDefaultPath() {
    if (getFilesPath() != defaultPath) {
        setFilesPath(defaultPath);
        ui->statusTL->setText("Files Path: " + getFilesPath());
    }
}

void MainWindow::startExtracting() {
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);
    ui->tableWidget_2->clearContents();
    ui->tableWidget_2->setRowCount(0);

    ui->statusTL->setText("Files Path: " + getFilesPath());

    QStringList csvFiles = getCsvFiles(getFilesPath());
    if (csvFiles.isEmpty()) {
        ui->statusTL->setText("WARNING: No CSV files found to process.");
        return;
    }

    QMap<QString, QMap<QString, QList<double>>> averagesData;
    QMap<QString, QStringList> nameData;
    QMap<QString, int> fileCounts;

    for (const QString &fileName : csvFiles) {
        QString firstname, surname, date, time, eye;
        QMap<QString, QString> extractedValues = parseCsvFile(getFilesPath() + "/" + fileName, firstname, surname, date, time, eye);

        int currentRow = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(currentRow);
        populateTableRow(currentRow, firstname, surname, date, time, eye, extractedValues);

        QString key = surname + ", " + firstname + " - " + date + " (" + eye + ")";
        nameData[key] = {surname, firstname, date, eye};

        fileCounts[key]++;

        for (const QString &metric : extractedValues.keys()) {
            averagesData[key][metric].append(extractedValues[metric].toDouble());
        }
    }

    populateAveragesTable(averagesData, nameData, fileCounts);

    ui->tableWidget->resizeColumnsToContents();
    ui->tableWidget->resizeRowsToContents();
    ui->tableWidget_2->resizeColumnsToContents();
    ui->tableWidget_2->resizeRowsToContents();
}

QStringList MainWindow::getCsvFiles(const QString &path) {
    QDir folder(path);
    return folder.entryList(QStringList() << "*.csv", QDir::Files);
}

QMap<QString, QString> MainWindow::parseCsvFile(const QString &filePath, QString &firstname, QString &surname, QString &date, QString &time, QString &eye) {
    QFile file(filePath);
    QMap<QString, QString> extractedValues;

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return extractedValues;
    }

    QStringList fileParts = QFileInfo(filePath).fileName().split('_');
    if (fileParts.size() >= 5) {
        surname = fileParts[0];
        firstname = fileParts[1];
        eye = fileParts[2];
        date = formatDate(fileParts[3]);
        time = formatTime(fileParts[4].split('.').first());
    }

    QTextStream in(&file);
    QStringList labels = {"Cornea Front Rh", "Cornea Front Rv", "Pachy Min", "K Max (Front)"};
    while (!in.atEnd()) {
        QStringList parts = in.readLine().split(';');
        if (parts.size() >= 2) {
            QString label = parts[0].trimmed();
            QString value = parts[1].trimmed();
            if (labels.contains(label)) {
                extractedValues[label] = value;
            }
        }
    }
    file.close();
    return extractedValues;
}



void MainWindow::populateAveragesTable(const QMap<QString, QMap<QString, QList<double>>> &averagesData,
                                       const QMap<QString, QStringList> &nameData,
                                       const QMap<QString, int> &fileCounts) {
    int row = 0;
    for (auto it = averagesData.begin(); it != averagesData.end(); ++it) {
        ui->tableWidget_2->insertRow(row);

        QStringList nameFields = nameData[it.key()];
        for (int i = 0; i < nameFields.size(); i++) {
            QTableWidgetItem *item = new QTableWidgetItem(nameFields[i]);
            item->setTextAlignment(Qt::AlignCenter);
            ui->tableWidget_2->setItem(row, i, item);
        }

        int col = 4;
        for (const QString &metric : {"Cornea Front Rh", "Cornea Front Rv", "Pachy Min", "K Max (Front)"}) {
            double sum = 0;
            for (double value : it.value()[metric]) {
                sum += value;
            }
            double average = it.value()[metric].isEmpty() ? 0.0 : sum / it.value()[metric].size();

            QString displayValue;
            if (metric == "Cornea Front Rh" || metric == "Cornea Front Rv") {
                displayValue = QString::number(337.5 / average, 'f', 1);
            } else if (metric == "Pachy Min") {
                displayValue = QString::number(qRound(average));
            } else {
                displayValue = QString::number(average, 'f', 2);
            }

            QTableWidgetItem *item = new QTableWidgetItem(displayValue);
            item->setTextAlignment(Qt::AlignCenter);
            ui->tableWidget_2->setItem(row, col++, item);
        }

        QTableWidgetItem *countItem = new QTableWidgetItem(QString::number(fileCounts[it.key()]));
        countItem->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget_2->setItem(row, col, countItem);

        row++;
    }
}

void MainWindow::populateTableRow(int row, const QString &firstname, const QString &surname,
                                  const QString &date, const QString &time, const QString &eye,
                                  const QMap<QString, QString> &extractedValues) {
    QStringList values = {surname, firstname, date, time, eye};
    for (int i = 0; i < values.size(); i++) {
        QTableWidgetItem *item = new QTableWidgetItem(values[i]);
        item->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(row, i, item);
    }

    QStringList labels = {"Cornea Front Rh", "Cornea Front Rv", "Pachy Min", "K Max (Front)"};
    int col = 5;
    for (const QString &label : labels) {
        QString value = extractedValues.value(label, "N/A");
        QTableWidgetItem *item = new QTableWidgetItem(value);
        item->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(row, col++, item);
    }
}

QString MainWindow::formatDate(const QString &rawDate) {
    QDate parsedDate = QDate::fromString(rawDate, "ddMMyyyy");
    return parsedDate.isValid() ? parsedDate.toString("dd-MM-yyyy") : "Invalid Date";
}

QString MainWindow::formatTime(const QString &rawTime) {
    QTime parsedTime = QTime::fromString(rawTime, "hhmmss");
    return parsedTime.isValid() ? parsedTime.toString("hh:mm:ss") : "Invalid Time";
}

QString MainWindow::getFilesPath() {
    return inputFilesPath.isEmpty() ? defaultPath : inputFilesPath;
}

void MainWindow::setFilesPath(const QString &path) {
    inputFilesPath = path;
}

MainWindow::~MainWindow() {
    delete ui;
}
