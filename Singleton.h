/*
 * Singleton.h
 *
 *  Created on: 25 juil. 2015
 *      Author: horfee
 */

#ifndef SINGLETON_H_
#define SINGLETON_H_

#include <stdlib.h>
#include <iostream>

template <typename T>
class Singleton
{
protected:
  // Constructeur/destructeur
  Singleton () { }
  ~Singleton () { }

public:
  // Interface publique
  static T *getInstance ()
  {
    if (NULL == _singleton)
      {
        _singleton = new T();
      }
    else
      {
      }

    return (static_cast<T*> (_singleton));
  }

  static void kill ()
  {
    if (NULL != _singleton)
      {
        delete _singleton;
        _singleton = NULL;
      }
  }

private:
  // Unique instance
  static T *_singleton;
};

template <typename T>
T *Singleton<T>::_singleton = NULL;


#endif /* SINGLETON_H_ */
