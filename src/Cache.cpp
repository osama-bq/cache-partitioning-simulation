#include "Cache.h"

Cache::Cache(int s, int w) : size(s), ways(w), mem(s) {}

Cache* Cache::createInstance(int s, int w) {
  instance = std::unique_ptr<Cache>(new Cache(s, w));
  return instance.get();
}