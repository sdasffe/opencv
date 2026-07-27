#ifndef IBLOCKHOST_H
#define IBLOCKHOST_H

#include <QList>
#include <QPixmap>
#include "roiinfo.h"

/** 宿主能力：块需要原图/ROI 时用（如 Otsu）。主窗口实现。 */
class IBlockHost
{
public:
    virtual ~IBlockHost() = default;
    virtual bool hostHasImage() const = 0;
    virtual QPixmap hostOriginalImage() const = 0;
    virtual QList<RoiInfo> hostCurrentRois() const = 0;
};

#endif
