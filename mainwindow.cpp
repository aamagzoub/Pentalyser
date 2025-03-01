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
#include <QLabel>
#include <QProcess>

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
    connect(ui->tableWidget_2->selectionModel(), &QItemSelectionModel::selectionChanged,this, &MainWindow::onRowsSelectionChangedTable2);
    connect(ui->tableWidget_4->selectionModel(), &QItemSelectionModel::selectionChanged,this, &MainWindow::onRowsSelectionChangedTable4);
    connect(ui->calculateDiffBtn, &QPushButton::clicked, this, &MainWindow::calculateAveragesDiff);
    connect(ui->calculateDiffBtn_2, &QPushButton::clicked, this, &MainWindow::calculateAveragesDiff_2);
    connect(ui->clearTablesBtn, &QPushButton::clicked, this, &MainWindow::clearAllTables);
    connect(ui->clearSelectionsBtn, &QPushButton::clicked, this, &MainWindow::clearAllSelections);
    connect(ui->saveToCsvFileBtn, &QPushButton::clicked, this, &MainWindow::saveToCsvFile);
    connect(ui->showInFinderBtn, &QPushButton::clicked, this, &MainWindow::openFileLocationInFinder);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::browseFolders() {
    QString selectedPath = QFileDialog::getExistingDirectory(nullptr, "Select Folder", QDir::homePath());
    if (selectedPath.isEmpty()) {
        ui->statusTL_2->setText("WARNING: No path is selected! (default path will be used: " + getFilesPath() + ")");
        ui->statusTL_2->setStyleSheet("color: red;");
        return;
    }
    else {
        ui->statusTL_2->clear();
    }

    setFilesPath(selectedPath);
    ui->statusTL->setText("Files Path: " + getFilesPath());
}

void MainWindow::useDefaultPath() {
    if (getFilesPath() != defaultPath) {
        setFilesPath(defaultPath);
        ui->statusTL->setText("Files Path: " + getFilesPath());
        ui->statusTL_2->clear();
    }
}

QString MainWindow::getFilesPath() {
    return inputFilesPath.isEmpty() ? defaultPath : inputFilesPath;
}

QString MainWindow::getdefaultSavePath() {
    return defaultSavePath;
}

void MainWindow::setFilesPath(const QString &path) {
    inputFilesPath = path;
}

void MainWindow::startExtracting() {
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);
    ui->tableWidget_2->clearContents();
    ui->tableWidget_2->setRowCount(0);
    ui->tableWidget_4->clearContents();
    ui->tableWidget_4->setRowCount(0);

    ui->statusTL->setText("Files Path: " + getFilesPath());

    QStringList csvFiles = getCsvFiles(getFilesPath());
    if (csvFiles.isEmpty()) {
        ui->statusTL_2->setText("WARNING: No CSV files found to process.");
        ui->statusTL_2->setStyleSheet("color: red;");
        return;
    } else {
        ui->statusTL_2->clear();
    }

    QMap<QString, QMap<QString, QList<double>>> averagesData;
    QMap<QString, QStringList> nameData;
    QMap<QString, int> fileCounts;

    for (const QString &fileName : csvFiles) {
        QString firstname, surname, date, time, eye;
        QMap<QString, QString> extractedValues = parseCsvFile(getFilesPath() + "/" + fileName, firstname, surname, date, time, eye);

        int recentRow = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(recentRow);
        populateTableRow(recentRow, firstname, surname, date, time, eye, extractedValues);

        QString key = surname + ", " + firstname + " - " + date + " (" + eye + ")";
        nameData[key] = {surname, firstname, date, eye};

        fileCounts[key]++;

        for (const QString &metric : extractedValues.keys()) {
            averagesData[key][metric].append(extractedValues[metric].toDouble());
        }
    }

    populateSplitAveragesTables(averagesData, nameData, fileCounts);

    ui->tableWidget->resizeColumnsToContents();
    ui->tableWidget->resizeRowsToContents();
    ui->tableWidget_2->resizeColumnsToContents();
    ui->tableWidget_2->resizeRowsToContents();
    ui->tableWidget_4->resizeColumnsToContents();
    ui->tableWidget_4->resizeRowsToContents();
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
    QStringList labels = {"Cornea Front Rh", "Cornea Front Rv", "Pachy Min", "K Max (Front)", "QS Error"};

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

