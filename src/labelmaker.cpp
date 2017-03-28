#include "labelmaker.h"
#include <ui_labelmaker.h>
#include <ui_dirdialog.h>
using namespace std;

LabelMaker::LabelMaker(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LabelMaker),
    d_ui(new Ui::DirDialog),
    img_index(0),
    viewoffset(80),
    key(QDir::homePath()+"/.labelmaker.ini",QSettings::IniFormat)
{
    ui->setupUi(this);
    d_ui->setupUi(&dialog);
    dialog.setWindowFlags(Qt::WindowStaysOnTopHint);
    connectSignals();
    ui->graphicsView->setScene(&scene);
    readKey();
    dialog.show();
}

void LabelMaker::readKey()
{
    int index = key.value("INDEX").toInt();
    QString imagedir = key.value("IMAGESDIR").toString();
    QString savedir = key.value("SAVEDIR").toString();
    if(index)
    {
        img_index = index;
    }
    if(imagedir.size())
    {
        d_ui->lineImageDir->setText(imagedir);
    }
    if(savedir.size())
    {
        d_ui->lineSaveTo->setText(savedir);
    }
}

void LabelMaker::saveKey()
{
    key.setValue("IMAGESDIR",d_ui->lineImageDir->text());
    key.setValue("SAVEDIR",d_ui->lineSaveTo->text());
    key.setValue("INDEX",img_index);
}

LabelMaker::~LabelMaker()
{
    writeText();
    saveKey();
    delete ui;
    delete d_ui;
}

void LabelMaker::onMouseMovedGraphicsView(int x, int y, Qt::MouseButton b)
{
    int offset = viewoffset;
    int w = ui->graphicsView->width()-offset*2;
    int h = ui->graphicsView->height()-offset*2;
    float xf,yf;
    x = x-offset;
    y = y-offset;
    correctCoordiante(x,y);
    c_view.x = x;
    c_view.y = y;
    c_view.b = b;
    xf = (float)x/w;
    yf = (float)y/h;
    //ui->labelMousePos->setText(QString("x: %1, y: %2").arg(xf).arg(yf));
    updateView();
}

void LabelMaker::onMousePressedGraphicsView(int mx, int my, Qt::MouseButton b)
{
    if(b == Qt::LeftButton)
    {
        int offset = viewoffset;
        mx = mx-offset;
        my = my-offset;
        correctCoordiante(mx,my);
        c_rect.x = mx;
        c_rect.y = my;
        c_rect.b = b;
        c_rect.flag = 1;
        updateView();
    }
    if(b == Qt::RightButton)
    {
        if(bboxes.size())
        {
            bboxes.pop_back();
        }
    }
}

void LabelMaker::onMouseReleasedGraphicsView(int mx, int my, Qt::MouseButton b)
{
    if(b == Qt::LeftButton)
    {
    appendBbox(ui->spinLabelNumber->value(),c_rect.x,c_rect.y,c_view.x,c_view.y);
    c_rect.flag = 0;
    updateView();
    }
    else if(b == Qt::RightButton)
    {
        updateView();
    }
}

void LabelMaker::resizeGraphicsView()
{
    updateView();
}

void LabelMaker::onPushNext()
{
    changeIndex(1);
}

void LabelMaker::onPushBack()
{
    changeIndex(-1);
}

void LabelMaker::onPushChooseDirectory()
{
    dialog.show();
}

void LabelMaker::onPushChooseImagesDir()
{
    img_index = 0;
    QDir dir = myq.selectDir(QDir(d_ui->lineImageDir->text()));
    d_ui->lineImageDir->setText(dir.path());
    d_ui->lineSaveTo->setText(dir.path()+"_labels");
}

void LabelMaker::onPushChooseSaveTo()
{
    QDir dir = myq.selectDir(QDir(d_ui->lineImageDir->text()));
    d_ui->lineSaveTo->setText(dir.path());
}

void LabelMaker::destroyDirDialog()
{
    img_list.clear();
    img_list = makeImageList(d_ui->lineImageDir->text());

    loadImage();
    updateView();
}

void LabelMaker::onPushPlus()
{
    ui->spinLabelNumber->setValue(ui->spinLabelNumber->value()+1);
}

