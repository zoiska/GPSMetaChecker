#include "MainWindow.h"
#include "ui_MainWIndow.h"
#include <windows.h>
#include <shobjidl.h>
#include <iostream>
#include <string>
#include <QFile>
#include <QString>
#include <QImageReader>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    ui->label_output->clear();
    ui->label_directory->clear();

    connect(ui->button_openFileDialog, &QPushButton::clicked, this, &MainWindow::processFolder);
    connect(ui->button_run, &QPushButton::clicked, this, &MainWindow::deleteMarked);
}

MainWindow::~MainWindow() {
    delete ui;
}

bool MainWindow::checkCoords(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        size_t delim_position = filePath.rfind(delimiter);
        ui->label_output->setText(ui->label_output->text() + "Error: Could not open file \"" + QString::fromStdString((delim_position != std::string::npos) ? filePath.substr(delim_position + 1) : "") + "\"\n");
        return false;
    }

    std::vector<unsigned char> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    TinyEXIF::EXIFInfo exif(data.data(), data.size());
    if (!exif.Fields) {
        size_t delim_position = filePath.rfind(delimiter);
        ui->label_output->setText(ui->label_output->text() + "No EXIF data found in file \"" + QString::fromStdString((delim_position != std::string::npos) ? filePath.substr(delim_position + 1) : "") + "\"\n");
        return true;
    }

    if (exif.GeoLocation.hasLatLon()) {
        if (exif.GeoLocation.Latitude == 0.0 && exif.GeoLocation.Longitude == 0.0) {
            size_t delim_position = filePath.rfind(delimiter);
            ui->label_output->setText(ui->label_output->text() + "Invalid GPS data found in: \"" + QString::fromStdString((delim_position != std::string::npos) ? filePath.substr(delim_position + 1) : "") + "\"\n");
            return true;
        }
        return false;
    }

    size_t delim_position = filePath.rfind(delimiter);
    ui->label_output->setText(ui->label_output->text() + "No GPS data found in: \"" + QString::fromStdString((delim_position != std::string::npos) ? filePath.substr(delim_position + 1) : "") + "\"\n");
    return true;
}

void MainWindow::processFolder() {
    std::wstring wstr = OpenFolderDialog();
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::string folderPath = converter.to_bytes(wstr);

    ui->label_directory->setText(QString::fromStdString(folderPath));

    if(std::filesystem::exists(folderPath)) {
        for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
            if (entry.is_regular_file()) {
                const auto& filePath = entry.path().string();

                std::string extension = entry.path().extension().string();
                if (extension == ".jpg" || extension == ".jpeg" || extension == ".JPG" || extension == ".JPEG" || extension == ".png" || extension == ".PNG") {
                    if (checkCoords(filePath)) {
                        imagePaths.push_back(filePath);

                        if (!filePath.empty()) {
                            auto *item = new QListWidgetItem(QIcon(QString::fromStdString(filePath)), QString::fromStdString(filePath));
                            item->setData(Qt::UserRole, QString::fromStdString(filePath));
                            ui->listWidget->addItem(item);
                        }
                        else {
                            size_t delim_position = filePath.rfind(delimiter);
                            ui->label_output->setText(ui->label_output->text() + "Cannot image preview: \"" + QString::fromStdString((delim_position != std::string::npos) ? filePath.substr(delim_position + 1) : "") + "\"\n\n");
                        }
                        size_t delim_position = filePath.rfind(delimiter);
                        ui->label_output->setText(ui->label_output->text() + "Adding for deletion: \"" + QString::fromStdString((delim_position != std::string::npos) ? filePath.substr(delim_position + 1) : "") + "\"\n\n");
                    }
                }
            }
        }
    }
    else {
        ui->label_output->setText(ui->label_output->text() + "Error: Invalid path.\n");
    }
}

std::wstring MainWindow::OpenFolderDialog() {
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) return L"";

    IFileDialog* pFileDialog;
    hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFileDialog));
    if (FAILED(hr)) {
        CoUninitialize();
        return L"";
    }

    DWORD options;
    pFileDialog->GetOptions(&options);
    pFileDialog->SetOptions(options | FOS_PICKFOLDERS);

    hr = pFileDialog->Show(NULL);
    if (SUCCEEDED(hr)) {
        IShellItem* pItem;
        hr = pFileDialog->GetResult(&pItem);
        if (SUCCEEDED(hr)) {
            PWSTR pszFilePath;
            pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

            std::wstring folderPath = pszFilePath;
            CoTaskMemFree(pszFilePath);
            pItem->Release();
            pFileDialog->Release();
            CoUninitialize();
            return folderPath;
        }
    }

    pFileDialog->Release();
    CoUninitialize();
    return L"";
}

void MainWindow::deleteMarked() {
    if (!imagePaths.empty()) {
        for (const auto& path : imagePaths) {
            QString qPath = QString::fromStdString(path);
            QFile::moveToTrash(qPath);
            size_t delim_position = qPath.toStdString().rfind(delimiter);
            ui->label_output->setText(ui->label_output->text() + "Deleting file: \"" + QString::fromStdString((delim_position != std::string::npos) ? qPath.toStdString().substr(delim_position + 1) : "") + "\"\n\n");
        }
    }
    else {
        ui->label_output->setText(ui->label_output->text() + "Error: Vector is empty.\n");
    }
}