#include "musicscreensaverwidget.h"
#include "musicapplicationobject.h"
#include "musicsettingmanager.h"
#include "musicuiobject.h"
#include "musictoolsetsuiobject.h"
#include "musicdownloadqueuecache.h"
#include "musicimageutils.h"
#include "musicstringutils.h"
#///Oss import
#include "qoss/qossconf.h"

#include <QEvent>
#include <QTimer>
#include <QPainter>
#include <QBoxLayout>
#include <QApplication>

#define ITEM_COUNT          4
#define OS_COUNT            10
#define OS_SCREENSAVER_URL  "ScreenSaver"
#define OS_WALLPAPER_NAME   "wallpaper.png"
#define OS_WALLBAR_NAME     "wallbar.png"
#define OS_WALLNAIL_NAME    "thumbnail.png"

MusicScreenSaverListItem::MusicScreenSaverListItem(QWidget *parent)
    : QLabel(parent)
{
    setFixedSize(155, 100);
    setStyleSheet(MusicUIObject::MQSSBackgroundStyle01);

    m_index = -1;
    m_enableButton = new QPushButton(this);
    m_enableButton->setCursor(Qt::PointingHandCursor);
    m_enableButton->setStyleSheet(MusicUIObject::MQSSScreenItemDisable);
    m_enableButton->setGeometry((155 - 38) / 2, (100 - 38) / 2, 38, 38);
    m_enableButton->hide();
#ifdef Q_OS_UNIX
    m_ui->m_enableButton->setFocusPolicy(Qt::NoFocus);
#endif
    connect(m_enableButton, SIGNAL(clicked()), SLOT(caseButtonOnAndOff()));
}

void MusicScreenSaverListItem::setFilePath(const QString &path)
{
    m_path = path;
    setPixmap(QPixmap(m_path));
}

QString MusicScreenSaverListItem::getFilePath() const
{
    return m_path;
}

void MusicScreenSaverListItem::setStatus(int index, bool status)
{
    m_index = index;
    if(!status)
    {
        caseButtonOnAndOff();
    }
}

void MusicScreenSaverListItem::caseButtonOnAndOff()
{
    if(m_enableButton->styleSheet().contains(MusicUIObject::MQSSScreenItemDisable))
    {
        setPixmap(MusicUtils::Image::grayScalePixmap(QPixmap(m_path), 70));
        m_enableButton->setStyleSheet(MusicUIObject::MQSSScreenItemEnable);
        Q_EMIT itemClicked(m_index, false);
    }
    else
    {
        setPixmap(QPixmap(m_path));
        m_enableButton->setStyleSheet(MusicUIObject::MQSSScreenItemDisable);
        Q_EMIT itemClicked(m_index, true);
    }
}

void MusicScreenSaverListItem::leaveEvent(QEvent *event)
{
    QLabel::leaveEvent(event);
    m_enableButton->hide();
}

void MusicScreenSaverListItem::enterEvent(QEvent *event)
{
    QLabel::enterEvent(event);
    m_enableButton->show();
}



MusicScreenSaverListWidget::MusicScreenSaverListWidget(QWidget *parent)
    : QWidget(parent)
{
    m_gridLayout = new QGridLayout(this);
    m_gridLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_gridLayout->setContentsMargins(7, 7, 7, 7);
    setLayout(m_gridLayout);
}

MusicScreenSaverListWidget::~MusicScreenSaverListWidget()
{
    qDeleteAll(m_items);
}

void MusicScreenSaverListWidget::createItem(QObject *object, const QString &path, int index, bool status)
{
    MusicScreenSaverListItem *item = new MusicScreenSaverListItem(this);
    item->setFilePath(path);
    item->setStatus(index, status);
    connect(item, SIGNAL(itemClicked(int,bool)), object, SLOT(itemHasClicked(int,bool)));

    m_gridLayout->addWidget(item, m_items.count() / ITEM_COUNT, m_items.count() % ITEM_COUNT, Qt::AlignLeft | Qt::AlignTop);
    m_items << item;
}



