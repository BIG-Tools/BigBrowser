#include "chromosomwidget.h"
#include "app.h"
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>

namespace big {
namespace gui {
ChromosomWidget::ChromosomWidget(const QString &filename, QWidget * parent)
    :QWidget(parent), mCytoBandFileName(filename)
{
    // Create color map
    QColor base = QColor(200,200,200);

    mStains["stalk"] = Qt::darkCyan;
    mStains["gneg"] = base;
    mStains["gpos25"]  = base.darker(100);
    mStains["gpos50"]  = base.darker(150);
    mStains["gpos75"]  = base.darker(200);
    mStains["gpos100"] = base.darker(250);
    mStains["gvar"] = QColor(144, 195, 212);
    mStains["acen"] = QColor(80, 144, 212);

    setMouseTracking(true);
}


void ChromosomWidget::setChromosom(const QString &chromosom)
{
    CytobandReader mReader(mCytoBandFileName);

    if (mReader.open())
    {
        mChromosoms.clear();

        while (mReader.next())
        {
            if (mReader.region().chromosom() == chromosom )
                mChromosoms.append(mReader.region());
        }
        mRegionSelector.setChromosom(chromosom);

    }

    // Force the redraw of the background
    mBackgroundLayer = QImage();
    update();
}


void ChromosomWidget::setRange(qint64 start, qint64 end)
{
    qDebug() << "ChromosomWidget::setRange USED ?";
    mRegionSelector.setRange(start, end);
}





void ChromosomWidget::paintEvent(QPaintEvent *)
{
    // Is there something to draw ?
    if (mChromosoms.isEmpty())
    {
        // Todo draw empty screen error : something like the genome of the "anthropopitheque" ;)
        return;
    }



    // When resize, need to (re)compute the background
    if (mBackgroundLayer.isNull() || mBackgroundLayer.rect() != rect())
    {
        // Recompute main drawing variables
        mChromosomWidth = rect().width() - mOffsetX * 2;
        mB2PCoeff = mChromosomWidth / mChromosoms.last().last();


        // 1st part : the wrapper
        QImage img1(size(), QImage::Format_ARGB32_Premultiplied);
        QPainter img1Painter;
        img1.fill(0);

        img1Painter.begin(&img1);
        img1Painter.setRenderHint(QPainter::Antialiasing);
        img1Painter.setBrush(QColor(250,250,250));
        img1Painter.setPen(QColor(100,100,100));
        img1Painter.drawPath(getChromosomWrapperShape(2, 5));
        // Todo : apply dropshadow to this image ?
        img1Painter.end();

        // 2nd part : the regions
        QImage img2(size(), QImage::Format_ARGB32_Premultiplied);
        QPainter img2Painter;
        img2.fill(0);

        img2Painter.begin(&img2);
        img2Painter.setRenderHint(QPainter::Antialiasing);
        img2Painter.setClipPath(getChromosomWrapperShape(1, 5));
        drawRegions(&img2Painter);
        img2Painter.end();

        // 3rd part : the labels
        QImage img3(size(), QImage::Format_ARGB32_Premultiplied);
        QPainter img3Painter;
        img3.fill(0);

        img3Painter.begin(&img3);
        drawLabels(&img3Painter);
        img3Painter.end();




        // Now merge the parts into background img
        mBackgroundLayer = QImage(size(), QImage::Format_ARGB32_Premultiplied);

        QPainter bgPainter(&mBackgroundLayer);
        bgPainter.setBrush(Qt::white);
        bgPainter.setPen(Qt::NoPen);
        bgPainter.drawRect(rect());

        bgPainter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        bgPainter.drawImage(rect(), img1);
        bgPainter.drawImage(rect(), img2);
        bgPainter.drawImage(rect(), img3);
        bgPainter.end();
    }

    // Display on screen
    QPainter painter(this);
    painter.drawImage(rect(), mBackgroundLayer);
    drawFrameLayer(&painter);
}



//int wrapperPadding = 2;
//int wrc = 5; // wrapperRoundCoeff : pixel distance used by spline anchores
QPainterPath ChromosomWidget::getChromosomWrapperShape(int wrapperPadding, int wrc) const
{
    // Chromosom wrapper parameters
    float endPart1 = 0;
    float startPart2 = 0;
    float centX = 0;

    // Compute centromere special coordinates
    foreach ( Region region, mChromosoms)
    {
        QString stain = region.data("stain").toString();

        if(stain == "acen" && endPart1 == 0)
        {
            endPart1 = region.first() * mB2PCoeff;
            centX = region.last() * mB2PCoeff;
        }
        else if (endPart1 > 0  && stain != "acen" && startPart2 == 0)
        {
            startPart2 = region.first() * mB2PCoeff;
        }
    }

    // Defining key points of the spline line that will be used to draw the chromosome smoothly
    // A-----------B      D---------E
    // |             >CH<           |
    // J-----------I      G---------F
    float offsetX = mOffsetX - wrapperPadding;
    float offsetY = mOffsetY - wrapperPadding;
    QPoint A(offsetX, offsetY);
    QPoint B(offsetX + endPart1 + 2* wrapperPadding, offsetY);
    QPoint C(offsetX + centX, offsetY + mChromosomHeight / 2 - 5 );
    QPoint D(offsetX + startPart2, offsetY);
    QPoint E(offsetX + mChromosomWidth + 2* wrapperPadding , offsetY);

    offsetY += 2 * wrapperPadding + mChromosomHeight;
    QPoint F(offsetX + mChromosomWidth + 2* wrapperPadding, offsetY);
    QPoint G(offsetX + startPart2, offsetY);
    QPoint H(offsetX + centX, offsetY - mChromosomHeight / 2 + 5);
    QPoint I(offsetX + endPart1 + 2* wrapperPadding, offsetY);
    QPoint J(offsetX, offsetY );


    // Drawing chromosom wrapper
    QPainterPath path;

    path.moveTo(A);
    path.cubicTo(QPoint(A.x() + wrc, A.y()),QPoint(B.x() - wrc, B.y()),B);
    path.cubicTo(QPoint(B.x() + wrc, B.y()),QPoint(C.x() - wrc, C.y()),C);
    path.cubicTo(QPoint(C.x() + wrc, C.y()),QPoint(D.x() - wrc, D.y()),D);
    path.cubicTo(QPoint(D.x() + wrc, D.y()),QPoint(E.x() - wrc, E.y()),E);
    path.cubicTo(QPoint(E.x() + wrc, E.y()),QPoint(F.x() + wrc, F.y()),F);
    path.cubicTo(QPoint(F.x() - wrc, F.y()),QPoint(G.x() + wrc, G.y()),G);
    path.cubicTo(QPoint(G.x() - wrc, G.y()),QPoint(H.x() + wrc, H.y()),H);
    path.cubicTo(QPoint(H.x() - wrc, H.y()),QPoint(I.x() + wrc, I.y()),I);
    path.cubicTo(QPoint(I.x() - wrc, I.y()),QPoint(J.x() + wrc, J.y()),J);
    path.cubicTo(QPoint(J.x() - wrc, J.y()),QPoint(A.x() - wrc, A.y()),A);

    // Job done
    return path;
}





void ChromosomWidget::drawRegions(QPainter *painter)
{
    // Loop to draw regions
    foreach ( Region region, mChromosoms)
    {
        float regionWidth = region.length() * mB2PCoeff;
        float regionStart = mOffsetX + region.first() * mB2PCoeff;

        // Define region
        QRect fragment;
        fragment.setTopLeft(QPoint(regionStart, mOffsetY));
        fragment.setWidth(regionWidth + 1); // +1 to compensate bad rounded performed sometime by cast : float->int
        fragment.setHeight(mChromosomHeight);

        // Define brush accoding to thge region stain
        QColor base = mStains.value(region.data("stain").toString(),Qt::red);
        QLinearGradient lgrad(fragment.topLeft(), fragment.bottomLeft());
        lgrad.setColorAt(0.0, base.lighter(200));
        lgrad.setColorAt(0.4, base);
        lgrad.setColorAt(0.8, base);
        lgrad.setColorAt(1.0, base.darker(150));

        // Draw the region
        painter->setBrush(lgrad);
        painter->setPen(QPen(Qt::NoPen));
        painter->drawRect(fragment);
    }
}


void ChromosomWidget::drawLabels(QPainter *painter)
{
    // Define canvas boudaries
    int offsetY = mOffsetY + mChromosomHeight - 1 ;

    painter->setPen(QPen(QColor(50,50,50)));
    painter->setFont(QFont("Arial", 7));

    // Loop to draw labels
    foreach ( Region region, mChromosoms)
    {
        float regionWidth = region.length() * mB2PCoeff;
        float regionStart = mOffsetX + region.first() * mB2PCoeff;

        // Draw bounds lines
        if (region.data("stain").toString() != "acen")
        painter->drawLine(regionStart, mOffsetY, regionStart, offsetY );

        // Draw region label
        int deltaX =  regionWidth / 2 - 4 ;

        painter->save();
        painter->setPen(QPen(QColor(100,100,100)));
        painter->setTransform(QTransform());
        painter->translate(regionStart + deltaX, offsetY + 10);
        painter->rotate(90);
        painter->drawText(0,0, region.name());
        painter->restore();

    }
    // And the last line
    painter->drawLine(mOffsetX + mChromosomWidth, mOffsetY, mOffsetX + mChromosomWidth, offsetY );
}


void ChromosomWidget::drawFrameLayer(QPainter *painter)
{
    QColor cursorCol = (!mCursorClicked) ? Qt::red : QColor(240,140,0);

    if (mCursorActive)
    {
        // Draw the cursor
        painter->setPen(QPen(cursorCol, 1, Qt::DashLine));
        painter->drawLine(mCursorPosition.x() , 0, mCursorPosition.x(), rect().height());



        // Draw additional data if enough space
        int width = 200;
        int height = 15;
        if(rect().width() > width * 2)
        {
            painter->setPen(QPen(cursorCol, 1, Qt::SolidLine));
            painter->setBrush(cursorCol.lighter(170));

            int startX = mCursorPosition.x() - (width / 2);
            if (startX < 0)
            {
                startX = 0;
            }
            else if (startX + width > rect().width())
            {
                startX = rect().width() - width;
            }

            QRect labelRect(startX, rect().height() - height - 2, width, height);
            painter->drawRect(labelRect);

            const QLocale & cLocale = QLocale::c();
            QString ss = cLocale.toString(mCursorBasePosition);
            ss.replace(cLocale.groupSeparator(), ' ');

            QString label = mCursorRegion.chromosom().toUpper() + "    " + mCursorRegion.name() + "    " + ss;
            painter->drawText(labelRect,label, QTextOption(Qt::AlignHCenter | Qt::AlignVCenter));
        }
    }
}


void ChromosomWidget::mouseMoveEvent(QMouseEvent * event)
{
    // save cursor position
    mCursorPosition = event->pos();

    // Retrieve position in the genom
    Region region;
    float regionWidth, regionStart;

    // 1) Find the region (be human friendly with first and last regions)
    foreach (region, mChromosoms)
    {
        regionWidth = region.length() * mB2PCoeff;
        regionStart = mOffsetX + region.first() * mB2PCoeff;

        if (mCursorPosition.x() < regionStart + regionWidth)
        {
            break;
        }
    }

    // 2) Control cursor position and find position in the region
    if (mCursorPosition.x() < mOffsetX) mCursorPosition.setX(mOffsetX);

    if (mCursorPosition.x() > mOffsetX + mChromosomWidth)
    {
        mCursorPosition.setX(mOffsetX + mChromosomWidth);
        mCursorBasePosition = region.last();
        mCursorRegion = region;
    }
    else
    {
        mCursorBasePosition = (mCursorPosition.x() - mOffsetX) / mB2PCoeff;
        mCursorRegion = region;
    }

    // 3) Save information




    // Redraw browser frame layer
    update();
}

void ChromosomWidget::leaveEvent(QEvent *)
{
    mCursorActive = false;
    setCursor(Qt::ArrowCursor);
    update();
}

void ChromosomWidget::enterEvent(QEvent *)
{
    mCursorActive = true;
    setCursor(Qt::BlankCursor);
    update();
}

void ChromosomWidget::mousePressEvent(QMouseEvent * event)
{
    mCursorClicked = event->button() == Qt::LeftButton;
    update();
}
void ChromosomWidget::mouseReleaseEvent(QMouseEvent *)
{
    mCursorClicked = false;
    update();
}


}}
