#include "AbClass.h"
#include <iostream>
#include <iterator>
#include <utility>
#include <vector>

using namespace AbClass;

class point {
  int id_;

public:
  point(int id) : id_(id) {}
  int id() const { return id_; }
};

class pointf : public point {
  std::vector<point> outs_;

public:
  pointf(int id) : point(id) {}

  void add_point(int x) { outs_.emplace_back(x); }

  const std::vector<point> &outs() { return outs_; }
};

const std::vector<point> kEmpty = {};

class base {
public:
  virtual int id() const { return 0; }
};

class basef {
public:
  virtual int id() const { return 0; }
  virtual AbConstructor<base>::Range outpoints() const = 0;
};

template <typename T>
class derived<base, T> : public base {
private:
  T *const ptr_;

public:
  derived(std::remove_pointer_t<T> *data_ptr) : ptr_(data_ptr) {}
  ~derived() = default;
  int id() const final { return ptr_->id(); }
};

template <typename T> class derived<basef, T> : public basef {
private:
  T *const ptr_;

public:
  derived(std::remove_pointer_t<T> *data_ptr) : ptr_(data_ptr) {}
  ~derived() = default;
  int id() const final { return ptr_->id(); }
  virtual AbConstructor<base>::Range outpoints() const {
    return {ptr_->outs().begin(), ptr_->outs().end()};
  }
};

int main() {
  std::vector<pointf> vs;
  for (int i = 1; i < 4; ++i) {
    pointf v(i);
    v.add_point(i * 10 + 1);
    v.add_point(i * 10 + 2);
    v.add_point(i * 10 + 3);
    vs.push_back(v);
  }

  AbConstructor<basef>::Range vrs(vs.begin(), vs.end());
  for (const auto &v : vrs) {
    std::cout << "id : " << v.id() << '\n';
    for (const auto &it : v.outpoints()) {
      std::cout << it.id() << ' ';
    }
    std::cout << "\n--------------------\n";

    std::cout << "siz : " << v.outpoints().size() << '\n';
    for (int i = 0; i < v.outpoints().size(); ++i) {
      std::cout << (v.outpoints().begin() + i)->id() << ' ';
    }
    std::cout << "\n====================\n";
  }
}
