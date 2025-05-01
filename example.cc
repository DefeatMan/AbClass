#include "AbClass.h"
#include <iostream>
#include <iterator>
#include <utility>
#include <vector>

class Point {
  int id_;

public:
  Point(int id) : id_(id) {}
  int id() const { return id_; }
};

class Points : public Point {
  std::vector<Point> outs_;

public:
  Points(int id) : Point(id) {}

  void add_point(int x) { outs_.emplace_back(x); }

  const std::vector<Point> &outs() { return outs_; }
};

const std::vector<Point> kEmpty = {};

class IdObject {
public:
  virtual int id() const { return 0; }
};

class IdObjects {
public:
  virtual int id() const { return 0; }
  virtual AbClass::Constructor<IdObject>::Range outs() const = 0;
};

// custom AbClass::derived
template <typename T> class AbClass::derived<IdObject, T> : public IdObject {
private:
  T *const ptr_;

public:
  derived(std::remove_pointer_t<T> *data_ptr) : ptr_(data_ptr) {}
  ~derived() = default;
  int id() const final { return ptr_->id(); }
};

template <typename T> class AbClass::derived<IdObjects, T> : public IdObjects {
private:
  T *const ptr_;

public:
  derived(std::remove_pointer_t<T> *data_ptr) : ptr_(data_ptr) {}
  ~derived() = default;
  int id() const final { return ptr_->id(); }
  virtual Constructor<IdObject>::Range outs() const {
    return {ptr_->outs().begin(), ptr_->outs().end()};
  }
};

int main() {
  std::vector<Points> points_vec;
  for (int i = 1; i < 4; ++i) {
    Points points(i);
    points.add_point(i * 10 + 1);
    points.add_point(i * 10 + 2);
    points.add_point(i * 10 + 3);
    points_vec.push_back(points);
  }

  AbClass::Constructor<IdObjects>::Range id_objs_vec(points_vec.begin(),
                                                     points_vec.end());
  for (const auto &id_objs : id_objs_vec) {
    std::cout << "points set id : " << id_objs.id() << '\n';
    std::cout << "points set size : " << id_objs.outs().size() << '\n';
    std::cout << "## usage 1\n";
    for (const auto &id_obj : id_objs.outs()) {
      std::cout << id_obj.id() << ' ';
    }

    std::cout << "\n--------------------\n";

    std::cout << "## usage 2\n";
    for (int i = 0; i < id_objs.outs().size(); ++i) {
      std::cout << (id_objs.outs().begin() + i)->id() << ' ';
    }

    std::cout << "\n====================\n";
  }
}
