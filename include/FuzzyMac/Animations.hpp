#pragma once
#include <QPropertyAnimation>
#include <QWidget>

QPropertyAnimation* opacityAnimator(QWidget* widget, const QVariant& start_val, const QVariant& end_val, int duration); 
QPropertyAnimation* bounceAnimator(QWidget* widget,const QVariant& scale_factor, int duration); 
QPropertyAnimation* resizeAnimation(QWidget* widget,const QSize& final_size, int duration); 