void MainWindow::populateTableRow(int row, const QString &firstname, const QString &surname,
                                  const QString &date, const QString &time, const QString &eye,
                                  const QMap<QString, QString> &extractedValues) {
    QStringList values = {surname, firstname, date, time, eye, extractedValues.value("QS Error", "N/A")};
    for (int i = 0; i < values.size(); i++) {
        QTableWidgetItem *item = new QTableWidgetItem(values[i]);
        if (i < 2) {
            item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        } else {
            item->setTextAlignment(Qt::AlignCenter);
        }
        ui->tableWidget->setItem(row, i, item);
    }

    QStringList labels = {"Cornea Front Rh", "Cornea Front Rv", "Pachy Min", "K Max (Front)"};
    int col = 6;
    for (const QString &label : labels) {
        QString value = extractedValues.value(label, "N/A");
        QTableWidgetItem *item = new QTableWidgetItem(value);
        item->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(row, col++, item);
    }
}

void MainWindow::populateSplitAveragesTables(const QMap<QString, QMap<QString, QList<double>>> &averagesData,
                                             const QMap<QString, QStringList> &nameData,
                                             const QMap<QString, int> &fileCounts) {
    int rowOD = 0, rowOS = 0;
    for (auto it = averagesData.begin(); it != averagesData.end(); ++it) {
        QStringList nameFields = nameData[it.key()];
        QString eye = nameFields[3];

        QTableWidget *targetTable = (eye == "OD") ? ui->tableWidget_2 : ui->tableWidget_4;
        int &recentRow = (eye == "OD") ? rowOD : rowOS;

        targetTable->insertRow(recentRow);

        for (int i = 0; i < nameFields.size(); i++) {
            QTableWidgetItem *item = new QTableWidgetItem(nameFields[i]);
            if (i < 2) {
                item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            } else {
                item->setTextAlignment(Qt::AlignCenter);
            }
            targetTable->setItem(recentRow, i, item);
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
            targetTable->setItem(recentRow, col++, item);
        }

        QTableWidgetItem *countItem = new QTableWidgetItem(QString::number(fileCounts[it.key()]));
        countItem->setTextAlignment(Qt::AlignCenter);
        targetTable->setItem(recentRow, col, countItem);

        recentRow++;
    }
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
            if (i < 2) {
                item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter); // Align left for first two columns
            } else {
                item->setTextAlignment(Qt::AlignCenter);
            }
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

void MainWindow::applyTableFormatting(QTableWidget *table) {
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    for (int row = 0; row < table->rowCount(); ++row) {
        for (int col = 0; col < table->columnCount(); ++col) {
            QTableWidgetItem *item = table->item(row, col);
            if (!item) {
                item = new QTableWidgetItem();
                table->setItem(row, col, item);
            }

            if (col < 2) {
                item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            } else {
                item->setTextAlignment(Qt::AlignCenter);
            }
        }
    }
}

void MainWindow::clearAllTables() {

    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);

    ui->tableWidget_2->clearContents();
    ui->tableWidget_2->setRowCount(0);

    ui->tableWidget_4->clearContents();
    ui->tableWidget_4->setRowCount(0);

    ui->tableWidget_5->clearContents();
    ui->tableWidget_5->setRowCount(0);

    ui->rowsSelectionWarningL->clear();
    ui->rowsSelectionWarningL_2->clear();

    ui->saveToCsvFileBtn->setEnabled(false);
}

void MainWindow::clearAllSelections() {
    ui->tableWidget->clearSelection();
    ui->tableWidget_2->clearSelection();
    ui->tableWidget_4->clearSelection();
    ui->tableWidget_5->clearSelection();
    ui->rowsSelectionWarningL->clear();
    ui->rowsSelectionWarningL_2->clear();
}


#include <QDir> // Ensure this is included for directory handling

