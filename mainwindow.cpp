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
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Pentalyser");

    // General button connections
    connect(ui->selectPathBtn, &QPushButton::clicked, this, &MainWindow::browseFolders);
    connect(ui->startExctractingBtn, &QPushButton::clicked, this, &MainWindow::startExtracting);
    connect(ui->useDefaultPathBtn, &QPushButton::clicked, this, &MainWindow::useDefaultPath);
    connect(ui->closeAppBtn, &QPushButton::clicked, this, &MainWindow::close);

    // Table selection changes
    auto connectSelection = [](QTableWidget *table, auto slot, QObject *obj) {
        connect(table->selectionModel(), &QItemSelectionModel::selectionChanged, obj, slot);
    };
    connectSelection(ui->odAveragesTw, &MainWindow::onRowsSelectionChangedTable2, this);
    connectSelection(ui->osAveragesTw, &MainWindow::onRowsSelectionChangedTable4, this);

    // Calculation and data operations
    connect(ui->calculateOdDiffBtn, &QPushButton::clicked, this, &MainWindow::calculateAveragesDiff);
    connect(ui->calculateOsDiffBtn, &QPushButton::clicked, this, &MainWindow::calculateAveragesDiff_2);
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
    ui->statusTL_2->clear();
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

QString MainWindow::getDefaultSavePath() {
    return defaultSavePath;
}

void MainWindow::setFilesPath(const QString &path) {
    inputFilesPath = path;
}

void MainWindow::startExtracting() {
    ui->allDataTw->clearContents();
    ui->allDataTw->setRowCount(0);
    ui->odAveragesTw->clearContents();
    ui->odAveragesTw->setRowCount(0);
    ui->osAveragesTw->clearContents();
    ui->osAveragesTw->setRowCount(0);

    QString filePath = getFilesPath();
    ui->statusTL->setText("Files Path: " + filePath);

    QStringList csvFiles = getCsvFiles(filePath);
    if (csvFiles.isEmpty()) {
        ui->statusTL_2->setText("WARNING: No CSV files found to process.");
        ui->statusTL_2->setStyleSheet("color: red;");
        return;
    }
    ui->statusTL_2->clear();

    QMap<QString, QMap<QString, QList<double>>> averagesData;
    QMap<QString, QStringList> nameData;
    QMap<QString, int> fileCounts;

    for (const QString &fileName : csvFiles) {
        QString firstname, surname, date, time, eye;
        QMap<QString, QString> extractedValues = parseCsvFile(filePath + "/" + fileName, firstname, surname, date, time, eye);

        int recentRow = ui->allDataTw->rowCount();
        ui->allDataTw->insertRow(recentRow);
        populateTableRow(recentRow, firstname, surname, date, time, eye, extractedValues);

        QString key = surname + ", " + firstname + " - " + date + " (" + eye + ")";
        nameData[key] = {surname, firstname, date, eye};
        fileCounts[key] = fileCounts.value(key, 0) + 1;

        for (const auto &metric : extractedValues.keys()) {
            averagesData[key][metric].append(extractedValues[metric].toDouble());
        }
    }

    populateSplitAveragesTables(averagesData, nameData, fileCounts);

    for (auto *table : {ui->allDataTw, ui->odAveragesTw, ui->osAveragesTw}) {
        table->resizeColumnsToContents();
        table->resizeRowsToContents();
    }
}

QStringList MainWindow::getCsvFiles(const QString &path) {
    QDir folder(path);
    return folder.entryList(QStringList() << "*.csv", QDir::Files);
}

QMap<QString, QString> MainWindow::parseCsvFile(const QString &filePath, QString &firstname, QString &surname, QString &date, QString &time, QString &eye) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return {};

    QMap<QString, QString> extractedValues;
    QStringList fileParts = QFileInfo(filePath).fileName().split('_');

    if (fileParts.size() >= 5) {
        surname = fileParts[0];
        firstname = fileParts[1];
        eye = fileParts[2];
        date = formatDate(fileParts[3]);
        time = formatTime(fileParts[4].section('.', 0, 0));
    }

    QTextStream in(&file);
    static const QSet<QString> labels = {"Cornea Front Rh", "Cornea Front Rv", "Pachy Min", "K Max (Front)", "QS Error"};

    while (!in.atEnd()) {
        QStringList parts = in.readLine().split(';');
        if (parts.size() >= 2 && labels.contains(parts[0].trimmed())) {
            extractedValues[parts[0].trimmed()] = parts[1].trimmed();
        }
    }
    return extractedValues;
}

