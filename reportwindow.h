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
    QGraphicsScene *clinicalScene;
    QGraphicsScene *simplifiedScene;

public:
    explicit ReportWindow(QWidget *parent = 0);
    ~ReportWindow();
    void render_report(char *absolutPath, char *studyName);

private slots:
    void display_clinical();
    void display_simplified();
    void display_two_report();

private:
    Ui::ReportWindow *ui;
    void add_buttons();

};

#endif // REPORTWINDOW_H