MusicScreenSaverWidget::MusicScreenSaverWidget(QWidget *parent)
    : QWidget(parent)
{
    setStyleSheet(MusicUIObject::MQSSBackgroundStyle17 + MusicUIObject::MQSSColorStyle09);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(mainWidget);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainWidget->setLayout(mainLayout);
    layout->addWidget(mainWidget);
    setLayout(layout);
    //
    QWidget *topWidget = new QWidget(this);
    topWidget->setFixedHeight(50);
    QHBoxLayout *topWidgetLayout = new QHBoxLayout(topWidget);
    topWidgetLayout->setContentsMargins(10, 10, 10, 10);

    QLabel *pLabel = new QLabel(tr("Screen Saver "), topWidget);
    QFont pLabelFont = pLabel->font();
    pLabelFont.setPixelSize(20);
    pLabel->setFont(pLabelFont);
    pLabel->setStyleSheet(MusicUIObject::MQSSColorStyle13);

    QLabel *iLabel = new QLabel(tr("Spend Your Leisure Time With You"), topWidget);
    QFont iLabelFont = iLabel->font();
    iLabelFont.setPixelSize(15);
    iLabel->setFont(iLabelFont);

    QLabel *wLabel = new QLabel(tr("Wait"), topWidget);
    QLabel *mLabel = new QLabel(tr("Min"), topWidget);

    m_inputEdit = new QLineEdit(topWidget);
    m_inputEdit->setFixedWidth(50);
    m_inputEdit->setEnabled(false);
    m_inputEdit->setAlignment(Qt::AlignCenter);
    m_inputEdit->setStyleSheet(MusicUIObject::MQSSLineEditStyle01);

    m_caseButton = new QPushButton(topWidget);
    m_caseButton->setFixedSize(44, 20);
    m_caseButton->setCursor(Qt::PointingHandCursor);
    m_caseButton->setStyleSheet(MusicUIObject::MQSSScreenSaverOff);
#ifdef Q_OS_UNIX
    m_ui->m_caseButton->setFocusPolicy(Qt::NoFocus);
#endif

    topWidgetLayout->addWidget(pLabel);
    topWidgetLayout->addWidget(iLabel);
    topWidgetLayout->addStretch(1);
    topWidgetLayout->addWidget(wLabel);
    topWidgetLayout->addWidget(m_inputEdit);
    topWidgetLayout->addWidget(mLabel);
    topWidgetLayout->addWidget(m_caseButton);
    topWidget->setLayout(topWidgetLayout);
    mainLayout->addWidget(topWidget);
    //
    QFrame *frame = new QFrame(this);
    frame->setFixedHeight(1);
    frame->setFrameShape(QFrame::HLine);
    frame->setFrameShadow(QFrame::Plain);
    frame->setStyleSheet(MusicUIObject::MQSSColorStyle14);
    mainLayout->addWidget(frame);
    //
    QWidget *functionWidget = new QWidget(this);
    QHBoxLayout *functionWidgetLayout = new QHBoxLayout(functionWidget);
    functionWidgetLayout->setContentsMargins(10, 10, 10, 10);
    functionWidget->setLayout(functionWidgetLayout);
    mainLayout->addWidget(functionWidget);

    m_backgroundList = new MusicScreenSaverListWidget(this);
    functionWidgetLayout->addWidget(m_backgroundList);

    connect(m_inputEdit, SIGNAL(textChanged(QString)), SLOT(inputDataChanged()));
    connect(m_caseButton, SIGNAL(clicked()), SLOT(caseButtonOnAndOff()));

    initialize();
    applySettingParameter();
}

MusicScreenSaverWidget::~MusicScreenSaverWidget()
{

}

