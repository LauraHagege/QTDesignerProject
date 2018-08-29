#include "dialog.h"
#include "ui_dialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <iostream>
#include <QDebug>

using namespace std;

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

    setWindowTitle( tr("Welcome!") );

    //displaying welcoming image
    welcomeImg = QPixmap("C:/Users/simms/Desktop/Laura/img/disclaimer.jpg").scaled(QSize(570,570));
    welcomingScene = new QGraphicsScene(this);
    welcomingScene->addPixmap(welcomeImg);
    ui->welcomeView->setScene(welcomingScene);
    ui->welcomeView->show();
    ui->WrongFolder->setVisible(false);
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::on_pushButton_clicked()
{
    QString Filepath = QFileDialog::getExistingDirectory(this);

    QByteArray ba = Filepath.toLatin1();
    strcpy(studyPath,ba.data());

    int size =(int) strlen(studyPath);

    if(studyPath[size-8] =='S' && studyPath[size-7] =='D' && studyPath[size-6] =='Y')
        this->close();
    else{
        ui->WrongFolder->setVisible(true);
        this->open();
    }

    char store[11];

    int count =0;
    for(int i=size-5; i<size; i++){
        store[count]=studyPath[i];
        count ++;
    }
    store[count]='\0';

    studyNumber=atoi(store)+1;

    strcpy(studyName,string("SDY").c_str());
    strcat(studyName,store);


    //Storing DICOMDIR path
    //DICOMDIR allows to go through all the arborescence of the file
    for(int i=0; i<size-17; i++){
        dicomdirpath[i]=studyPath[i];
    }
    dicomdirpath[size-17]='\0';

    strcat(dicomdirpath,string("DICOMDIR").c_str());





}

char *Dialog::getStudyName(){
    return studyName;
}

char *Dialog::getDicomDirPath(){
    return dicomdirpath;
}

char * Dialog::getStudyPath(){
    return studyPath;
}

int Dialog::getStudyNumber(){
    return studyNumber;
}

