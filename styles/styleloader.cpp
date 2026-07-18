#include "styleloader.h"

#include <QFile>
#include <QTextStream>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>
#endif

namespace StyleLoader {

QString loadAppStyle()
{
    QFile file(QStringLiteral(":/styles/app.qss"));
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        in.setCodec("UTF-8");
#else
        in.setEncoding(QStringConverter::Utf8);
#endif
        return in.readAll();
    }

    return QStringLiteral(
        "QWidget#Widget { background-color: #E8ECF1; }"
        "QGraphicsView#graphicsView { background-color: #1A1D23; border-radius: 10px; }"
        "QPushButton#pushButton, QPushButton#pushButton_3 {"
        "  background-color: #0F766E; color: white; border: none;"
        "  border-radius: 8px; padding: 8px 14px; }"
        "QPushButton#deltete {"
        "  background-color: #DC2626; color: white; border: none;"
        "  border-radius: 8px; padding: 8px 14px; }"
        );
}

} // namespace StyleLoader
