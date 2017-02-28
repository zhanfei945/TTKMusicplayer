#include "musicplaylistfoundcategorywidget.h"
#include "musicclickedlabel.h"
#include "musicuiobject.h"

#include <QLabel>
#include <QGridLayout>
#include <QPushButton>
#include <QSignalMapper>
#include <QScrollArea>

MusicPlaylistFoundCategoryItem::MusicPlaylistFoundCategoryItem(QWidget *parent)
    : QWidget(parent)
{
    setStyleSheet(QString());
}

QString MusicPlaylistFoundCategoryItem::getClassName()
{
    return staticMetaObject.className();
}

void MusicPlaylistFoundCategoryItem::setCategory(const PlaylistCategory &category)
{
    m_category = category;

    QHBoxLayout *layout = new QHBoxLayout(this);
    QLabel *label = new QLabel(category.m_category, this);
    label->setStyleSheet(MusicUIObject::MColorStyle03 + MusicUIObject::MFontStyle03);
    label->setFixedSize(80, 20);
    layout->addWidget(label, 0, Qt::AlignTop);

    QWidget *item = new QWidget(this);
    QGridLayout *itemlayout = new QGridLayout(item);
    itemlayout->setContentsMargins(0, 0, 0, 0);

    QSignalMapper *group = new QSignalMapper(this);
    connect(group, SIGNAL(mapped(int)), SLOT(buttonClicked(int)));

    for(int i=0; i<m_category.m_items.count(); ++i)
    {
        MusicClickedLabel *l = new MusicClickedLabel(m_category.m_items[i].m_name, item);
        l->setStyleSheet(QString("QLabel::hover{%1}").arg(MusicUIObject::MColorStyle08));
        l->setFixedSize(75, 20);
        connect(l, SIGNAL(clicked()), group, SLOT(map()));
        group->setMapping(l, i);
        itemlayout->addWidget(l, i/6, i%6, Qt::AlignLeft);
    }
    item->setLayout(itemlayout);

    layout->addWidget(item, 0, Qt::AlignTop);
    setLayout(layout);
}

void MusicPlaylistFoundCategoryItem::buttonClicked(int index)
{
    if(0 <= index && index < m_category.m_items.count())
    {
        emit categoryChanged(m_category.m_items[index]);
    }
}


MusicPlaylistFoundCategoryWidget::MusicPlaylistFoundCategoryWidget(QWidget *parent)
    : MusicToolMenuWidget(parent)
{
    initWidget();

    disconnect(this, SIGNAL(clicked()), this, SLOT(popupMenu()));
    connect(this, SIGNAL(clicked()), SLOT(popupMenu()));
}

MusicPlaylistFoundCategoryWidget::~MusicPlaylistFoundCategoryWidget()
{

}

QString MusicPlaylistFoundCategoryWidget::getClassName()
{
    return staticMetaObject.className();
}

void MusicPlaylistFoundCategoryWidget::setCategory(const QString &server, QObject *obj)
{
    PlaylistCategorys categorys;
    MusicCategoryConfigManager manager;
    manager.readCategoryConfig();
    manager.readCategoryConfig(categorys, server);

    QVBoxLayout *layout = new QVBoxLayout(m_containWidget);
    QWidget *containWidget = new QWidget(m_containWidget);
    containWidget->setStyleSheet(MusicUIObject::MBackgroundStyle17);
    QVBoxLayout *containLayout = new QVBoxLayout(containWidget);
    containWidget->setLayout(containLayout);

    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setStyleSheet(MusicUIObject::MScrollBarStyle01);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setAlignment(Qt::AlignLeft);
    scrollArea->setWidget(containWidget);
    layout->addWidget(scrollArea);

    foreach(const PlaylistCategory &category, categorys)
    {
        MusicPlaylistFoundCategoryItem *item = new MusicPlaylistFoundCategoryItem(this);
        connect(item, SIGNAL(categoryChanged(PlaylistCategoryItem)), obj, SLOT(categoryChanged(PlaylistCategoryItem)));
        item->setCategory(category);
        containLayout->addWidget(item);
    }
    m_containWidget->setLayout(layout);
}

void MusicPlaylistFoundCategoryWidget::closeMenu()
{
    m_menu->close();
}

void MusicPlaylistFoundCategoryWidget::popupMenu()
{
    m_menu->exec( mapToGlobal(QPoint(0, 0)) );
}

void MusicPlaylistFoundCategoryWidget::initWidget()
{
    setFixedSize(100, 30);
    setTranslucentBackground();
    setText(tr("All"));

    QString style = MusicUIObject::MBorderStyle04 + MusicUIObject::MBackgroundStyle17;
    setObjectName("mianWidget");
    setStyleSheet(QString("#mianWidget{%1}").arg(style));

    m_containWidget->setFixedSize(630, 370);
    m_containWidget->setObjectName("containWidget");
    m_containWidget->setStyleSheet(QString("#containWidget{%1}").arg(style));

    m_menu->setStyleSheet(MusicUIObject::MMenuStyle05);

}
