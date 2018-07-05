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

QGrayColorTable::QGrayColorTable()
{
        //color table for monochroom images.
        for(unsigned int i=0; i < GRAY_LEVELS; i++) {
                m_colortable[i] = qRgb(i,i,i);
        }
}

QRgb QGrayColorTable::m_colortable[GRAY_LEVELS];





void MainWindow::setFilepath(QString path){
    //QMessageBox::information(this,tr("Directory path"), path);
    QByteArray ba = path.toLatin1();
    const char *file = ba.data();
    int size = strlen(file);
    cout << "char size " << size << endl;
    char *filepath=new char[130];
    for(int i=0; i<size-8; i++){
        filepath[i]=file[i];
    }
    filepath[size-8]='\0';
    cout << "path " << filepath <<endl;

    DcmDicomDir myDir(file);

   // DcmFileFormat myFile = myDir ->getDirFileFormat(); //&fileformat ?
    DcmDirectoryRecord *   root = &(myDir.getRootRecord());
    DcmDirectoryRecord *   PatientRecord = NULL;
    DcmDirectoryRecord *   StudyRecord = NULL;
    DcmDirectoryRecord *   SeriesRecord = NULL;
    DcmDirectoryRecord *   FileRecord = NULL;
    OFString            tmpString;

    int count =0;

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
                            if (SeriesRecord->findAndGetOFString(DCM_SeriesInstanceUID, tmpString).good()) {
                                cout <<"Serie UID: " <<tmpString.c_str() << endl ;
                            }
                            if (SeriesRecord->findAndGetOFString(DCM_SeriesNumber, tmpString).good()) {
                                cout <<"Serie number: " <<tmpString.c_str() << endl ;
                            }
                            if (SeriesRecord->findAndGetOFString(DCM_SeriesType, tmpString).good()) {
                                cout <<"Serie type: " <<tmpString.c_str() << endl ;
                            }
                            if (SeriesRecord->findAndGetOFString(DCM_SeriesDescriptionCodeSequence, tmpString).good()) {
                                cout <<"Serie description code sequence: " <<tmpString.c_str() << endl ;
                            }

                            cout << endl;

                            while((FileRecord = SeriesRecord->nextSub(FileRecord)) != NULL){
                                /*if(FileRecord->findAndGetOFString(DCM_ImageType,tmpString).good()){
                                     cout <<"Img description: " <<tmpString.c_str() << endl ;
                                }
                                if(FileRecord->findAndGetOFStringArray(DCM_ReferencedFileID,tmpString).good()){
                                     cout <<"File path: " <<tmpString.c_str() << endl ;
                                }*/
                                if (count == 0){
                                    if(FileRecord->findAndGetOFStringArray(DCM_ReferencedFileID,tmpString).good()){
                                         int followingsize = strlen(tmpString.c_str());
                                         for(int i=0; i<=followingsize ; i ++){
                                             if(tmpString.c_str()[i]=='\\'){
                                                     filepath[size-8+i]='/';

                                         }
                                             else
                                                     filepath[size-8+i]=tmpString.c_str()[i];

                                         }
                                    }
                                    count +=1;
                                    cout << "full path " << filepath << endl;
                                }
                            }
                        }
                    }
                }
            }


    DicomImage *image = new DicomImage(filepath);

    if (image != NULL)
        {
          if (image->getStatus() == EIS_Normal)
          {
            image ->setMinMaxWindow();
            Uint8 *pixelData = (Uint8 *)(image->getOutputData(8 )); // bits per sample
            // Uint8 is a pointer to internal memory buffer
            if (pixelData != NULL)
            {
                // do something useful with the pixel data
                QImage img(pixelData,image->getWidth(), image->getHeight(), QImage::Format_Indexed8);
                for( int i = 0; i < 256; ++i )
                    img.setColor(i, qRgb(i,i,i));


                QGraphicsScene * graphic = new QGraphicsScene( this );

                    graphic->addPixmap( QPixmap::fromImage( img ) );

                    ui->graphicsView->setScene(graphic);
            }
          } else
            cerr << "Error: cannot load DICOM image (" << DicomImage::getString(image->getStatus()) << ")" << endl;
        }
    delete filepath;
}