void MainWindow::saveToCsvFile() {
    // Check if tableWidget_5 is empty
    if (ui->tableWidget_5->rowCount() == 0) {
        ui->savingWarningL->setText("Warning: The table is empty. No data to save.");
        ui->savingWarningL->setStyleSheet("color: red;");
        return;
    }

    // Get current date and time for filename
    QString currentDateTime = QDateTime::currentDateTime().toString("ddMMyyyy_HHmmss");
    QString saveDir = getdefaultSavePath();
    QString savePath = saveDir + "/output_data_" + currentDateTime + ".csv";

    // Ensure the save directory exists, create if necessary
    QDir dir(saveDir);
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {  // Create the directory and check for failure
            ui->savingWarningL->setText("Error: Could not create save directory.");
            ui->savingWarningL->setStyleSheet("color: red;");
            return;
        }
    }

    QFile file(savePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        ui->savingWarningL->setText("Error: Could not save file.");
        ui->savingWarningL->setStyleSheet("color: red;");
        return;
    }

    QTextStream out(&file);

    // Write headers safely
    QStringList headers;
    for (int col = 0; col < ui->tableWidget_5->columnCount(); ++col) {
        QTableWidgetItem *headerItem = ui->tableWidget_5->horizontalHeaderItem(col);
        headers << (headerItem ? headerItem->text() : "");  // Handle possible nullptr
    }
    out << headers.join(",") << "\n";

    // Write table data
    for (int row = 0; row < ui->tableWidget_5->rowCount(); ++row) {
        QStringList rowValues;
        for (int col = 0; col < ui->tableWidget_5->columnCount(); ++col) {
            QTableWidgetItem *item = ui->tableWidget_5->item(row, col);
            rowValues << (item ? item->text() : "");  // Handle empty cells
        }
        out << rowValues.join(",") << "\n";
    }

    // Ensure data is properly flushed to the file
    if (out.status() != QTextStream::Ok) {
        ui->savingWarningL->setText("Error: Failed to write data to file.");
        ui->savingWarningL->setStyleSheet("color: red;");
        file.close();
        return;
    }

    file.close();
    ui->savingWarningL->setText("Table saved to: " + savePath);
    ui->savingWarningL->setStyleSheet("color: green;");

    // Enable the show-in-finder button after a successful save
    ui->showInFinderBtn->setEnabled(true);
}


void MainWindow::openFileLocationInFinder() {
    QString folderPath = getdefaultSavePath(); // Ensure this returns the directory where files are saved

    // Open Finder at the specified folder
    QProcess::startDetached("open", QStringList() << folderPath);
}

void MainWindow::calculateAveragesDiff() {
    calculateAveragesDiffTable(ui->tableWidget_2, ui->rowsSelectionWarningL);
    applyTableFormatting(ui->tableWidget_2);
}

void MainWindow::calculateAveragesDiff_2() {
    calculateAveragesDiffTable(ui->tableWidget_4, ui->rowsSelectionWarningL_2);
    applyTableFormatting(ui->tableWidget_4);
}

void MainWindow::calculateAveragesDiffTable(QTableWidget *sourceTable, QLabel *warningLabel) {
    warningLabel->clear();

    QModelIndexList selectedRows = sourceTable->selectionModel()->selectedRows();
    if (selectedRows.count() != 2) return;

    int row1 = selectedRows.at(0).row();
    int row2 = selectedRows.at(1).row();

    QString surname1 = sourceTable->item(row1, 0)->text();
    QString firstname1 = sourceTable->item(row1, 1)->text();
    QString eye1 = sourceTable->item(row1, 3)->text();
    QString date1 = sourceTable->item(row1, 2)->text();

    QString surname2 = sourceTable->item(row2, 0)->text();
    QString firstname2 = sourceTable->item(row2, 1)->text();
    QString eye2 = sourceTable->item(row2, 3)->text();
    QString date2 = sourceTable->item(row2, 2)->text();

    if (surname1 != surname2 || firstname1 != firstname2 || eye1 != eye2) {
        warningLabel->setText("Error: Selected rows must be for the same patient and eye.");
        warningLabel->setStyleSheet("color: red;");
        return;
    }

    QDate dateObj1 = QDate::fromString(date1, "dd-MM-yyyy");
    QDate dateObj2 = QDate::fromString(date2, "dd-MM-yyyy");

    QString baselineDate = (dateObj1 <= dateObj2) ? date1 : date2;
    QString recentDate = (dateObj1 > dateObj2) ? date1 : date2;

    // Clear previous selections before checking for duplicates
    ui->tableWidget_5->clearSelection();

    // Check for duplicates and highlight the existing row if found
    for (int row = 0; row < ui->tableWidget_5->rowCount(); ++row) {
        if (ui->tableWidget_5->item(row, 0)->text() == surname1 &&
            ui->tableWidget_5->item(row, 1)->text() == firstname1 &&
            ui->tableWidget_5->item(row, 2)->text() == eye1 &&
            ui->tableWidget_5->item(row, 3)->text() == baselineDate &&
            ui->tableWidget_5->item(row, 4)->text() == recentDate) {

            // Select and highlight the existing row
            ui->tableWidget_5->selectRow(row);
            ui->tableWidget_5->scrollToItem(ui->tableWidget_5->item(row, 0));

            return; // Stop here to prevent adding a duplicate
        }
    }

    double k1Recent = sourceTable->item(row1, 4)->text().toDouble();
    double k2Recent = sourceTable->item(row1, 5)->text().toDouble();
    double pachyMinRecent = sourceTable->item(row1, 6)->text().toDouble();
    double kMaxRecent = sourceTable->item(row1, 7)->text().toDouble();

    double k1Baseline = sourceTable->item(row2, 4)->text().toDouble();
    double k2Baseline = sourceTable->item(row2, 5)->text().toDouble();
    double pachyMinBaseline = sourceTable->item(row2, 6)->text().toDouble();
    double kMaxBaseline = sourceTable->item(row2, 7)->text().toDouble();

    double diffK1 = k1Recent - k1Baseline;
    double diffK2 = k2Recent - k2Baseline;
    double diffPachyMin = pachyMinRecent - pachyMinBaseline;
    double diffKMax = kMaxRecent - kMaxBaseline;

    int newRow = ui->tableWidget_5->rowCount();
    ui->tableWidget_5->insertRow(newRow);

    ui->tableWidget_5->setItem(newRow, 0, new QTableWidgetItem(surname1));
    ui->tableWidget_5->setItem(newRow, 1, new QTableWidgetItem(firstname1));
    ui->tableWidget_5->setItem(newRow, 2, new QTableWidgetItem(eye1));
    ui->tableWidget_5->setItem(newRow, 3, new QTableWidgetItem(baselineDate));
    ui->tableWidget_5->setItem(newRow, 4, new QTableWidgetItem(recentDate));
    ui->tableWidget_5->setItem(newRow, 5, new QTableWidgetItem(QString::number(diffK1, 'f', 2)));
    ui->tableWidget_5->setItem(newRow, 6, new QTableWidgetItem(QString::number(diffK2, 'f', 2)));
    ui->tableWidget_5->setItem(newRow, 7, new QTableWidgetItem(QString::number(diffPachyMin, 'f', 2)));
    ui->tableWidget_5->setItem(newRow, 8, new QTableWidgetItem(QString::number(diffKMax, 'f', 2)));

    sourceTable->clearSelection();
    ui->tableWidget_5->scrollToBottom();
    applyTableFormatting(ui->tableWidget_5);
    ui->saveToCsvFileBtn->setEnabled(true);
}

