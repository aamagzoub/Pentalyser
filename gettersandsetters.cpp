#include "gettersandsetters.h"

gettersAndSetters::gettersAndSetters() {}

QString gettersAndSetters::getInputFilesPath() {
    return inputFilesPath;

}

void gettersAndSetters::setInputFilesPath(QString path) {
    inputFilesPath = path;
}
