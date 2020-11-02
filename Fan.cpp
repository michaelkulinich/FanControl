/*Michael Kulinich
 * kulinich@chapman.edu
 * Intuitive Challenge c++
 * Fan.cpp
*/
#include "Fan.h"


Fan::Fan()
{

}

Fan::Fan(uint maxPWMCount){
  m_maxPWMCount = maxPWMCount;
  m_currentPWMCount = 0;
}

uint Fan::getMaxPWMCount(){
  return m_maxPWMCount;
}

uint Fan::getCurrentPWMCount(){
  return m_currentPWMCount;
}
void Fan::setPWMCount(float duty_cycle){
  m_currentPWMCount = round(m_maxPWMCount * duty_cycle);
}