void MainWindow::onRowsSelectionChangedTable2() {
    QModelIndexList selectedRows = ui->tableWidget_2->selectionModel()->selectedRows();
    int selectedCount = selectedRows.count();

    if (selectedCount == 2) {
        int row1 = selectedRows.at(0).row();
        int row2 = selectedRows.at(1).row();

        QString surname1 = ui->tableWidget_2->item(row1, 0)->text();
        QString firstname1 = ui->tableWidget_2->item(row1, 1)->text();

        QString surname2 = ui->tableWidget_2->item(row2, 0)->text();
        QString firstname2 = ui->tableWidget_2->item(row2, 1)->text();

        bool nameMismatch = (surname1 != surname2 || firstname1 != firstname2);

        if (nameMismatch) {
            ui->rowsSelectionWarningL->setText("WARNING: You have selected two different patients");
            ui->rowsSelectionWarningL->setStyleSheet("color: red;");
            ui->calculateDiffBtn->setEnabled(false);
        } else {
            ui->rowsSelectionWarningL->clear();
            ui->rowsSelectionWarningL->setStyleSheet("");
            ui->calculateDiffBtn->setEnabled(true);
        }
    } else {
        ui->rowsSelectionWarningL->clear();
        ui->rowsSelectionWarningL->setStyleSheet("");
        ui->calculateDiffBtn->setEnabled(false);
    }
}

void MainWindow::onRowsSelectionChangedTable4() {
    QModelIndexList selectedRows = ui->tableWidget_4->selectionModel()->selectedRows();
    int selectedCount = selectedRows.count();

    if (selectedCount == 2) {
        int row1 = selectedRows.at(0).row();
        int row2 = selectedRows.at(1).row();

        QString surname1 = ui->tableWidget_4->item(row1, 0)->text();
        QString firstname1 = ui->tableWidget_4->item(row1, 1)->text();

        QString surname2 = ui->tableWidget_4->item(row2, 0)->text();
        QString firstname2 = ui->tableWidget_4->item(row2, 1)->text();

        bool nameMismatch = (surname1 != surname2 || firstname1 != firstname2);

        if (nameMismatch) {
            ui->rowsSelectionWarningL_2->setText("WARNING: You have selected two different patients");
            ui->rowsSelectionWarningL_2->setStyleSheet("color: red;");
            ui->calculateDiffBtn_2->setEnabled(false);
        } else {
            ui->rowsSelectionWarningL_2->clear();
            ui->rowsSelectionWarningL_2->setStyleSheet("");
            ui->calculateDiffBtn_2->setEnabled(true);
        }
    } else {
        ui->rowsSelectionWarningL_2->clear();
        ui->rowsSelectionWarningL_2->setStyleSheet("");
        ui->calculateDiffBtn_2->setEnabled(false);
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
