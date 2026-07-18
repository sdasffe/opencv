#include "core/widget.h"
#include "styles/styleloader.h"

#include <QApplication>
#include <QFont>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFont appFont(QStringLiteral("Microsoft YaHei UI"), 9);
    if (appFont.exactMatch() == false)
        appFont = QFont(QStringLiteral("Microsoft YaHei"), 9);
    a.setFont(appFont);

    a.setStyleSheet(StyleLoader::loadAppStyle());

    Widget w;
    w.show();
    return QApplication::exec();
}
