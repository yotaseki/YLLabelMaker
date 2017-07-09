#ifndef LABELMAKER_H
#define LABELMAKER_H

#include <QWidget>
#include <fstream>
#include <QGraphicsObject>
#include <QGraphicsItem>
#include <QSettings>
#include <MyQclass.h>
#include <ui_dirdialog.h>
#include <QEvent>

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
	void textChangedLinePage();

private:
    int img_index;
    int viewoffset;
    int scene_img_w;
    int scene_img_h;
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
    void searchBboxByMI(int mx, int my, int *out_x1, int *out_y1, int *out_x2, int *out_y2);
    void convertAxsisGraphics2CurrentImage(int mx, int my, int *out_mx, int *out_my );
    double calc_mi(const QImage &img, const QImage &maskimg, int x0, int y0);
    QImage CreateMask();
    QFileInfoList makeImageList(QString path);
protected:
	bool eventFilter(QObject *widget, QEvent *event);
};

#endif // LABELMAKER_H

