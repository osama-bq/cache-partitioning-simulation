#include <stdexcept>
#include <memory>

#ifndef HARDWARECOMPONENT_H
#define HARDWARECOMPONENT_H

template <typename T>
class HardwareComponent {
protected:
  static std::unique_ptr<T> instance;

  HardwareComponent() = default;

public:
  static T* createInstance();
  static T* getInstance();

  // Optional: prevent copying
  HardwareComponent(const HardwareComponent&) = delete;
  HardwareComponent& operator=(const HardwareComponent&) = delete;
};

#endif