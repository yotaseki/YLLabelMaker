#include "MyQclass.h"

QFileInfo MyQclass::selectFile(QString filter)
{
    QFileDialog::Options options;
    QString strSelectedFilter;
    QString strFName = QFileDialog::getOpenFileName(
                this,
                tr( "select file" ),
                QDir::homePath(),
                filter,
                &strSelectedFilter,
                options );

    if ( !strFName.isEmpty() )
    {
        QFileInfo fileinfo(strFName);
        return fileinfo;
    }
    QMessageBox::warning(this,"Error","File not found.");
    QFileInfo none;
    return none;
}

QDir MyQclass::selectDir(QDir current)
{
    QFileDialog::Options options = QFileDialog::DontResolveSymlinks | QFileDialog::ShowDirsOnly;
    if(!current.exists())
    {
        current = QDir::homePath();
    }
    QString strDir = QFileDialog::getExistingDirectory(
                this
                ,tr("select Dir")
                ,current.path()
                ,options);

    if ( !strDir.isEmpty() )
    {
        QDir dir(strDir);
        return dir;
    }
    QMessageBox::warning(this,"Error","Directly not found.");
    QDir none;
    return none;
}


QFileInfoList MyQclass::scanDirs(QString targetpath,QString filter)
{
    QDir dir( targetpath );
    QStringList strlFilter;
    strlFilter << filter;
    QFileInfoList list = dir.entryInfoList( strlFilter, QDir::Dirs );

    return list;
}

QFileInfoList MyQclass::scanFiles(QString targetpath ,QString filter)
{
    QDir dir( targetpath );
    QStringList strlFilter;
    strlFilter << filter;
    QFileInfoList list = dir.entryInfoList( strlFilter, QDir::Files );

    return list;
}

QPixmap MyQclass::MatBGR2pixmap(cv::Mat src)
{
    if(CV_MAJOR_VERSION == 4){
        cv::cvtColor(src, src,cv::COLOR_BGR2BGRA);
    }
    else{
        cv::cvtColor(src, src,CV_BGR2BGRA);
    }
    QPixmap pixmap = QPixmap::fromImage(QImage(src.data, src.cols, src.rows, QImage::Format_ARGB32));
    return pixmap;
}

QDir MyQclass::makeDirectory(QString targetPath, QString dirname)
{
    QDir dir(targetPath);
    if(!dir.cd(dirname))
    {
        dir.mkdir(dirname);
        dir.cd(dirname);
    }
    return dir;
}

QString MyQclass::filenameGen(QString objectname,int num,QString extension)
{
    QDateTime dateTime = QDateTime::currentDateTime();
    QString fname = QString("%1%2%3%4%5")
            .arg(objectname)
            .arg(dateTime.date().toString("MMdd"))
            .arg(dateTime.time().toString("hhmm"))
            .arg(num+1,5,10,QChar('0'))
            .arg(extension);
    return fname;
}

QColor MyQclass::retColor(int color)
{
    if(color==RED   )return QColor(0xFF,   0,   0);
    if(color==GREEN )return QColor(   0,0xFF,   0);
    if(color==BLUE  )return QColor(   0,   0,0xFF);
    if(color==ORANGE)return QColor(0xFF,0x69,0x28);
    if(color==YELLOW)return QColor(0xFF,0xFF,   0);
    if(color==AQUA  )return QColor(   0,0xFF,0xFF);
    if(color==PURPLE)return QColor(0xFF,   0,0xFF);
    if(color==DARK  )return QColor(   0,0x80,   0);
    if(color==NAVY  )return QColor(   0,   0,0x80);
    else      return QColor(255/color,255/color,255/color);
};
