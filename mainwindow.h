#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QMap>
#include <QStringList>
#include <QTableWidget>

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
private:
    Ui::MainWindow *ui;
    QString defaultPath = "/Users/emagabu/workspace/Pentalyser/inputCsvFiles";
    QString inputFilesPath;

private slots:
    void browseFolders();
    void useDefaultPath();
    void startExtracting();
};

#endif // MAINWINDOW_H