void MainWindow::populateTableRow(int row, const QString &firstname, const QString &surname,
                                  const QString &date, const QString &time, const QString &eye,
                                  const QMap<QString, QString> &extractedValues) {
    QStringList values = {surname, firstname, date, time, eye, extractedValues.value("QS Error", "N/A")};

    for (int i = 0; i < values.size(); ++i) {
        setTableItem(ui->allDataTw, row, i, values[i], (i < 2) ? Qt::AlignLeft | Qt::AlignVCenter : Qt::AlignCenter);
    }

    static const QStringList labels = {"Cornea Front Rh", "Cornea Front Rv", "Pachy Min", "K Max (Front)"};
    int col = values.size();

    for (const auto &label : labels) {
        setTableItem(ui->allDataTw, row, col++, extractedValues.value(label, "N/A"), Qt::AlignCenter);
    }
}

void MainWindow::populateSplitAveragesTables(const QMap<QString, QMap<QString, QList<double>>> &averagesData,
                                             const QMap<QString, QStringList> &nameData,
                                             const QMap<QString, int> &fileCounts) {
    int rowOD = 0, rowOS = 0;

    static const QStringList metrics = {"Cornea Front Rh", "Cornea Front Rv", "Pachy Min", "K Max (Front)"};

    for (auto it = averagesData.begin(); it != averagesData.end(); ++it) {
        const QStringList &nameFields = nameData[it.key()];
        const QString &eye = nameFields[3];

        QTableWidget *targetTable = (eye == "OD") ? ui->odAveragesTw : ui->osAveragesTw;
        int &recentRow = (eye == "OD") ? rowOD : rowOS;

        targetTable->insertRow(recentRow);

        // Insert name details
        for (int i = 0; i < nameFields.size(); ++i) {
            QTableWidgetItem *item = new QTableWidgetItem(nameFields[i]);
            item->setTextAlignment((i < 2) ? (Qt::AlignLeft | Qt::AlignVCenter) : Qt::AlignCenter);
            targetTable->setItem(recentRow, i, item);
        }

        const auto &data = it.value();  // Avoid redundant lookups
        int col = 4;

        for (const auto &metric : metrics) {
            const QList<double> &values = data[metric];

            double sum = std::accumulate(values.begin(), values.end(), 0.0);
            double average = values.isEmpty() ? 0.0 : sum / values.size();

            QString displayValue = (metric == "Cornea Front Rh" || metric == "Cornea Front Rv")
                                       ? QString::number(337.5 / average, 'f', 1)
                                       : (metric == "Pachy Min")
                                             ? QString::number(qRound(average))
                                             : QString::number(average, 'f', 2);

            QTableWidgetItem *item = new QTableWidgetItem(displayValue);
            item->setTextAlignment(Qt::AlignCenter);
            targetTable->setItem(recentRow, col++, item);
        }

        // Insert file count
        QTableWidgetItem *countItem = new QTableWidgetItem(QString::number(fileCounts.value(it.key(), 0)));
        countItem->setTextAlignment(Qt::AlignCenter);
        targetTable->setItem(recentRow, col, countItem);

        ++recentRow;
    }
}

void MainWindow::populateAveragesTable(const QMap<QString, QMap<QString, QList<double>>> &averagesData,
                                       const QMap<QString, QStringList> &nameData,
                                       const QMap<QString, int> &fileCounts) {
    int row = 0;

    static const QStringList metrics = {"Cornea Front Rh", "Cornea Front Rv", "Pachy Min", "K Max (Front)"};

    for (auto it = averagesData.begin(); it != averagesData.end(); ++it) {
        ui->odAveragesTw->insertRow(row);

        const QStringList &nameFields = nameData[it.key()]; // Avoid redundant lookups

        for (int i = 0; i < nameFields.size(); ++i) {
            QTableWidgetItem *item = new QTableWidgetItem(nameFields[i]);
            item->setTextAlignment((i < 2) ? (Qt::AlignLeft | Qt::AlignVCenter) : Qt::AlignCenter);
            ui->odAveragesTw->setItem(row, i, item);
        }

        const auto &data = it.value();  // Avoid redundant map lookups
        int col = 4;

        for (const auto &metric : metrics) {
            const QList<double> &values = data[metric];

            double sum = std::accumulate(values.begin(), values.end(), 0.0);
            double average = values.isEmpty() ? 0.0 : sum / values.size();

            QString displayValue = (metric == "Cornea Front Rh" || metric == "Cornea Front Rv")
                                       ? QString::number(337.5 / average, 'f', 1)
                                       : (metric == "Pachy Min")
                                             ? QString::number(qRound(average))
                                             : QString::number(average, 'f', 2);

            QTableWidgetItem *item = new QTableWidgetItem(displayValue);
            item->setTextAlignment(Qt::AlignCenter);
            ui->odAveragesTw->setItem(row, col++, item);
        }

        // Insert file count
        QTableWidgetItem *countItem = new QTableWidgetItem(QString::number(fileCounts.value(it.key(), 0)));
        countItem->setTextAlignment(Qt::AlignCenter);
        ui->odAveragesTw->setItem(row, col, countItem);

        ++row;
    }
}