void LabelMaker::onPushMinus()
{
    ui->spinLabelNumber->setValue(ui->spinLabelNumber->value()-1);
}

void LabelMaker::connectSignals()
{
    bool ret;
    ret = connect(ui->graphicsView,SIGNAL(mouseMoved(int,int,Qt::MouseButton)),this,SLOT(onMouseMovedGraphicsView(int,int,Qt::MouseButton)));
    assert(ret);
    ret = connect(ui->graphicsView,SIGNAL(mousePressed(int,int,Qt::MouseButton)),this,SLOT(onMousePressedGraphicsView(int,int,Qt::MouseButton)));
    assert(ret);
    ret = connect(ui->graphicsView,SIGNAL(mouseReleased(int,int,Qt::MouseButton)),this,SLOT(onMouseReleasedGraphicsView(int,int,Qt::MouseButton)));
    assert(ret);
    ret = connect(ui->graphicsView,SIGNAL(resized()),this, SLOT(resizeGraphicsView()));
    assert(ret);
    ret = connect(ui->pushNext,SIGNAL(clicked()),this,SLOT(onPushNext()));
    assert(ret);
    ret = connect(ui->pushBack,SIGNAL(clicked()),this,SLOT(onPushBack()));
    assert(ret);
    ret = connect(ui->pushChooseDirectory,SIGNAL(clicked()),this,SLOT(onPushChooseDirectory()));
    assert(ret);
    ret = connect(d_ui->pushChooseImageDir,SIGNAL(clicked()),this,SLOT(onPushChooseImagesDir()));
    assert(ret);
    ret = connect(d_ui->pushChooseSaveto,SIGNAL(clicked()),this,SLOT(onPushChooseSaveTo()));
    assert(ret);
    ret = connect(d_ui->pushOK,SIGNAL(clicked()),&dialog,SLOT(close()));
    assert(ret);
    ret = connect(&dialog,SIGNAL(finished(int)),this,SLOT(destroyDirDialog()));
    assert(ret);
    ret = connect(ui->pushPlus,SIGNAL(clicked()),this,SLOT(onPushPlus()));
    assert(ret);
    ret = connect(ui->pushMinus,SIGNAL(clicked()),this,SLOT(onPushMinus()));
    assert(ret);

}

int LabelMaker::updateView()
{
    scene.clear();
    if(img_list.size() == 0)
    {
        return -1;
    }
    if(currentimg.empty())
    {
        qDebug() << "img empty.";
        return -1;
    }
    setImage(currentimg);
    drawCursur();
    drawBbox();
    if(c_rect.flag == 1)
    {
        drawRect();
    }
    ui->labelFilename->setText(QString("%1 (%2/%3)")
                               .arg(img_list[img_index].fileName())
                               .arg(img_index+1)
                               .arg(img_list.size())
                               );
    return 0;
}

int LabelMaker::setImage(cv::Mat img)
{
    QPixmap pix;
    int offset = viewoffset*2;
    int w = ui->graphicsView->width()-offset;
    int h = ui->graphicsView->height()-offset;
    if(w<=0 || h<=0)
    {
        return -1;
    }
    cv::resize(img,img,cv::Size(w,h));
    pix = myq.MatBGR2pixmap(img);
    scene.addPixmap(pix);
    return 0;
}

int LabelMaker::drawCursur()
{
    scene.addEllipse(c_view.x,c_view.y,2,2,QPen(QColor(Qt::red)));
    return 0;
}

int LabelMaker::drawRect()
{
    scene.addRect(QRect(QPoint(c_rect.x,c_rect.y),QPoint(c_view.x,c_view.y)),QPen(myq.retColor(ui->spinLabelNumber->value()),3));
    return 0;
}

int LabelMaker::drawBbox()
{
    int offset = viewoffset;
    int w = ui->graphicsView->width()-offset*2;
    int h = ui->graphicsView->height()-offset*2;
    for(int i=0;i<bboxes.size();i++)
    {
        int x1 = w * (bboxes[i].x - (bboxes[i].w/2));
        int y1 = h * (bboxes[i].y - (bboxes[i].h/2));
        int x2 = w * (bboxes[i].x + (bboxes[i].w/2));
        int y2 = h * (bboxes[i].y + (bboxes[i].h/2));
        scene.addRect(QRect(QPoint(x1,y1),QPoint(x2,y2)),QPen(QBrush(myq.retColor(bboxes[i].label)),3));
    }
    return 0;
}

