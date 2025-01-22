#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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

    QString filesPath;
    QString defaultPath = ("/Users/emagabu/workspace/Pentalyser/inputCsvFiles");

private:
    Ui::MainWindow *ui;
    void setFilesPath(QString path);
    QString getFilesPath();

private slots:
    void browseFolders();
    void readCsvFiles(QString);
    void useDefaultPath();
    void setPathAsDefault();
    void startAnalysis();
};

#endif // MAINWINDOW_H