void MainWindow::applyTableFormatting(QTableWidget *table) {
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    const int rowCount = table->rowCount();
    const int colCount = table->columnCount();

    for (int row = 0; row < rowCount; ++row) {
        for (int col = 0; col < colCount; ++col) {
            QTableWidgetItem *item = table->item(row, col);
            if (!item) continue;  // Skip empty cells

            item->setTextAlignment((col < 2) ? (Qt::AlignLeft | Qt::AlignVCenter) : Qt::AlignCenter);
        }
    }
}

void MainWindow::clearAllTables() {
    for (QTableWidget *table : {ui->allDataTw, ui->odAveragesTw, ui->osAveragesTw, ui->deltaTw}) {
        table->clearContents();
        table->setRowCount(0);
    }

    for (QLabel *label : {ui->rowsSelectionWarningL, ui->rowsSelectionWarningL_2}) {
        label->clear();
    }

    ui->saveToCsvFileBtn->setEnabled(false);
}

void MainWindow::clearAllSelections() {
    for (auto *table : {ui->allDataTw, ui->odAveragesTw, ui->osAveragesTw, ui->deltaTw}) {
        table->clearSelection();
    }
    ui->rowsSelectionWarningL->clear();
    ui->rowsSelectionWarningL_2->clear();
}

void MainWindow::saveToCsvFile() {
    auto setWarning = [&](const QString &message, const QString &color) {
        ui->savingWarningL->setText(message);
        ui->savingWarningL->setStyleSheet("color: " + color + ";");
    };

    if (ui->deltaTw->rowCount() == 0) {
        setWarning("Warning: The table is empty. No data to save.", "red");
        return;
    }

    QString saveDir = getDefaultSavePath();
    QString savePath = saveDir + "/output_data_" + QDateTime::currentDateTime().toString("ddMMyyyy_HHmmss") + ".csv";

    if (!QDir(saveDir).mkpath(".")) {
        setWarning("Error: Could not create save directory.", "red");
        return;
    }

    QFile file(savePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        setWarning("Error: Could not save file.", "red");
        return;
    }

    QTextStream out(&file);

    // Write headers
    QStringList headers;
    for (int col = 0; col < ui->deltaTw->columnCount(); ++col) {
        QTableWidgetItem *headerItem = ui->deltaTw->horizontalHeaderItem(col);
        headers << (headerItem ? headerItem->text() : "");
    }
    out << headers.join(",") << "\n";

    // Write table data
    for (int row = 0; row < ui->deltaTw->rowCount(); ++row) {
        QStringList rowValues;
        for (int col = 0; col < ui->deltaTw->columnCount(); ++col) {
            QTableWidgetItem *item = ui->deltaTw->item(row, col);
            rowValues << (item ? item->text() : "");
        }
        out << rowValues.join(",") << "\n";
    }

    if (out.status() != QTextStream::Ok) {
        setWarning("Error: Failed to write data to file.", "red");
        return;
    }

    setWarning("Table saved to: " + savePath, "green");
    ui->showInFinderBtn->setEnabled(true);
}

void MainWindow::openFileLocationInFinder() {
    QString folderPath = getDefaultSavePath(); // Ensure this returns the directory where files are saved

    // Open Finder at the specified folder
    QProcess::startDetached("open", QStringList() << folderPath);
}

void MainWindow::calculateAveragesDiff() {
    //calculateAveragesDiffTable(ui->odAveragesTw, ui->rowsSelectionWarningL);
    calculateAveragesDiffForTable(ui->odAveragesTw, ui->rowsSelectionWarningL);
    applyTableFormatting(ui->odAveragesTw);
}

void MainWindow::calculateAveragesDiff_2() {
    //calculateAveragesDiffTable(ui->osAveragesTw, ui->rowsSelectionWarningL_2);
    calculateAveragesDiffForTable(ui->osAveragesTw, ui->rowsSelectionWarningL_2);
    applyTableFormatting(ui->osAveragesTw);
}

