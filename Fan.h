/*Michael Kulinich
 * kulinich@chapman.edu
 * Intuitive Challenge c++
 * Fan.h
*/
#ifndef FAN_H
#define FAN_H

#include <iostream>
#include <math.h>

//creates an object that will act as a fan
class Fan
{
  public:

    //constructor
    Fan();
    Fan(uint PWMCount);

    //getters setters
    uint getMaxPWMCount();
    uint getCurrentPWMCount();
    void setPWMCount(float duty_cycle);


  private:
    uint m_maxPWMCount; //make constant
    uint m_currentPWMCount;


};


#endif
