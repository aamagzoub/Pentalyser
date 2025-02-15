#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QMap>
#include <QStringList>
#include <QTableWidget>
#include <QLabel>

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
    QString calculateFormula(const QString &value);
    QStringList getCsvFiles(const QString &path);
    QMap<QString, QString> parseCsvFile(const QString &filePath, QString &firstname, QString &surname, QString &date, QString &time, QString &eye);
    void populateTableRow(int row, const QString &firstname, const QString &surname, const QString &date, const QString &time, const QString &eye, const QMap<QString, QString> &extractedValues);
    void populateAveragesTable(const QMap<QString, QMap<QString, QList<double> > > &averagesData, const QMap<QString, QStringList> &nameData, const QMap<QString, int> &fileCounts);
    void populateSplitAveragesTables(const QMap<QString, QMap<QString, QList<double> > > &averagesData, const QMap<QString, QStringList> &nameData, const QMap<QString, int> &fileCounts);
    //void calculateAveragesDiffTable(QTableWidget *sourceTable, QTableWidget *targetTable, QLabel *warningLabel);
    void calculateAveragesDiffTable(QTableWidget *sourceTable, QTableWidget *targetTable, QLabel *warningLabel);

    void populateResultsTable(QTableWidget *targetTable, const QString &surname, const QString &firstname, const QString &eye, double diffK1, double diffK2, double diffPachyMin, double diffKMax, const QString &baselineDate, const QString &currentDate);
    void formatResultsTable(QTableWidget *table);
    //void applyTableFormatting(QTableWidget *table);
    void applyTableFormatting(QTableWidget *table, bool colorize);
private:
    Ui::MainWindow *ui;
    QString defaultPath = "/Users/emagabu/workspace/Pentalyser/inputCsvFiles";
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
};

#endif // MAINWINDOW_H
