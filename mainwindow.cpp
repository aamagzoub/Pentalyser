#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "gettersandsetters.h"

#include <QString>
#include <QDir>
#include <QTextStream>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Pentalyser");

    mpGettersAndSetters = new gettersAndSetters;

    connect(ui->selectPathBtn,SIGNAL(clicked()),this,SLOT(browseFolders()));
    connect(ui->useDefulatPathBtn,SIGNAL(clicked()),this,SLOT(useDefaultPath()));
    connect(ui->closeAppBtn,SIGNAL(clicked()),this,SLOT(close()));
    connect(ui->startExctractingBtn,SIGNAL(clicked()),this,SLOT(readCsvFiles()));
    connect(ui->showAveragesBtn,SIGNAL(clicked()),this,SLOT(compareData()));
    connect(ui->printBtn,SIGNAL(clicked()),this,SLOT(featureToBeIntroduced()));
    connect(ui->saveAsCsvBtn,SIGNAL(clicked()),this,SLOT(featureToBeIntroduced()));
    connect(ui->saveAsPdfBtn,SIGNAL(clicked()),this,SLOT(featureToBeIntroduced()));
    connect(ui->emailBtn,SIGNAL(clicked()),this,SLOT(featureToBeIntroduced()));
    connect(ui->tableWidget,SIGNAL(clicked(QModelIndex)),this,SLOT(countNumberOfRowsSelected()));
    connect(ui->compareDataBtn,SIGNAL(clicked()),this,SLOT(compareSelectedRows()));
    connect(ui->showAveragesBtn,SIGNAL(clicked()),this,SLOT(showAverages()));
    connect(ui->clearRowsSelectionBtn,SIGNAL(clicked()),this,SLOT(clearRowsSelection()));

    ui->tableWidget->setHorizontalHeaderLabels({
        "Surname", "First Name", "Date", "Time", "Eye", "K1", "K2", "Pachy Min", "KMax"
    });

}

void MainWindow::clearRowsSelection(){

    ui->tableWidget->clearSelection();
    ui->compareDataBtn->setEnabled(false);
    ui->clearRowsSelectionBtn->setEnabled(false);

}


void MainWindow::showAverages(){
    ui->tabWidget->setCurrentIndex(1);
}


void MainWindow::countNumberOfRowsSelected(){

    ui->clearRowsSelectionBtn->setEnabled(true);

    int numberOfRowsSelected = ui->tableWidget->selectionModel()->selectedRows().size();

    if (numberOfRowsSelected == 3) {
        ui->compareDataBtn->setEnabled(true);
    } else {
        ui->compareDataBtn->setEnabled(false);
    }
}

void MainWindow::compareSelectedRows(){
    featureToBeIntroduced();
}

void MainWindow::useDefaultPath(){
    ui->tableWidget->clearContents();
    mpGettersAndSetters->setInputFilesPath(defaultPath);
    ui->startExctractingBtn->setEnabled(true);
}

void MainWindow::browseFolders(){

    ui->tableWidget->clearContents();
    QString selectedPath = QFileDialog::getExistingDirectory(nullptr, "Select Folder", QDir::homePath());
    mpGettersAndSetters->setInputFilesPath(selectedPath);
    ui->startExctractingBtn->setEnabled(true);
}

void MainWindow::readCsvFiles() {
    ui->tableWidget->setRowCount(0);

    QString selectedPath = mpGettersAndSetters->getInputFilesPath(); // Use the getter function to retrieve the path

    QStringList csvFiles = getCsvFiles(selectedPath);
    if (csvFiles.isEmpty()) {
        int currentRow = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(currentRow);
        ui->tableWidget->setItem(currentRow, 0, new QTableWidgetItem("No CSV files found"));
        return;
    }

    for (const QString &fileName : csvFiles) {
        QString firstname, surname, date, time, eye;
        QMap<QString, QString> extractedValues = parseCsvFile(selectedPath + "/" + fileName, firstname, surname, date, time, eye);

        int currentRow = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(currentRow);
        populateTableRow(currentRow, firstname, surname, date, time, eye, extractedValues);
    }

    // Resize columns to fit content after populating data
    ui->tableWidget->resizeColumnsToContents();
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
        eye = (fileParts[2].toLower() == "od") ? "D" :
                  (fileParts[2].toLower() == "os") ? "S" :
                  "Unknown";
        date = formatDate(fileParts[3]);
        time = formatTime(fileParts[4].split('.').first());
    } else {
        firstname = "Unknown";
        surname = "Unknown";
        date = "Unknown";
        time = "Unknown";
        eye = "Unknown";
    }

    QTextStream in(&file);
    QStringList labels = {"Cornea Front Rh", "Cornea Front Rv", "Pachy Min", "K Max (Front)"};

    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList parts = line.split(';');

        if (parts.size() >= 2) {
            QString label = parts[0].trimmed();
            QString value = parts[1].trimmed();
            if (labels.contains(label)) {
                extractedValues[label] = (label == "Cornea Front Rh" || label == "Cornea Front Rv") ? calculateFormula(value) : value;
            }
        }
    }

    file.close();
    return extractedValues;
}