void MainWindow::calculateAveragesDiffForTable(QTableWidget *sourceTable, QLabel *warningLabel) {
    warningLabel->clear();

    QModelIndexList selectedRows = sourceTable->selectionModel()->selectedRows();
    if (selectedRows.size() != 2) {
        warningLabel->setText("Please select exactly two rows.");
        warningLabel->setStyleSheet("color: red;");
        return;
    }

    int row1 = selectedRows.at(0).row();
    int row2 = selectedRows.at(1).row();

    const auto getText = [&](int row, int col) {
        return sourceTable->item(row, col) ? sourceTable->item(row, col)->text() : QString();
    };

    QString surname1 = getText(row1, 0), firstname1 = getText(row1, 1), eye1 = getText(row1, 3);
    QString surname2 = getText(row2, 0), firstname2 = getText(row2, 1), eye2 = getText(row2, 3);

    if (surname1 != surname2 || firstname1 != firstname2 || eye1 != eye2) {
        warningLabel->setText("Error: Selected rows must be for the same patient and eye.");
        warningLabel->setStyleSheet("color: red;");
        return;
    }

    calculateAveragesDiffTable(sourceTable, warningLabel, row1, row2);
    applyTableFormatting(sourceTable);
}

void MainWindow::calculateAveragesDiffTable(QTableWidget *sourceTable, QLabel *warningLabel, int row1, int row2) {
    warningLabel->clear();

    const auto getText = [&](int row, int col) {
        return sourceTable->item(row, col) ? sourceTable->item(row, col)->text() : QString();
    };

    // Get patient details and dates
    QString surname = getText(row1, 0);
    QString firstname = getText(row1, 1);
    QString eye = getText(row1, 3);
    QString baselineDate = getText(row1, 2);
    QString recentDate = getText(row2, 2);

    // Determine the baseline and recent row based on the date order
    QDate date1 = QDate::fromString(baselineDate, "dd-MM-yyyy");
    QDate date2 = QDate::fromString(recentDate, "dd-MM-yyyy");
    int baselineRow = (date1 <= date2) ? row1 : row2;
    int recentRow = (date1 <= date2) ? row2 : row1;
    baselineDate = getText(baselineRow, 2);
    recentDate = getText(recentRow, 2);

    // Clear previous selections
    sourceTable->clearSelection();

    // Check for duplicate entries in deltaTw
    if (isDuplicateEntry(ui->deltaTw, baselineDate, recentDate, surname, firstname, eye)) {
        return;
    }

    // Insert comparison row into deltaTw
    int newRow = ui->deltaTw->rowCount();
    ui->deltaTw->insertRow(newRow);
    insertComparisonRow(ui->deltaTw, newRow, baselineRow, recentRow, sourceTable);

    // Apply formatting
    applyTableFormatting(ui->deltaTw);
    ui->saveToCsvFileBtn->setEnabled(true);
}

bool MainWindow::isDuplicateEntry(QTableWidget *table, const QString &baselineDate, const QString &recentDate,
                                  const QString &surname, const QString &firstname, const QString &eye) {
    for (int row = 0; row < table->rowCount(); ++row) {
        if (table->item(row, 0)->text() == surname &&
            table->item(row, 1)->text() == firstname &&
            table->item(row, 2)->text() == eye &&
            table->item(row, 3)->text() == baselineDate &&
            table->item(row, 8)->text() == recentDate) {

            table->selectRow(row);
            table->scrollToItem(table->item(row, 0));
            return true;
        }
    }
    return false;
}

