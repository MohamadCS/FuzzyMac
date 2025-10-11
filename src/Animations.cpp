#include "FuzzyMac/Animations.hpp"
#include <QPropertyAnimation>

QPropertyAnimation* opacityAnimator(QWidget* widget, const QVariant& start_val, const QVariant& end_val, int duration) {
    QPropertyAnimation* anim = new QPropertyAnimation(widget, "windowOpacity");
    anim->setDuration(duration);
    anim->setStartValue(start_val);
    anim->setEndValue(end_val);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
    return anim;
}

QPropertyAnimation* bounceAnimator(QWidget* widget, const QVariant& scale_factor, int duration) {

    // Animation on geometry (size + position)
    QPropertyAnimation* animation = new QPropertyAnimation(widget, "geometry");

    QRect originalRect = widget->geometry();
    QPoint center = originalRect.center(); // Get the center point

    int scale = 20; // Total increase in width and height

    // New scaled size keeping center unchanged
    QSize scaledSize = originalRect.size() + QSize(scale, scale);
    QRect scaledRect(QPoint(0, 0), scaledSize);
    scaledRect.moveCenter(center); // Keep the center in place

    animation->setDuration(600);
    animation->setStartValue(originalRect);
    animation->setKeyValueAt(0.5, scaledRect);
    animation->setEndValue(originalRect);
    animation->setEasingCurve(QEasingCurve::OutInBack);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
    return animation;
}

QPropertyAnimation* resizeAnimation(QWidget* widget, const QSize& final_size, int duration) {
    QSize targetSize = widget->sizeHint();
    QPropertyAnimation* anim = new QPropertyAnimation(widget, "size");
    anim->setDuration(150);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->setStartValue(widget->size());
    anim->setEndValue(targetSize);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
    return anim;
}
