#include "HardwareComponent.h"
#include "CPU.h"
#include "RAM.h"
#include "Cache.h"


template class HardwareComponent<CPU>;
template class HardwareComponent<RAM>;
template class HardwareComponent<Cache>;

template <typename T>
std::unique_ptr<T> HardwareComponent<T>::instance = nullptr;

template <typename T>
T* HardwareComponent<T>::createInstance() {
  instance = std::unique_ptr<T>(new T());
  return instance.get();
}

template <typename T>
T* HardwareComponent<T>::getInstance() {
  if (!instance)
    throw std::runtime_error("No instance created!");
  return instance.get();
}
