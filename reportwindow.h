#ifndef REPORTWINDOW_H
#define REPORTWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>

using namespace std;

namespace Ui {
class ReportWindow;
}

class ReportWindow : public QMainWindow
{
    Q_OBJECT

private:
    //Scene containing the corresponding report
    QGraphicsScene *clinicalScene;
    QGraphicsScene *simplifiedScene;

public:
    explicit ReportWindow(QWidget *parent = 0);
    ~ReportWindow();

    //public function to be called by the main window
    void render_report(char *absolutPath, char *studyName);

private slots:
    //Depending on the selected button, display the relevant report
    void display_clinical();
    void display_simplified();
    void display_two_report();

private:
    //creation of the window
    Ui::ReportWindow *ui;
    void add_buttons();

};

#endif // REPORTWINDOW_H
