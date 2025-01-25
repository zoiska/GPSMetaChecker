#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <QString>

#include "includes/TinyExif.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    Ui::MainWindow *ui;
    bool checkCoords(const std::string& filePath);
    void processFolder();
    std::wstring OpenFolderDialog();
    void deleteMarked();

    std::vector<std::string> imagePaths;
    char delimiter = '\\';
};


#endif //MAINWINDOW_H
