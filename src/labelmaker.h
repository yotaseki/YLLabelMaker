#ifndef LABELMAKER_H
#define LABELMAKER_H

#include <QWidget>
#include <fstream>
#include <QGraphicsObject>
#include <QGraphicsItem>
#include <QSettings>
#include "MyQclass.h"
#include "ui_dirdialog.h"

namespace Ui {
class LabelMaker;
class DirDialog;
}

class LabelMaker : public QWidget
{
    Q_OBJECT

public:
    explicit LabelMaker(QWidget *parent = 0);
    ~LabelMaker();
    struct Cursur
    {
        int x;
        int y;
        Qt::MouseButton b;
        int flag;
    };
    Cursur c_view;
    Cursur c_rect;
    struct Bbox
    {
        int label;
        float x;
        float y;
        float w;
        float h;
    };
    std::vector<Bbox> bboxes;

private slots:
    void onMouseMovedGraphicsView(int x,int y,Qt::MouseButton b);
    void onMousePressedGraphicsView(int x,int y,Qt::MouseButton b);
    void onMouseReleasedGraphicsView(int x,int y,Qt::MouseButton b);
    void resizeGraphicsView();
    void onPushNext();
    void onPushBack();
    void onPushChooseDirectory();
    void onPushChooseImagesDir();
    void onPushChooseSaveTo();
    void destroyDirDialog();
    void onPushPlus();
    void onPushMinus();

private:
    int img_index;
    int viewoffset;
    Ui::LabelMaker *ui;
    Ui::DirDialog *d_ui;
    QDialog dialog;
    QFileInfoList img_list;
    MyQclass myq;
    QGraphicsScene scene;
    cv::Mat currentimg;
    QSettings key;
    void connectSignals();
    int updateView();
    int setImage(cv::Mat src);
    int drawCursur();
    int drawRect();
    int drawBbox();
    void correctCoordiante(int &x, int &y);
    void loadImage();
    void readKey();
    void saveKey();
    void writeText();
    void readText();
    void appendBbox(int label,int x1, int y1, int x2, int y2);
    void changeIndex(int num);
    QFileInfoList makeImageList(QString path);
};

#endif // LABELMAKER_H
