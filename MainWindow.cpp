#include "MainWindow.h"
#include "ui_MainWIndow.h"
#include <windows.h>
#include <shobjidl.h>
#include <iostream>
#include <string>
#include <QFile>
#include <QString>
#include <QPixmap>
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
        ui->label_output->setText(ui->label_output->text() + "\nError: Could not open file \"" + QString::fromStdString((delim_position != std::string::npos) ? filePath.substr(delim_position + 1) : "") + "\"");
        return false;
    }

    std::vector<unsigned char> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    TinyEXIF::EXIFInfo exif(data.data(), data.size());
    if (!exif.Fields) {
        size_t delim_position = filePath.rfind(delimiter);
        ui->label_output->setText(ui->label_output->text() + "\nNo EXIF data found in file \"" + QString::fromStdString((delim_position != std::string::npos) ? filePath.substr(delim_position + 1) : "") + "\"");
        return true;
    }

    if (exif.GeoLocation.hasLatLon()) {
        if (exif.GeoLocation.Latitude == 0.0 && exif.GeoLocation.Longitude == 0.0) {
            size_t delim_position = filePath.rfind(delimiter);
            ui->label_output->setText(ui->label_output->text() + "\nInvalid GPS data found in: \"" + QString::fromStdString((delim_position != std::string::npos) ? filePath.substr(delim_position + 1) : "") + "\"");
            return true;
        }
        return false;
    }

    size_t delim_position = filePath.rfind(delimiter);
    ui->label_output->setText(ui->label_output->text() + "\nNo GPS data found in: \"" + QString::fromStdString((delim_position != std::string::npos) ? filePath.substr(delim_position + 1) : "") + "\"");
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
                if (extension == ".jpg" || extension == ".jpeg" || extension == ".png" || extension == ".PNG" || extension == ".JPG" || extension == ".JPEG") {
                    if (checkCoords(filePath)) {
                        imagePaths.push_back(filePath);


                        // FUCKING QT JPEG, JPG FORMATS NOT WORKING ?????


                        QImageReader reader(QString::fromStdString(filePath));

                        QImage img = reader.read();
                        img = img.convertToFormat(QImage::Format_RGB32);
                        if (!img.isNull()) {
                            QPixmap scaledPixmap = QPixmap::fromImage(img).scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                            auto *item = new QListWidgetItem(QIcon(scaledPixmap), QString::fromStdString(filePath));
                            item->setData(Qt::UserRole, QString::fromStdString(filePath));
                            ui->listWidget->addItem(item);
                        }
                        else {
                            size_t delim_position = filePath.rfind(delimiter);
                            ui->label_output->setText(ui->label_output->text() + "\nCannot image preview: \"" + QString::fromStdString((delim_position != std::string::npos) ? filePath.substr(delim_position + 1) : "") + "\"\n");
                        }
                        size_t delim_position = filePath.rfind(delimiter);
                        ui->label_output->setText(ui->label_output->text() + "\nAdding for deletion: \"" + QString::fromStdString((delim_position != std::string::npos) ? filePath.substr(delim_position + 1) : "") + "\"\n");
                    }
                }
            }
        }
    }
    else {
        ui->label_output->setText(ui->label_output->text() + "\n" + "Error: Invalid path.");
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
            ui->label_output->setText(ui->label_output->text() + "\nDeleting file: \"" + QString::fromStdString((delim_position != std::string::npos) ? qPath.toStdString().substr(delim_position + 1) : "") + "\"\n");
        }
    }
    else {
        ui->label_output->setText(ui->label_output->text() + "Error: Vector is empty.");
    }
}