void MusicScreenSaverWidget::applySettingParameter()
{
    const bool state = M_SETTING_PTR->value(MusicSettingManager::OtherScreenSaverEnable).toBool();
    const int value = M_SETTING_PTR->value(MusicSettingManager::OtherScreenSaverTime).toInt();

    m_inputEdit->setText(QString::number(value));
    if(state)
    {
        caseButtonOnAndOff();
    }
}

QVector<bool> MusicScreenSaverWidget::parseSettingParameter()
{
    QVector<bool> statusVector;
    statusVector.fill(true, OS_COUNT);

    const QString &value = M_SETTING_PTR->value(MusicSettingManager::OtherScreenSaverIndex).toString();
    const QStringList items(value.split(";"));
    foreach(const QString &item, items)
    {
        const QStringList itemStatus(item.split(","));
        if(itemStatus.count() == 2)
        {
            const int index = itemStatus[0].toInt();
            const bool status = itemStatus[1].toInt();
            statusVector[index] = status;
        }
    }
    return statusVector;
}

void MusicScreenSaverWidget::inputDataChanged()
{
    const bool state = M_SETTING_PTR->value(MusicSettingManager::OtherScreenSaverEnable).toBool();
    if(state)
    {
        M_SETTING_PTR->setValue(MusicSettingManager::OtherScreenSaverTime, m_inputEdit->text().toInt());
        MusicApplicationObject::instance()->applySettingParameter();
    }
}

void MusicScreenSaverWidget::caseButtonOnAndOff()
{
    const bool state = m_caseButton->styleSheet().contains(":/toolSets/btn_saver_off");
    if(state)
    {
        m_caseButton->setStyleSheet(MusicUIObject::MQSSScreenSaverOn);
        M_SETTING_PTR->setValue(MusicSettingManager::OtherScreenSaverTime, m_inputEdit->text().toInt());
    }
    else
    {
        m_caseButton->setStyleSheet(MusicUIObject::MQSSScreenSaverOff);
    }

    m_inputEdit->setEnabled(state);
    M_SETTING_PTR->setValue(MusicSettingManager::OtherScreenSaverEnable, state);
    MusicApplicationObject::instance()->applySettingParameter();
}

void MusicScreenSaverWidget::downLoadDataChanged(const QString &data)
{
    QVector<bool> statusVector(parseSettingParameter());
    if(data.contains(OS_WALLNAIL_NAME))
    {
        const int index = MusicUtils::String::stringSplitToken(data, SCREEN_DIR, "/", true).toInt();
        if(index < 0 || index >= statusVector.count())
        {
            return;
        }
        m_backgroundList->createItem(this, data, index, statusVector[index]);
    }
}

void MusicScreenSaverWidget::itemHasClicked(int index, bool status)
{
    QVector<bool> statusVector(parseSettingParameter());
    statusVector[index] = status;

    QStringList items;
    for(int i=0; i<statusVector.count(); ++i)
    {
        items << QString("%1,%2").arg(i).arg(statusVector[i]);
    }
    M_SETTING_PTR->setValue(MusicSettingManager::OtherScreenSaverIndex, items.join(";"));
    MusicApplicationObject::instance()->applySettingParameter();
}

void MusicScreenSaverWidget::initialize()
{
    m_downloadQueue = new MusicDownloadQueueCache(MusicObject::DownloadBigBackground, this);
    connect(m_downloadQueue, SIGNAL(downLoadDataChanged(QString)), SLOT(downLoadDataChanged(QString)));

    MusicDownloadQueueDatas datas;
    for(int i=0; i<OS_COUNT; i++)
    {
        const QString &url = QOSSConf::generateDataBucketUrl() + QString("%1/%2/").arg(OS_SCREENSAVER_URL).arg(i);
        const QString &path = QString("%1%2/").arg(SCREEN_DIR_FULL).arg(i);
        QDir().mkpath(path);

        MusicDownloadQueueData wallData;
        wallData.m_url = url + OS_WALLPAPER_NAME;
        wallData.m_savePath = path + OS_WALLPAPER_NAME;
        datas << wallData;

        MusicDownloadQueueData barData;
        barData.m_url = url + OS_WALLBAR_NAME;
        barData.m_savePath = path + OS_WALLBAR_NAME;
        datas << barData;

        MusicDownloadQueueData nailData;
        nailData.m_url = url + OS_WALLNAIL_NAME;
        nailData.m_savePath = path + OS_WALLNAIL_NAME;
        datas << nailData;
    }

    m_downloadQueue->addImageQueue(datas);
    m_downloadQueue->startToDownload();
}


