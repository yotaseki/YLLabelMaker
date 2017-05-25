#include "labelmaker.h"
#include <ui_labelmaker.h>
#include <ui_dirdialog.h>
using namespace std;

LabelMaker::LabelMaker(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LabelMaker),
    d_ui(new Ui::DirDialog),
    img_index(0),
    viewoffset(50),
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
    if(imagedir.size() > 0)
    {
        d_ui->lineImageDir->setText(imagedir);
    }
    if(savedir.size() > 0)
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
    correctCoordiante(x,y);
    c_view.x = x;
    c_view.y = y;
    c_view.b = b;
    updateView();
}

void LabelMaker::onMousePressedGraphicsView(int mx, int my, Qt::MouseButton b)
{
    if(b == Qt::LeftButton)
    {
        correctCoordiante(mx,my);
        c_rect.x = mx;
        c_rect.y = my;
        c_rect.b = b;
        c_rect.flag = 1;
        updateView();
    }
    if(b == Qt::RightButton)
    {
        if(bboxes.size() > 0)
        {
            bboxes.pop_back();
        }
    }
}

void LabelMaker::onMouseReleasedGraphicsView(int mx, int my, Qt::MouseButton b)
{
    int r_x1,r_y1,r_x2,r_y2;
    if(b == Qt::LeftButton)
    {
        if(img_list.size() != 0)
        {
            if(ui->checkUseMI->checkState() == Qt::Checked)
            {
                //std::cout << "(x , y)" << mx << "," << my << std::endl;
                //std::cout << "(cols , rows )" << currentimg.cols << "," << currentimg.rows << std::endl;
                searchBboxByMI(mx,my, &r_x1, &r_y1, &r_x2, &r_y2);
                appendBbox(0, r_x1, r_y1, r_x2, r_y2);
            }
            else appendBbox(ui->spinLabelNumber->value(),c_rect.x,c_rect.y,c_view.x,c_view.y);
        }
        c_rect.flag = 0;
        updateView();

    }
    else if(b == Qt::RightButton)
    {
        updateView();
    }
}
void LabelMaker::convertAxsisGraphics2CurrentImage(int mx, int my, int *out_mx, int *out_my )
{
	int offset = viewoffset;
	mx -= offset;
	my -= offset;
    int w = scene_img_w;
    int h = scene_img_h;
    *out_mx = (int)((double)mx / w * currentimg.cols);
    *out_my = (int)((double)my / h * currentimg.rows);
}
void LabelMaker::searchBboxByMI(int mx, int my, int *out_x1, int *out_y1, int *out_x2, int *out_y2)
{
    //std::cout << "(x , y)" << mx << "," << my << std::endl;
	convertAxsisGraphics2CurrentImage(mx, my, &mx,&my);
    QImage mask = CreateMask();
    int max_r = 0;
    double max_mi = 0;
    QPoint maxp;
	QImage img = QImage(currentimg.data, currentimg.cols,currentimg.rows, QImage::Format_RGB888);
	int r_min = 15 * ((double)my / img.height());
	int r_max = 90 * ((double)my / img.height());
	r_min = (r_min > 10)?r_min:10;
	r_max = (r_max > 20)?r_max:20;
	//qDebug() << "min" << r_min << " max" << r_max;
    for(int r = r_min; r < r_max; r++)
    {
        double scale = r/100.;
        QImage ballmask = mask.scaled(mask.width()*scale, mask.height()*scale);
        int mw = ballmask.width();
        int mh = ballmask.height();
        //std::cout << "mw,mh = " << mw << "," << mh << std::endl;
		int size = 10;
		int step = 2;
        for(int x=mx-size; x < mx+size; x+=step)
		{
			if( x >= currentimg.cols )continue;
			for(int y=my-size; y < my+size; y+=step)
			{
				if( y >= currentimg.rows )continue;
				double mi = calc_mi(img, ballmask, x-r, y-r);
				//std::cout << mi << std::endl;
				if(max_mi < mi)
				{
					max_mi = mi;
					max_r = r;
					//In max mi x1,y1,x2,y2
					//std::cout << "(x1 , y1)" << "(" << *out_x1 << "," << *out_y1 << ")" <<  "(x2 , y2)" << "(" << *out_x2 << "," << *out_y2 << ")" << std::endl;
				}
			}
		}
    }
    int offset = viewoffset;
    *out_x1 = (int)((double)(mx - max_r)/currentimg.cols * scene_img_w)+offset;
    *out_y1 = (int)((double)(my - max_r)/currentimg.rows * scene_img_h)+offset;
    *out_x2 = (int)((double)(mx + max_r)/currentimg.cols * scene_img_w)+offset;
    *out_y2 = (int)((double)(my + max_r)/currentimg.rows * scene_img_h)+offset;
}
double LabelMaker::calc_mi( const QImage &img, const QImage &maskimg, int x0, int y0 )
{
    int hist[256][2];
    for(int i=0; i<256; i++) {
        for(int j=0; j<2; j++) hist[i][j] = 0;
    }

    std::vector<int> pbg(256, 0); //BG
    std::vector<int> pfg(256, 0); //FG
    int cnt_fg = 0;
    const unsigned char *imdata = img.bits();
    int bpl = img.bytesPerLine();
    const unsigned char *maskdata = maskimg.bits();
    int maskbpl = maskimg.bytesPerLine();
    int mh = maskimg.height();
    int mw = maskimg.width();
    for(int i=0; i<mh; i++) {
        for(int j=0; j<mw; j++) { 
            //int gray = qGray(img.pixel(j+x0, i+y0));
            int gray = imdata[(j+x0) * 3 + (i+y0) * bpl];
            int mask = maskdata[j*3 + i*maskbpl];
            if (mask) {
                ++pfg[gray];
                ++cnt_fg;
                ++hist[gray][0];
            } else {
                ++pbg[gray];
                ++hist[gray][1];
            }
        }
    }
    int cnt = maskimg.width()*maskimg.height();
    double p_fg = (double)cnt_fg / (double)cnt;
    double jent = 0;
    double jx = 0;
    double jy = -p_fg * log(p_fg) - (1-p_fg) * log(1-p_fg);
    //std::cout << "pfg: " << pfg << std::endl;
    for(int i=0; i<pbg.size(); i++) {
        double px = 0;
        for(int j=0; j<2; j++) {
            if (hist[i][j] > 0) {
                double p = ((double)hist[i][j]/(double)cnt);
                jent += -p * log(p);
                px += p;
            }
        }
        if (px > 0) {
            jx += -px * log(px);
        }
    }
    //std::cout << "jx, jy, jent" << jx << " " << jy << " " << jent << std::endl;
    return jx + jy - jent;
}