QString MainWindow::formatDate(const QString &rawDate) {
    QDate parsedDate = QDate::fromString(rawDate, "ddMMyyyy");
    return parsedDate.isValid() ? parsedDate.toString("dd-MM-yyyy") : "Invalid Date";
}

QString MainWindow::formatTime(const QString &rawTime) {
    QTime parsedTime = QTime::fromString(rawTime, "hhmmss");
    return parsedTime.isValid() ? parsedTime.toString("hh:mm:ss") : "Invalid Time";
}

QString MainWindow::calculateFormula(const QString &value) {
    bool ok;
    double numericValue = value.toDouble(&ok);
    return (ok && numericValue != 0) ? QString::number(337.5 / numericValue, 'f', 2) : "Invalid";
}

void MainWindow::populateTableRow(int row, const QString &firstname, const QString &surname, const QString &date, const QString &time, const QString &eye, const QMap<QString, QString> &extractedValues) {
    // Helper to create and configure table items
    auto createTableItem = [](const QString &text, Qt::Alignment alignment = Qt::AlignLeft | Qt::AlignVCenter) {
        QTableWidgetItem *item = new QTableWidgetItem(text);
        item->setTextAlignment(alignment);
        item->setFlags(item->flags() & ~Qt::ItemIsEditable); // Make item non-editable
        return item;
    };

    // Create and configure table items for each column
    ui->tableWidget->setItem(row, 0, createTableItem(surname)); // Surname
    ui->tableWidget->setItem(row, 1, createTableItem(firstname)); // First Name
    ui->tableWidget->setItem(row, 2, createTableItem(date, Qt::AlignCenter)); // Date
    ui->tableWidget->setItem(row, 3, createTableItem(time, Qt::AlignCenter)); // Time
    ui->tableWidget->setItem(row, 4, createTableItem(eye, Qt::AlignCenter)); // Eye
    ui->tableWidget->setItem(row, 5, createTableItem(extractedValues.value("Cornea Front Rh", "Not found"), Qt::AlignCenter)); // K1
    ui->tableWidget->setItem(row, 6, createTableItem(extractedValues.value("Cornea Front Rv", "Not found"), Qt::AlignCenter)); // K2
    ui->tableWidget->setItem(row, 7, createTableItem(extractedValues.value("Pachy Min", "Not found"), Qt::AlignCenter)); // Pachy Min
    ui->tableWidget->setItem(row, 8, createTableItem(extractedValues.value("K Max (Front)", "Not found"), Qt::AlignCenter)); // KMax
}

void MainWindow::setupTableWidget() {
    // Set resize mode for all columns
    for (int i = 0; i < ui->tableWidget->columnCount(); ++i) {
        if (i == 0 || i == 1) {
            // Surname and First Name columns dynamically resize
            ui->tableWidget->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
        } else {
            // All other columns have fixed size
            ui->tableWidget->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Fixed);
            ui->tableWidget->setColumnWidth(i, 120); // Fixed size for other columns
        }
    }

    // Disable word wrapping globally
    ui->tableWidget->setWordWrap(false);

    // Enable horizontal scrolling for large tables
    ui->tableWidget->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

    // Update column headers
    QStringList headers = {"Surname", "First Name", "Date", "Time", "Eye", "K1", "K2", "Pachy Min", "KMax"};
    ui->tableWidget->setHorizontalHeaderLabels(headers);
}


void MainWindow::setUiButtons(){
    ui->showAveragesBtn->setEnabled(true);
    ui->startExctractingBtn->setEnabled(false);
    ui->printBtn->setEnabled(true);
    ui->saveAsCsvBtn->setEnabled(true);
    ui->saveAsPdfBtn->setEnabled(true);
    ui->emailBtn->setEnabled(true);
    ui->showAveragesBtn->setEnabled(true);
}

void MainWindow::featureToBeIntroduced(){
    qDebug() << "This feature is yet to be introdcued, apologies for the inconvenience ";
}

MainWindow::~MainWindow()
{
    delete ui;
}



























