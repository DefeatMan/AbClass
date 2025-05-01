#pragma once

#include <algorithm>
#include <cstddef>
#include <memory>
#include <type_traits>

namespace AbClass {

template <typename Base, typename T> class derived : public Base {
private:
  T *const ptr_;

public:
  derived(T *data_ptr) : ptr_(data_ptr) {}
  ~derived() = default;
};

template <typename Base> class Wrap {
private:
  std::unique_ptr<Base> ptr_;

public:
  template <typename T>
  Wrap(T *data_ptr) : ptr_(new derived<Base, T>(data_ptr)) {}
  Base &operator*() const { return *ptr_.get(); }
  Base *operator->() const { return ptr_.get(); }
};

template <typename Base> class WrapIter {
private:
  class iterbase {
  private:
    virtual iterbase *copy() const = 0;
    virtual void move(std::ptrdiff_t i) = 0;
    friend class WrapIter<Base>;

    virtual Base &operator*() = 0;
    virtual const Wrap<Base> &operator->() = 0;
    virtual std::ptrdiff_t operator-(const iterbase *other) const = 0;
    virtual bool operator==(const iterbase *other) const = 0;
    virtual bool operator!=(const iterbase *other) const = 0;
  };

  template <typename T> class iterderived : public iterbase {
  private:
    T iter_;
    Wrap<Base> ptr_;
    iterbase *copy() const final { return new iterderived<T>(iter_); }
    void move(std::ptrdiff_t i) final { iter_ += i; };

    friend class WrapIter<Base>;

    iterderived(const T &iter) : iter_(iter), ptr_(&*iter_) {}
    iterderived(const iterderived &other) : iter_(other.iter_), ptr_(&*iter_) {}

    bool operator==(const iterderived &other) const {
      return iter_ == other.iter_;
    }

    bool operator!=(const iterderived &other) const {
      return iter_ != other.iter_;
    }

    std::ptrdiff_t operator-(const iterderived &other) const {
      return iter_ - other.iter_;
    }

    Base &operator*() final {
      ptr_ = {&*iter_};
      return *ptr_;
    }

    const Wrap<Base> &operator->() final {
      ptr_ = {&*iter_};
      return ptr_;
    }

    std::ptrdiff_t operator-(const iterbase *other) const final {
      return operator-(*(dynamic_cast<const iterderived<T> *>(other)));
    }

    bool operator==(const iterbase *other) const final {
      return operator==(*(dynamic_cast<const iterderived<T> *>(other)));
    }

    bool operator!=(const iterbase *other) const final {
      return operator!=(*(dynamic_cast<const iterderived<T> *>(other)));
    }
  };

private:
  std::unique_ptr<iterbase> ptr_;

public:
  template <typename T>
  WrapIter(const T &iter) : WrapIter(new iterderived<T>(iter)) {}
  template <typename T> WrapIter(iterderived<T> *ptr) : ptr_(ptr) {}
  template <typename T> WrapIter(const iterderived<T> *ptr) : ptr_(ptr) {}
  WrapIter(const WrapIter &other) : ptr_(other.ptr_->copy()) {}
  WrapIter(WrapIter &&other) : ptr_(std::move(other.ptr_)) {}

  Base &operator*() const { return ptr_->operator*(); }
  const Wrap<Base> &operator->() const { return ptr_->operator->(); }

  bool operator==(const WrapIter &other) const {
    return ptr_.get()->operator==(other.ptr_.get());
  }

  bool operator!=(const WrapIter &other) const {
    return ptr_.get()->operator!=(other.ptr_.get());
  }

  WrapIter &operator++() {
    ptr_.get()->move(1);
    return *this;
  }

  WrapIter &operator++(int) {
    WrapIter tmp(*this);
    ptr_.get()->move(1);
    return tmp;
  }

  WrapIter &operator--() {
    ptr_.get()->move(-1);
    return *this;
  }

  WrapIter &operator--(int) {
    WrapIter tmp(*this);
    ptr_.get()->move(-1);
    return tmp;
  }

  WrapIter &operator+=(std::ptrdiff_t i) {
    ptr_.get()->move(i);
    return *this;
  }

  WrapIter &operator-=(std::ptrdiff_t i) {
    ptr_.get()->move(-i);
    return *this;
  }

  friend WrapIter operator+(const WrapIter &lhs, std::ptrdiff_t rhs) {
    WrapIter tmp(lhs);
    tmp.ptr_->move(rhs);
    return tmp;
  }

  friend WrapIter operator-(const WrapIter &lhs, std::ptrdiff_t rhs) {
    WrapIter tmp(lhs);
    tmp.ptr_->move(-rhs);
    return tmp;
  }

  friend std::ptrdiff_t operator-(const WrapIter &lhs, const WrapIter &rhs) {
    return lhs.ptr_.get()->operator-(rhs.ptr_.get());
  }

  friend WrapIter operator+(std::ptrdiff_t lhs, const WrapIter &rhs) {
    return rhs + lhs;
  }

  friend WrapIter operator-(std::ptrdiff_t lhs, const WrapIter &rhs) {
    return rhs - lhs;
  }
};

template <typename Base> class AbConstructor {
public:
  template <typename T> static WrapIter<Base> make_wrapiter(const T &iter) {
    return iter;
  }

  class Range {
  private:
    const WrapIter<Base> begin_;
    const WrapIter<Base> end_;

  public:
    template <typename T>
    Range(const T &begin, const T &end) : begin_(begin), end_(end) {}
    const auto &begin() const { return begin_; }
    const auto &end() const { return end_; }
    auto size() const { return end() - begin(); }
  };
};

} // namespace AbClass
