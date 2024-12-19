#include "MainWindow.h"
#include "ui_MainWIndow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    connect(ui->button_start, &QPushButton::clicked, this, &MainWindow::processFolder);

    ui->label_output->clear();
    ui->lineEdit->clear();
}

MainWindow::~MainWindow() {
    delete ui;
}

bool MainWindow::checkCoords(const std::string& filePath) {

    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        char delimiter = '\\';
        size_t delim_position = filePath.rfind(delimiter);
        ui->label_output->setText(ui->label_output->text() + "\nError: Could not open file \"" + QString::fromStdString((delim_position != std::string::npos) ? filePath.substr(delim_position + 1) : "") + "\"");
        return false;
    }

    std::vector<unsigned char> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    TinyEXIF::EXIFInfo exif(data.data(), data.size());
    if (!exif.Fields) {
        char delimiter = '\\';
        size_t delim_position = filePath.rfind(delimiter);
        ui->label_output->setText(ui->label_output->text() + "\nNo EXIF data found in file \"" + QString::fromStdString((delim_position != std::string::npos) ? filePath.substr(delim_position + 1) : "") + "\"");
        return true;
    }

    if (exif.GeoLocation.hasLatLon()) {
        if (exif.GeoLocation.Latitude == 0.0 && exif.GeoLocation.Longitude == 0.0) {
            char delimiter = '\\';
            size_t delim_position = filePath.rfind(delimiter);
            ui->label_output->setText(ui->label_output->text() + "\nInvalid GPS data found in: \"" + QString::fromStdString((delim_position != std::string::npos) ? filePath.substr(delim_position + 1) : "") + "\"");
            return true;
        }
        //ui->label_output->setText(ui->label_output->text() + "\nValid GPS data found in: " + QString::fromStdString(filePath));
        return false;
    }

    char delimiter = '\\';
    size_t delim_position = filePath.rfind(delimiter);
    ui->label_output->setText(ui->label_output->text() + "\nNo GPS data found in: \"" + QString::fromStdString((delim_position != std::string::npos) ? filePath.substr(delim_position + 1) : "") + "\"");
    return true;
}

void MainWindow::processFolder() {
    QString qstr = ui->lineEdit->text();
    std::string folderPath = qstr.toStdString();
    if(std::filesystem::exists(folderPath)) {
        for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
            if (entry.is_regular_file()) {
                const auto& filePath = entry.path().string();

                std::string extension = entry.path().extension().string();
                if (extension == ".jpg" || extension == ".jpeg" || extension == ".png") {
                    if (checkCoords(filePath)) {
                        // Delete file if GPS data is invalid
                        char delimiter = '\\';
                        size_t delim_position = filePath.rfind(delimiter);
                        ui->label_output->setText(ui->label_output->text() + "\nDeleting file: \"" + QString::fromStdString((delim_position != std::string::npos) ? filePath.substr(delim_position + 1) : "") + "\"\n");
                        std::filesystem::remove(filePath);
                    }
                }
            }
        }
        ui->label_output->setText(ui->label_output->text() + "\n\nDone!");
    }
    else {
        if(ui->lineEdit->text().isEmpty()) {
            ui->label_output->setText(ui->label_output->text() + "\nPath cannot be empty.");
        }
        else {
            ui->label_output->setText(ui->label_output->text() + "\n" + R"(Error: Invalid path. Path must be an absolute path starting from root (e.g. "C:\users\pictures"))");
        }
    }
}