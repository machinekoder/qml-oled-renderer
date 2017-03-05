#ifndef ANIMATIONDRIVER_H
#define ANIMATIONDRIVER_H

#include <QtCore/QAnimationDriver>

class AnimationDriver : public QAnimationDriver
{
public:
    AnimationDriver(int msPerStep);

    void advance() override;
    qint64 elapsed() const override;

private:
    int m_step;
    qint64 m_elapsed;

};

#endif // ANIMATIONDRIVER_H
