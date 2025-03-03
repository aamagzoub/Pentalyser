#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QMap>
#include <QStringList>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QString getFilesPath();
    void setFilesPath(const QString &path);
    QString formatDate(const QString &rawDate);
    QString formatTime(const QString &rawTime);
    QStringList getCsvFiles(const QString &path);
    QMap<QString, QString> parseCsvFile(const QString &filePath, QString &firstname, QString &surname, QString &date, QString &time, QString &eye);
    void populateTableRow(int row, const QString &firstname, const QString &surname, const QString &date, const QString &time, const QString &eye, const QMap<QString, QString> &extractedValues);
    void populateAveragesTable(const QMap<QString, QMap<QString, QList<double>>>& averagesData, const QMap<QString, QStringList>& nameData, const QMap<QString, int>& fileCounts);
    void populateSplitAveragesTables(const QMap<QString, QMap<QString, QList<double>>>& averagesData, const QMap<QString, QStringList>& nameData, const QMap<QString, int>& fileCounts);
    void populateResultsTable(QTableWidget *targetTable, const QString &surname, const QString &firstname, const QString &eye, double diffK1, double diffK2, double diffPachyMin, double diffKMax, const QString &baselineDate, const QString &currentDate);
    void formatResultsTable(QTableWidget *table);
    void populateTableWidget5WithAveragesAndDeltas(const QMap<QString, QMap<QString, QList<double>>>& averagesData, const QMap<QString, QStringList>& nameData);
    void calculateAveragesDiffTable(QTableWidget *sourceTable, QLabel *warningLabel);
    void applyTableFormatting(QTableWidget *table);
    QString getDefaultSavePath();

    void onRowsSelectionChanged(QTableWidget *table, QLabel *warningLabel, QPushButton *diffButton);
    void calculateAveragesDiffForTable(QTableWidget *sourceTable, QLabel *warningLabel);
    void calculateAveragesDiffTable(QTableWidget *sourceTable, QLabel *warningLabel, int row1, int row2);
    bool isDuplicateEntry(QTableWidget *table, const QString &baselineDate, const QString &recentDate, const QString &surname, const QString &firstname, const QString &eye);
    void insertComparisonRow(QTableWidget *table, int newRow, int baselineRow, int recentRow, QTableWidget *sourceTable);
    void setTableItem(QTableWidget *table, int row, int col, const QString &value, Qt::Alignment alignment);
    void setTableItemWithColor(QTableWidget *table, int row, int col, const QString &value, const QString &color);

private:
    Ui::MainWindow *ui;
    // QString defaultPath = "/Users/mahmoud/workspace/pentalyser/input_csv_files";
    // QString defaultSavePath = "/Users/mahmoud/workspace/pentalyser/output_csv_files";

    QString defaultPath = "/Users/emagabu/workspace/Pentalyser/input_csv_files";
    QString defaultSavePath = "/Users/emagabu/workspace/Pentalyser/output_csv_files";

    QString inputFilesPath;

private slots:
    void browseFolders();
    void useDefaultPath();
    void startExtracting();
    void onRowsSelectionChangedTable2();
    void onRowsSelectionChangedTable4();
    void calculateAveragesDiff();
    void calculateAveragesDiff_2();
    void clearAllTables();
    void clearAllSelections();
    void saveToCsvFile();
    void openFileLocationInFinder();
};

#endif // MAINWINDOW_H
