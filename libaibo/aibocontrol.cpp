#include "aibocontrol.h"
AiboControl::AiboControl(Aibo* aibo) {
    m_aibo = aibo;
}
void AiboControl::moveFocusLeft(qreal velocity) {
    m_aibo->setPan(1);
}
void AiboControl::moveFocusRight(qreal velocity) {
    m_aibo->setPan(-1);
}
void AiboControl::moveFocusUp(qreal velocity) {
    m_aibo->setTilt(0);
}
void AiboControl::moveFocusDown(qreal velocity) {
    m_aibo->setTilt(-1);
}
void AiboControl::stopMovingFocus() {
}
void AiboControl::moveForward() {
    m_aibo->setTranslation(0.5);
}
void AiboControl::moveBackward() {
    m_aibo->setTranslation(-0.5);
}
void AiboControl::moveLeft() {
    m_aibo->setStrafing(0.5);
}
void AiboControl::moveRight() {
    m_aibo->setStrafing(-0.5);
}
void AiboControl::turnRight() {
    m_aibo->setRotation(-0.5);
}
void AiboControl::turnLeft() {
    m_aibo->setRotation(0.5);
}