MusicScreenSaverBackgroundWidget::MusicScreenSaverBackgroundWidget(QWidget *parent)
    : MusicTransitionAnimationLabel(parent)
{
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

    m_state = false;
    m_isRunning = false;

    m_runningTimer = new QTimer(this);
    connect(m_runningTimer, SIGNAL(timeout()), SLOT(runningTimeout()));

    m_backgroundTimer = new QTimer(this);
    m_backgroundTimer->setInterval(15 * MT_S2MS);
    connect(m_backgroundTimer, SIGNAL(timeout()), SLOT(backgroundTimeout()));

    hide();
    qApp->installEventFilter(this);
}

MusicScreenSaverBackgroundWidget::~MusicScreenSaverBackgroundWidget()
{
    if(m_runningTimer->isActive())
    {
        m_runningTimer->stop();
    }
    delete m_runningTimer;

    if(m_backgroundTimer->isActive())
    {
        m_backgroundTimer->stop();
    }
    delete m_backgroundTimer;
}

void MusicScreenSaverBackgroundWidget::applySettingParameter()
{
    if(m_runningTimer->isActive())
    {
        m_runningTimer->stop();
        m_backgroundTimer->stop();
    }

    m_state = M_SETTING_PTR->value(MusicSettingManager::OtherScreenSaverEnable).toBool();
    const int value = M_SETTING_PTR->value(MusicSettingManager::OtherScreenSaverTime).toInt();
    m_state = (m_state && (value > 0));
    if(value > 0)
    {
        m_runningTimer->setInterval(value * MT_M2MS);
        m_runningTimer->start();
    }
}

void MusicScreenSaverBackgroundWidget::runningTimeout()
{
    if(!m_isRunning)
    {
        m_isRunning = true;
        setParent(nullptr);
        showFullScreen();

        backgroundTimeout();
        m_backgroundTimer->start();
    }
}

void MusicScreenSaverBackgroundWidget::backgroundTimeout()
{
    QVector<bool> statusVector(MusicScreenSaverWidget::parseSettingParameter());
    QVector<int> intVector;
    for(int i=0; i<OS_COUNT; i++)
    {
        if(statusVector[i])
        {
            intVector << i;
        }
    }

    if(!intVector.isEmpty())
    {
        const int index = intVector[qrand() % intVector.count()];
        QPixmap background(QString("%1%2/").arg(SCREEN_DIR_FULL).arg(index) + OS_WALLPAPER_NAME);
        QPixmap bar(QString("%1%2/").arg(SCREEN_DIR_FULL).arg(index) + OS_WALLBAR_NAME);
        MusicUtils::Image::fusionPixmap(background, bar, QPoint(100, 900));
        setPixmap(QPixmap(background.scaled(size())));
    }
}

bool MusicScreenSaverBackgroundWidget::eventFilter(QObject *watched, QEvent *event)
{
    if(event->type()== QEvent::MouseButtonPress || event->type()== QEvent::MouseButtonRelease ||
       event->type()== QEvent::MouseButtonDblClick || event->type()== QEvent::MouseMove ||
       event->type()== QEvent::KeyPress || event->type()== QEvent::KeyRelease)
    {
        if(m_state)
        {
            if(m_isRunning)
            {
                m_isRunning = false;
                hide();
            }
            m_runningTimer->start();
        }
        else
        {
            m_runningTimer->stop();
            m_backgroundTimer->stop();
        }
    }
    return MusicTransitionAnimationLabel::eventFilter(watched, event);
}
