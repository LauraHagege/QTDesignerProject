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
#include <QButtonGroup>

using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    buttonGroup = new QButtonGroup(this);
    connect(buttonGroup, SIGNAL(buttonClicked(QAbstractButton*)), SLOT(buttonInGroupClicked(QAbstractButton*)));

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

    //char* path_to_file=new char[130];;



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
    //currentSerie=new char[30];
    //strcpy(currentSerie,"serie2");
    currentSerie = "serie2";




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
                            //vector to store paths to files
                            vector<string> paths;

                            //vector to store images of the current selected serie
                            vector<QImage*> CurrentSerie;

                            //adding button for each serie
                            char serieName[30]="serie";
                            series +=1;
                            strcat(serieName,to_string(series).c_str());

                            QPushButton *button = new QPushButton(this);
                            seriesButtons.push_back(button);
                            button->setText(tr(serieName));
                            button->setVisible(true);
                            buttonGroup->addButton(button);


                            ui->verticalLayout_2->addWidget(button);


                            //connect(button,SIGNAL(clicked()),this,SLOT(updateSerie(QPushButton*)));







                           // int counter=0;
                            while((FileRecord = SeriesRecord->nextSub(FileRecord)) != NULL){
                                /*if(FileRecord->findAndGetOFString(DCM_ImageType,tmpString).good()){
                                     cout <<"Img description: " <<tmpString.c_str() << endl ;
                                }
                               */
                               //if (series == 2){
                                   string fullpath;
                                    if(FileRecord->findAndGetOFStringArray(DCM_ReferencedFileID,tmpString).good()){
                                        //strcpy(path_to_file,filepath);
                                        //strcat(path_to_file,tmpString.c_str());
                                        fullpath = filepath + string(tmpString.c_str());
                                        //cout << "jpp" << fullpath << endl;

                                    }
                                    count +=1;
                                    //cout << "full path " << path_to_file << endl;
                                    paths.push_back(fullpath);
                                    //cout  << "jpp " << paths[counter] << endl;
                                   // counter +=1;

                                    //DicomImages.push_back(new DicomImage(path_to_file));
                              // }
                            }

                            cout <<" path size" << paths.size() << endl;

                            /*cout << "comp " << strcmp(currentSerie,serieName) << endl;
                            cout << "path0 " << paths[0]<< endl;*/


                          //if (series == 2){
                               //cout << "test path"<<paths[0] <<endl;
                              //cout << "test path"<<paths[1] <<endl;
                               allPath.insert(pair<string,vector<string>>("serie"+to_string(series),paths));

                               /*char currentserieName[30]="serie";
                               cout <<"serie name " << serieName << endl;
                               cout << "TESTTESTESTESTEST " << allPath[serieName][0] << endl;
                               strcpy(currentserieName,serieName);
                               cout <<"current serie name " << currentserieName << endl;
                               cout << "TESTTESTESTESTEST " << allPath[currentserieName][0] << endl;*/
                          // }
                        }
                    }
                }
            }

    //string buttonName = seriesButtons[1]->text().toLocal8Bit().constData();
    //cout << "size with button name "<<allPath[buttonName].size() << endl;

    cout << "sizeee " << allPath["serie2"].size() << endl;
    cout << "size of currentSerie " << allPath[currentSerie].size() << endl;

    cout << "path " << allPath[currentSerie][0] << endl;
    cout << "path " << allPath[currentSerie][1] << endl;

    for(int i=0; i<allPath[currentSerie].size(); i++){
        //cout << "path " << allPath[currentSerie][i] << endl;
        DicomImages.push_back(new DicomImage(allPath[currentSerie][i].c_str()));


    }




    //**********DEALING WITH ALL IMAGES OF THE SERIE**********//
    //see later not to open everything
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

            if (pixelData != NULL){
                // do something useful with the pixel data
                Images.push_back(new QImage (pixelData,DicomImages[0]->getWidth(), DicomImages[0]->getHeight(), QImage::Format_Indexed8));
                for( int j = 0; j < 256; ++j )
                    Images[i]->setColor(j, qRgb(j,j,j));
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
   // cout << "index" << index << endl;
   // cout << "size" << Images.size() <<endl;

    index+=1;
    if(index+1 > Images.size())
        index=0;

   // cout << "index after " << index << endl;

    myScene->addPixmap( QPixmap::fromImage( *Images[index] ) );
    ui->graphicsView->setScene(myScene);
}

void MainWindow::buttonInGroupClicked(QAbstractButton *b){
    string buttonName = b->text().toLocal8Bit().constData();
    cout << "button " << buttonName << " clicked, associated serie size: "<< this->allPath[buttonName].size() << endl;
}

