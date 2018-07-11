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
#include <contrast.h>
#include <QMap>
#include <QRgb>
#include <QColor>
#include <stdlib.h>

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
    index=0;
    series=0;
    //QMessageBox::information(this,tr("Directory path"), path);
    QByteArray ba = path.toLatin1();
    const char *file = ba.data();
    int size = strlen(file);
    cout << "char size " << size << endl;
    char *filepath=new char[130];
    char *filepath2=new char[130];
    for(int i=0; i<size-8; i++){
        filepath[i]=file[i];
        filepath2[i]=file[i];
    }
    filepath[size-8]='\0';
    //filepath2[size-8]='\0';
    cout << "path " << filepath <<endl;

    char* path_to_file=new char[130];;



    DcmDicomDir myDir(file);

   // DcmFileFormat myFile = myDir ->getDirFileFormat(); //&fileformat ?
    DcmDirectoryRecord *   root = &(myDir.getRootRecord());
    DcmDirectoryRecord *   PatientRecord = NULL;
    DcmDirectoryRecord *   StudyRecord = NULL;
    DcmDirectoryRecord *   SeriesRecord = NULL;
    DcmDirectoryRecord *   FileRecord = NULL;
    OFString            tmpString;

    int count =0;

    vector<DicomImage*> DicomImages;


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
                            vector<QImage*> CurrentSerie;
                            char serieName[30]="serie";
                            series +=1;
                            strcat(serieName,to_string(series).c_str());

                            /*
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

                            cout << endl;*/


                            while((FileRecord = SeriesRecord->nextSub(FileRecord)) != NULL){
                                /*if(FileRecord->findAndGetOFString(DCM_ImageType,tmpString).good()){
                                     cout <<"Img description: " <<tmpString.c_str() << endl ;
                                }
                                if(FileRecord->findAndGetOFStringArray(DCM_ReferencedFileID,tmpString).good()){
                                     cout <<"File path: " <<tmpString.c_str() << endl ;
                                }*/

                                if (series == 2){
                                    if(FileRecord->findAndGetOFStringArray(DCM_ReferencedFileID,tmpString).good()){
                                        strcpy(path_to_file,filepath);
                                        strcat(path_to_file,tmpString.c_str());
                                    }
                                    count +=1;
                                    //cout << "full path " << path_to_file << endl;

                                    DicomImages.push_back(new DicomImage(path_to_file));
                               }

                            }
                        }
                    }
                }
            }

    cout << "series numbers: " << series << endl;
    //DicomImage *image = new DicomImage(filepath);
    //DicomImage *image2 = new DicomImage(filepath2);
    //DicomImages.push_back(new DicomImage(path_to_file));
    //DicomImages.push_back(new DicomImage(path_to_file));
    vector <Uint8*> pixelDatas;

    int check=true;
    for(int i=0; i<DicomImages.size(); i++){
        if(DicomImages[i] == NULL)
            check=false;
        else if(!(DicomImages[i]->getStatus() == EIS_Normal))
            check=false;
    }


    if (check){
        //if (DicomImages[0]->getStatus() == EIS_Normal && DicomImages[1]->getStatus() == EIS_Normal)
        for(int i=0; i<DicomImages.size(); i++){
            DicomImages[0] ->setMinMaxWindow();
            Uint8 *pixelData = (Uint8 *)(DicomImages[i]->getOutputData(8 )); // bits per sample
            // Uint8 is a pointer to internal memory buffer

            // DicomImages[1] ->setMinMaxWindow();
            //Uint8 *pixelData2 = (Uint8 *)(DicomImages[1]->getOutputData(8 )); // bits per sample


            if (pixelData != NULL){
                // do something useful with the pixel data
                Images.push_back(new QImage (pixelData,DicomImages[0]->getWidth(), DicomImages[0]->getHeight(), QImage::Format_Indexed8));
                for( int j = 0; j < 256; ++j )
                    Images[i]->setColor(j, qRgb(j,j,j));

                //currentImage = img;

                // Images.push_back(new QImage (pixelData2,DicomImages[1]->getWidth(), DicomImages[1]->getHeight(), QImage::Format_Indexed8));
                //for( int i = 0; i < 256; ++i )
                  //  Images[1]->setColor(i, qRgb(i,i,i));

                /*Images.push_back(new QImage (pixelData2,DicomImages[1]->getWidth(), DicomImages[1]->getHeight(), QImage::Format_Indexed8));
                for( int i = 0; i < 256; ++i )
                    Images[2]->setColor(i, qRgb(i,i,i));
                //secondImage = img2;
*/

                // //////////////////////////////////////////////////////
                /*
                graphic = new QGraphicsScene( this );
                    graphic->addPixmap( QPixmap::fromImage( *img ) );
                    ui->graphicsView->setScene(graphic);
               */
                 // //////////////////////////////////////////////////////

               /* myScene= new QGraphicsScene(this);
                ui->graphicsView->setScene(myScene);
                myScene->addPixmap( QPixmap::fromImage( *Images[1] ) );*/
                // //////////////////////////////////////////////////////


                }
            }
    } else
            cerr << "Error: cannot load DICOM image (" << DicomImage::getString(DicomImages[0]->getStatus()) << ")" << endl;


    //creating scene
    myScene= new QGraphicsScene(this);
    ui->graphicsView->setScene(myScene);
    myScene->addPixmap( QPixmap::fromImage( *Images[1] ) );

    delete filepath;
}



void MainWindow::on_pushButton_clicked()
{
    for(int j=0; j<Images.size(); j++){
        for( int i = 0; i < 256; ++i )
            Images[j]->setColor(255-i, qRgb(i,i,i));

    }



    myScene->addPixmap( QPixmap::fromImage( *Images[index] ) );
    ui->graphicsView->setScene(myScene);
}

void MainWindow::wheelEvent(QWheelEvent *event)
{
   /* for( int i = 0; i < 256; ++i )
        currentImage->setColor(255-i, qRgb(i,i,i));
*/
    //myLabel->setPixmap( QPixmap::fromImage( *currentImage ) );
    cout << "index" << index << endl;
    cout << "size" << Images.size() <<endl;

    index+=1;
    if(index+1 > Images.size())
        index=0;

    cout << "index after " << index << endl;

    myScene->addPixmap( QPixmap::fromImage( *Images[index] ) );
    ui->graphicsView->setScene(myScene);
}
