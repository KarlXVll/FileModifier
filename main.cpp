#include <iostream>
#include <QtCore>
#include <QApplication>
#include <QWidget>
#include <QFormLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>


QString inputFilePath = "path/to/input/files";
QString outputFilePath = "path/to/output/files";
QString fileMask = "*.txt";
bool deleteInputFiles = false;
bool overwriteOutputFiles = true;
int timerInterval = 1000; // в миллисекундах
bool runOnce = false;
QString fileModificationMask = "modified_";


void modifyFile(const QString& filePath) {
    QFile inputFile(filePath);
    if (!inputFile.open(QIODevice::ReadOnly)) {
        std::cout << "Could not open input file: " << filePath.toStdString() << std::endl;
        return;
    }

    if (!inputFile.isReadable()) {
        std::cout << "Input file is not readable: " << filePath.toStdString() << std::endl;
        return;
    }

    QString fileName = QFileInfo(filePath).fileName();
    QString outputFileName = fileName;

    if (!overwriteOutputFiles) {
        int counter = 1;
        while (QFile(outputFilePath + "/" + outputFileName).exists()) {
            outputFileName = QString("%1%2.%3")
                                 .arg(fileModificationMask)
                                 .arg(QFileInfo(fileName).baseName())
                                 .arg(QFileInfo(fileName).suffix());
            outputFileName.insert(outputFileName.lastIndexOf('.'), QString::number(counter));
            counter++;
        }
    }

    QFile outputFile(outputFilePath + "/" + outputFileName);
    if (!outputFile.open(QIODevice::WriteOnly)) {
        std::cout << "Could not open output file: " << fileName.toStdString() << std::endl;
        return;
    }

    QByteArray fileData = inputFile.readAll();
    // Операция XOR с 8-байтной переменной
    QByteArray key = "abcdefgh";
    QByteArray modifiedData;
    for (int i = 0; i < fileData.size(); i++) {
        modifiedData.append(fileData[i] ^ key[i%8]);
    }

    outputFile.write(modifiedData);

    inputFile.close();
    outputFile.close();

    if (deleteInputFiles) {
        if (!QFile::remove(filePath)) {
            std::cout << "Could not delete input file: " << filePath.toStdString() << std::endl;
        }
    }

    std::cout << "File modified and saved: " << outputFileName.toStdString() << std::endl;
}

void checkForInputFiles() {
    QDir inputDir(inputFilePath);
    QFileInfoList files = inputDir.entryInfoList(QStringList(fileMask), QDir::Files);

    foreach(const QFileInfo& fileInfo, files) {
        QString filePath = fileInfo.absoluteFilePath();
        if (!QFile(filePath).isOpen()) {
            modifyFile(filePath);
        }
    }
}

void createUI()
{
    QWidget *window = new QWidget();
    QFormLayout *layout = new QFormLayout();

    QLineEdit *inputFilePathLineEdit = new QLineEdit();
    layout->addRow("Input File Path:", inputFilePathLineEdit);

    QLineEdit *outputFilePathLineEdit = new QLineEdit();
    layout->addRow("Output File Path:", outputFilePathLineEdit);

    QLineEdit *fileMaskLineEdit = new QLineEdit();
    layout->addRow("File Mask:", fileMaskLineEdit);

    QCheckBox *deleteInputFilesCheckBox = new QCheckBox();
    layout->addRow("Delete Input Files:", deleteInputFilesCheckBox);

    QCheckBox *overwriteOutputFilesCheckBox = new QCheckBox();
    layout->addRow("Overwrite Output Files:", overwriteOutputFilesCheckBox);

    QLineEdit *timerIntervalLineEdit = new QLineEdit();
    layout->addRow("Timer Interval (ms):", timerIntervalLineEdit);

    QCheckBox *runOnceCheckBox = new QCheckBox();
    layout->addRow("Run Once:", runOnceCheckBox);

    QLineEdit *fileModificationMaskLineEdit = new QLineEdit();
    layout->addRow("File Modification Mask:", fileModificationMaskLineEdit);

    QPushButton *startButton = new QPushButton("Start");
    layout->addWidget(startButton);

    QObject::connect(startButton, &QPushButton::clicked, [=]() {
        inputFilePath = inputFilePathLineEdit->text();
        outputFilePath = outputFilePathLineEdit->text();
        fileMask = fileMaskLineEdit->text();
        deleteInputFiles = deleteInputFilesCheckBox->isChecked();
        overwriteOutputFiles = overwriteOutputFilesCheckBox->isChecked();
        timerInterval = timerIntervalLineEdit->text().toInt();
        runOnce = runOnceCheckBox->isChecked();
        fileModificationMask = fileModificationMaskLineEdit->text();

        window->close();
    });

    window->setLayout(layout);
    window->show();
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, &checkForInputFiles);

    if (runOnce) {
        checkForInputFiles();
    } else {
        timer.start(timerInterval);
    }

    createUI();
    return app.exec();
}
