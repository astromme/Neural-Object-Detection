#include "aibocontrol.h"
AiboRemote::AiboRemote(Aibo* aibo) {
    m_aibo = aibo;
}
void AiboRemote::headLeft() {
    m_aibo->setPan(1);
}
void AiboRemote::headRight() {
    m_aibo->setPan(-1);
}
void AiboRemote::headUp() {
    m_aibo->setTilt(0);
}
void AiboRemote::headDown() {
    m_aibo->setTilt(-1);
}
void AiboRemote::headCenter() {
    m_aibo->setTilt(0);
    m_aibo->setPan(0);
}
void AiboRemote::stop() {
    m_aibo->setMovement(0, 0);
    m_aibo->setStrafing(0);
}
void AiboRemote::moveForward() {
    m_aibo->setTranslation(0.5);
}
void AiboRemote::moveBackward() {
    m_aibo->setTranslation(-0.5);
}
void AiboRemote::moveLeft() {
    m_aibo->setStrafing(0.5);
}
void AiboRemote::moveRight() {
    m_aibo->setStrafing(-0.5);
}
void AiboRemote::turnRight() {
    m_aibo->setRotation(-0.5);
}
void AiboRemote::turnLeft() {
    m_aibo->setRotation(0.5);
}