void LabelMaker::loadImage()
{
    currentimg = cv::imread(img_list[img_index].filePath().toStdString());
}

void LabelMaker::correctCoordiante(int &x, int &y)
{
    int offset = viewoffset*2;
    int w = ui->graphicsView->width()-offset;
    int h = ui->graphicsView->height()-offset;
    x = (x > 0) ?x:0;
    y = (y > 0) ?y:0;
    x = (x < w-1) ?x:w-1;
    y = (y < h-1) ?y:h-1;
}

QFileInfoList LabelMaker::makeImageList(QString path)
{
    QFileInfoList list,l_png,l_jpg;
    l_png = myq.scanFiles(path,"*png");
    l_jpg = myq.scanFiles(path,"*jpg");
    for(int i=0;i<l_png.size();i++)
    {
        list.append(l_png[i]);
    }
    for(int i=0;i<l_jpg.size();i++)
    {
        list.append(l_jpg[i]);
    }
    return list;
}

void LabelMaker::writeText()
{
    if(bboxes.size())
    {
        QString save_dir = d_ui->lineSaveTo->text();
        QDir dir(save_dir);
        QString fn = QString(dir.path()+"/"+img_list[img_index].fileName());
        fn.chop(4);
        fn = fn + ".txt";
        if (!dir.exists()){
            dir.mkpath(save_dir);
        }
        ofstream ofs;
        ofs.open(fn.toLocal8Bit());
        for(int i=0;i<bboxes.size();i++)
        {
            QString line = QString("%1 %2 %3 %4 %5").arg(bboxes[i].label).arg(bboxes[i].x).arg(bboxes[i].y).arg(bboxes[i].w).arg(bboxes[i].h);
            ofs << line.toStdString() << std::endl;
        }
        ofs.close();
    }
    bboxes.clear();
}

void LabelMaker::readText()
{
    vector<string> lines;
    QString save_dir = d_ui->lineSaveTo->text();
    QDir dir(save_dir);
    QString fn = QString(dir.path()+"/"+img_list[img_index].fileName());
    fn.chop(4);
    fn = fn + ".txt";
    QFileInfo finfo(fn);
    if(finfo.exists())
    {
        ifstream ifs;
        ifs.open(fn.toLocal8Bit());
        string l;
        while(getline(ifs,l))lines.push_back(l);
    }
    for(int i=0;i<lines.size();i++)
    {
        QString qstr = QString::fromStdString(lines[i]);
        QStringList ql = qstr.split(" ");
        Bbox box;
        box.label = ql[0].toInt();
        box.x = ql[1].toFloat();
        box.y = ql[2].toFloat();
        box.w = ql[3].toFloat();
        box.h = ql[4].toFloat();
        bboxes.push_back(box);
    }
}

void LabelMaker::appendBbox(int label, int x1, int y1, int x2, int y2)
{
    int offset = viewoffset;
    int width = ui->graphicsView->width()-offset*2;
    int height = ui->graphicsView->height()-offset*2;
    Bbox bbox;
    int x = (x1+x2)/2;
    int y = (y1+y2)/2;
    int w = abs(x1 - x2);
    int h = abs(y1 - y2);
    bbox.label = label;
    bbox.x = (float)x/width;
    bbox.y = (float)y/height;
    bbox.w = (float)w/width;
    bbox.h = (float)h/height;
    //qDebug() << QString("x:%1 ,y:%2 ,w:%3 ,h:%4").a(abs(c_rect.y - c_view.y))rg(bbox.x).arg(bbox.y).arg(bbox.w).arg(bbox.h);
    if((x&&y) && (w&&h))
    {
        bboxes.push_back(bbox);
    }
}

void LabelMaker::changeIndex(int num)
{
    if( 0<img_index && img_index<img_list.size())
    {
        writeText();
        img_index+=num;
        readText();
        loadImage();
        updateView();
    }
}
