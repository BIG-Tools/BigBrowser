#ifndef STATUSBAR_H
#define STATUSBAR_H
#include <QStatusBar>
#include <QSlider>
#include <QFrame>
#include "genom.h"
namespace big {
using namespace core;
namespace gui {
class StatusBar : public QFrame
{
    Q_OBJECT
public:
    StatusBar(QWidget * parent = 0);
    void setGenom(Genom * genom);


public Q_SLOTS:
    void setSelection(const Region& region);
Q_SIGNALS:
    void selectionChanged(const Region& region);

protected Q_SLOTS:
    void sliderChanged();


private:
    QSlider * mSlider;
    Genom   * mGenom;
    Region mCurrentRegion;


};
}}

#endif // STATUSBAR_H
