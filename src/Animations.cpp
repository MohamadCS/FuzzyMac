#include "FuzzyMac/Animations.hpp"

QPropertyAnimation* obacityAnimator(QWidget* widget, const QVariant& start_val, const QVariant& end_val, int duration) {
    QPropertyAnimation* anim = new QPropertyAnimation(widget, "windowOpacity");
    anim->setDuration(duration);
    anim->setStartValue(start_val);
    anim->setEndValue(end_val);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
    return anim;
}

QPropertyAnimation* bounceAnimator(QWidget* widget, const QVariant& scale_factor, int duration) {
    QRect original = widget->geometry();
    int dx = original.width() * scale_factor.toDouble();
    int dy = original.height() * scale_factor.toDouble();

    QRect enlarged(original.x() - dx / 2, original.y() - dy / 2, original.width() + dx, original.height() + dy);

    auto* anim = new QPropertyAnimation(widget, "geometry");
    anim->setDuration(duration);
    anim->setKeyValues({{0.0, original}, {0.5, enlarged}, {1.0, original}});
    anim->setEasingCurve(QEasingCurve::OutQuad);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
    return anim;
}

QPropertyAnimation* resizeAnimation(QWidget* widget, const QSize& final_size, int duration) {
    QRect start = widget->frameGeometry();
    QRect end = QRect(start.center() - QPoint(final_size.width() / 2, final_size.height() / 2), final_size);

    // Create animation
    QPropertyAnimation* anim = new QPropertyAnimation(widget, "geometry");
    anim->setDuration(duration); // milliseconds
    anim->setStartValue(start);
    anim->setEndValue(end);
    anim->setEasingCurve(QEasingCurve::OutExpo);
    anim->start(QAbstractAnimation::DeleteWhenStopped);

    return anim;
}