void MainWindow::insertComparisonRow(QTableWidget *table, int newRow, int baselineRow, int recentRow, QTableWidget *sourceTable) {
    const auto getText = [&](int row, int col) {
        return sourceTable->item(row, col) ? sourceTable->item(row, col)->text() : QString();
    };

    const auto getDouble = [&](int row, int col) {
        return getText(row, col).toDouble();
    };

    // Patient Info & Dates
    QString surname = getText(baselineRow, 0);
    QString firstname = getText(baselineRow, 1);
    QString eye = getText(baselineRow, 3);
    QString baselineDate = getText(baselineRow, 2);
    QString recentDate = getText(recentRow, 2);

    // Values from baseline and recent rows
    double bK1 = getDouble(baselineRow, 4);
    double bK2 = getDouble(baselineRow, 5);
    double bPachyMin = getDouble(baselineRow, 6);
    double bKMax = getDouble(baselineRow, 7);

    double rK1 = getDouble(recentRow, 4);
    double rK2 = getDouble(recentRow, 5);
    double rPachyMin = getDouble(recentRow, 6);
    double rKMax = getDouble(recentRow, 7);

    // Insert data in correct order with coloring
    int col = 0;
    setTableItemWithColor(table, newRow, col++, surname, "white");
    setTableItemWithColor(table, newRow, col++, firstname, "white");
    setTableItemWithColor(table, newRow, col++, eye, "white");
    setTableItemWithColor(table, newRow, col++, baselineDate, "lightblue");

    setTableItemWithColor(table, newRow, col++, QString::number(bK1, 'f', 2), "lightblue");
    setTableItemWithColor(table, newRow, col++, QString::number(bK2, 'f', 2), "lightblue");
    setTableItemWithColor(table, newRow, col++, QString::number(bPachyMin, 'f', 2), "lightblue");
    setTableItemWithColor(table, newRow, col++, QString::number(bKMax, 'f', 2), "lightblue");

    setTableItemWithColor(table, newRow, col++, recentDate, "lightgreen");

    setTableItemWithColor(table, newRow, col++, QString::number(rK1, 'f', 2), "lightgreen");
    setTableItemWithColor(table, newRow, col++, QString::number(rK2, 'f', 2), "lightgreen");
    setTableItemWithColor(table, newRow, col++, QString::number(rPachyMin, 'f', 2), "lightgreen");
    setTableItemWithColor(table, newRow, col++, QString::number(rKMax, 'f', 2), "lightgreen");

    // Calculate differences and insert them with orange color
    setTableItemWithColor(table, newRow, col++, QString::number(rK1 - bK1, 'f', 2), "orange");
    setTableItemWithColor(table, newRow, col++, QString::number(rK2 - bK2, 'f', 2), "orange");
    setTableItemWithColor(table, newRow, col++, QString::number(rPachyMin - bPachyMin, 'f', 2), "orange");
    setTableItemWithColor(table, newRow, col++, QString::number(rKMax - bKMax, 'f', 2), "orange");
}

void MainWindow::setTableItemWithColor(QTableWidget *table, int row, int col, const QString &value, const QString &color) {
    auto *item = new QTableWidgetItem(value);
    item->setBackground(QColor(color));
    item->setTextAlignment(Qt::AlignCenter);
    table->setItem(row, col, item);
}

void MainWindow::setTableItem(QTableWidget *table, int row, int col, const QString &value, Qt::Alignment alignment) {
    QTableWidgetItem *item = new QTableWidgetItem(value);
    item->setTextAlignment(alignment);
    table->setItem(row, col, item);
}

void MainWindow::onRowsSelectionChangedTable2() {
    QModelIndexList selectedRows = ui->odAveragesTw->selectionModel()->selectedRows();
    if (selectedRows.size() != 2) {
        ui->rowsSelectionWarningL->clear();
        ui->calculateOdDiffBtn->setEnabled(false);
        return;
    }

    const auto getText = [&](int row, int col) { return ui->odAveragesTw->item(row, col)->text(); };

    int row1 = selectedRows.at(0).row();
    int row2 = selectedRows.at(1).row();

    bool nameMismatch = (getText(row1, 0) != getText(row2, 0)) || (getText(row1, 1) != getText(row2, 1));

    auto setWarning = [&](const QString &message, bool enableButton) {
        ui->rowsSelectionWarningL->setText(message);
        ui->rowsSelectionWarningL->setStyleSheet(message.isEmpty() ? "" : "color: red;");
        ui->calculateOdDiffBtn->setEnabled(enableButton);
    };

    setWarning(nameMismatch ? "WARNING: You have selected two different patients" : "", !nameMismatch);
}

void MainWindow::onRowsSelectionChangedTable4() {
    QModelIndexList selectedRows = ui->osAveragesTw->selectionModel()->selectedRows();
    if (selectedRows.size() != 2) {
        ui->rowsSelectionWarningL_2->clear();
        ui->calculateOsDiffBtn->setEnabled(false);
        return;
    }

    const auto getText = [&](int row, int col) { return ui->osAveragesTw->item(row, col)->text(); };

    int row1 = selectedRows.at(0).row();
    int row2 = selectedRows.at(1).row();

    bool nameMismatch = (getText(row1, 0) != getText(row2, 0)) || (getText(row1, 1) != getText(row2, 1));

    ui->rowsSelectionWarningL_2->setText(nameMismatch ? "WARNING: You have selected two different patients" : "");
    ui->rowsSelectionWarningL_2->setStyleSheet(nameMismatch ? "color: red;" : "");
    ui->calculateOsDiffBtn->setEnabled(!nameMismatch);
}

QString MainWindow::formatDate(const QString &rawDate) {
    return QDate::fromString(rawDate, "ddMMyyyy").toString("dd-MM-yyyy");
}

QString MainWindow::formatTime(const QString &rawTime) {
    return QTime::fromString(rawTime, "hhmmss").toString("hh:mm:ss");
}
