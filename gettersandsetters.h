#ifndef GETTERSANDSETTERS_H
#define GETTERSANDSETTERS_H

#include <QObject>

class gettersAndSetters
{
public:
    gettersAndSetters();
    QString inputFilesPath;
    QString getInputFilesPath();
    void setInputFilesPath(QString path);
};

#endif // GETTERSANDSETTERS_H
