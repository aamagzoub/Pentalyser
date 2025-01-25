#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "gettersandsetters.h"
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    float formulaFactor = 337.5;

    QString filesPath;
    QString defaultPath = ("/Users/emagabu/workspace/Pentalyser/inputCsvFiles");

    void setUiButtons();
    void populateTableRow(int row, const QString &firstname, const QString &surname, const QString &date, const QString &time, const QString &eye, const QMap<QString, QString> &extractedValues);
    QString formatDate(const QString &rawDate);
    QString formatTime(const QString &rawTime);
    QString calculateFormula(const QString &value);
    QStringList getCsvFiles(const QString &path);
    QMap<QString, QString> parseCsvFile(const QString &filePath, QString &firstname, QString &surname, QString &date, QString &time, QString &eye);
    void setupTableWidget();
private:
    Ui::MainWindow *ui;
    gettersAndSetters *mpGettersAndSetters;
private slots:
    void browseFolders();
    void useDefaultPath();
    void featureToBeIntroduced();
    void countNumberOfRowsSelected();
    void compareSelectedRows();
    void showAverages();
    void clearRowsSelection();
    void readCsvFiles();
};

#endif // MAINWINDOW_H