QImage LabelMaker::CreateMask()
{
    QImage mask(250, 250, QImage::Format_RGB888);
    mask.fill(Qt::black);
    QPainter painter(&mask);
    painter.setBrush(Qt::white);
    painter.drawEllipse(QPoint(125,125), 100,100);
    //mask.save("balltemp.png", "PNG");

    return mask;
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
    if(img_list.size() != 0)
    {
        loadImage();
        readText();
        updateView();
    }
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
	scene.setSceneRect(0,0,ui->graphicsView->width(), ui->graphicsView->height());
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
    ui->labelDebug->clear();
	ui->labelDebug->setText(
            QString("NUM_BBOX[%1]").arg(bboxes.size())
            + QString(" x:%1 y:%2").arg(c_view.x).arg(c_view.y) 
            + QString(" (size %1 x %2 )").arg(scene_img_w).arg(scene_img_h)
            );
    return 0;
}

int LabelMaker::setImage(cv::Mat img)
{
    int offset = viewoffset;
    scene_img_w = ui->graphicsView->width()-offset*2;
    scene_img_h = ui->graphicsView->height()-offset*2;
    QPixmap pix;
    if(scene_img_w<=0 || scene_img_h<=0)
    {
        return -1;
    }
    cv::resize(img,img,cv::Size(scene_img_w,scene_img_h));
    pix = myq.MatBGR2pixmap(img);
    QGraphicsPixmapItem *p = scene.addPixmap(pix);
    p->setPos(offset,offset);
    return 0;
}

int LabelMaker::drawCursur()
{
	int r = 4;
	int offset = viewoffset;
    int w = scene_img_w;
    int h = scene_img_h;
	QPen p = QPen(myq.retColor(ui->spinLabelNumber->value())),QBrush(myq.retColor(ui->spinLabelNumber->value()));
    scene.addEllipse(c_view.x-(r/2),c_view.y-(r/2),r,r,p);
	if(ui->checkCrossLine->checkState() == Qt::Checked)
	{
		scene.addLine(c_view.x   ,offset+1          ,c_view.x   ,h-1+offset   ,p);
		scene.addLine(offset+1   ,c_view.y   ,w-1+offset ,c_view.y   ,p);
	}
    return 0;
}

int LabelMaker::drawRect()
{
    scene.addRect(QRect(QPoint(c_rect.x,c_rect.y),QPoint(c_view.x,c_view.y)),QPen(myq.retColor(ui->spinLabelNumber->value()),3));
    return 0;
}

int LabelMaker::drawBbox()
{
	int offset=viewoffset;
    int w = scene_img_w;
    int h = scene_img_h;
    for(int i=0;i<bboxes.size();i++)
    {
        int x1 = w * (bboxes[i].x - (bboxes[i].w/2));
        int y1 = h * (bboxes[i].y - (bboxes[i].h/2));
        int x2 = w * (bboxes[i].x + (bboxes[i].w/2));
        int y2 = h * (bboxes[i].y + (bboxes[i].h/2));
		x1 += offset;
		y1 += offset;
		x2 += offset;
		y2 += offset;
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
	int offset = viewoffset;
    int w = scene_img_w;
    int h = scene_img_h;
    x = (x > offset) ?x:offset;
    y = (y > offset) ?y:offset;
    x = (x < w+offset-1) ?x:w+offset-1;
    y = (y < h+offset-1) ?y:h+offset-1;
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
    if(bboxes.size() > 0)
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
    bboxes.clear();
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
	x1 -= offset;
	y1 -= offset;
	x2 -= offset;
	y2 -= offset;
    int width = scene_img_w;
    int height = scene_img_h;
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
    writeText();
    img_index+=num;
    if( img_index < 0)
    {
        img_index = 0;
    }
    if(img_list.size()-1 < img_index)
    {
        img_index = img_list.size()-1;
    }
    readText();
    loadImage();
    updateView();
}
