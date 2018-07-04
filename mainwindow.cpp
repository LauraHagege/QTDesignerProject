#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dialog.h"
#include <QLabel>
#include <QMessageBox>
#include <iostream>

#include <QGuiApplication>
#include <QApplication>
#include <iostream>
#undef UNICODE
#undef _UNICODE
#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dctk.h>
#include <dcmtk/dcmimgle/dcmimage.h>
#include <QPixmap>
#include <QLabel>
#include <QVBoxLayout>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsView>

using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setFilepath(QString path){
    QMessageBox::information(this,tr("Directory path"), path);
    QByteArray ba = path.toLatin1();
    const char *file = ba.data();


    DcmDicomDir myDir(file);

   // DcmFileFormat myFile = myDir ->getDirFileFormat(); //&fileformat ?
    DcmDirectoryRecord *   root = &(myDir.getRootRecord());
    DcmDirectoryRecord *   PatientRecord = NULL;
    DcmDirectoryRecord *   StudyRecord = NULL;
    DcmDirectoryRecord *   SeriesRecord = NULL;
    DcmDirectoryRecord *   FileRecord = NULL;
    OFString            tmpString;


    if(root != NULL)
            {
                while (((PatientRecord = root->nextSub(PatientRecord)) != NULL))
                {
                    if (PatientRecord->findAndGetOFString(DCM_PatientName, tmpString).good()) {
                        cout << "PatientName: " << tmpString.c_str() << endl ;
                    }
                    while (((StudyRecord = PatientRecord->nextSub(StudyRecord)) != NULL))
                    {
                        if (StudyRecord->findAndGetOFString(DCM_StudyDescription, tmpString).good()) {
                            cout <<"Study Description: " << tmpString.c_str() << endl ;
                        }
                        while (((SeriesRecord = StudyRecord->nextSub(SeriesRecord)) != NULL))
                        {
                            if (SeriesRecord->findAndGetOFString(DCM_SeriesDescription, tmpString).good()) {
                                cout <<"Serie description: " <<tmpString.c_str() << endl ;
                            }
                        }
                    }
                }
            }

    /*DicomImage *image = new DicomImage(file);

    if (image != NULL)
        {
          if (image->getStatus() == EIS_Normal)
          {
            Uint8 *pixelData = (Uint8 *)(image->getOutputData(8 )); // bits per sample
            // Uint8 is a pointer to internal memory buffer
            if (pixelData != NULL)
            {
                // do something useful with the pixel data
                QImage img(pixelData,image->getWidth(), image->getHeight(), QImage::Format_Indexed8 );
                    QGraphicsScene * graphic = new QGraphicsScene( this );

                    graphic->addPixmap( QPixmap::fromImage( img ) );

                    ui->graphicsView->setScene(graphic);
            }
          } else
            cerr << "Error: cannot load DICOM image (" << DicomImage::getString(image->getStatus()) << ")" << endl;
        }*/

}
