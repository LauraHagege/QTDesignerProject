#include "mainwindow.h"
#include <QApplication>
#include "dialog.h"
#include <QMessageBox>
#include <iostream>
#include "reportwindow.h"
//#include "dialogpinaccess.h"

using namespace std;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // dialog box to select the DICOMDIR file
    Dialog dialogbox;
    dialogbox.setModal(true);
    dialogbox.exec();

   // DialogPinAccess test;
    //Get the selected path
    //QString path=dialogbox.getFilepath();


    MainWindow w;

    //create window architecture
    // find images according to the given path
    w.checkPin(dialogbox.getStudyPath(),dialogbox.getStudyNumber(),dialogbox.getStudyName(),dialogbox.getDicomDirPath());


    //
    w.showMaximized();



    if(w.getAccess())
        return a.exec();
    else
        a.closeAllWindows();
}
