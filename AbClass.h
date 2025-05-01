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

template <typename Base> class wrapper {
private:
  std::unique_ptr<Base> ptr_;

public:
  template <typename T>
  wrapper(T *data_ptr) : ptr_(new derived<Base, T>(data_ptr)) {}
  Base &operator*() const { return *ptr_.get(); }
  Base *operator->() const { return ptr_.get(); }
};

template <typename Base> class iterator_wrapper {
private:
  class iterator_base {
  private:
    virtual iterator_base *copy() const = 0;
    virtual void move(std::ptrdiff_t i) = 0;
    friend class iterator_wrapper<Base>;

    virtual Base &operator*() = 0;
    virtual const wrapper<Base> &operator->() = 0;
    virtual std::ptrdiff_t operator-(const iterator_base *other) const = 0;
    virtual bool operator==(const iterator_base *other) const = 0;
    virtual bool operator!=(const iterator_base *other) const = 0;
  };

  template <typename T> class iterator_derived : public iterator_base {
  private:
    T iter_;
    wrapper<Base> ptr_;
    iterator_base *copy() const final { return new iterator_derived<T>(iter_); }
    void move(std::ptrdiff_t i) final { iter_ += i; };

    friend class iterator_wrapper<Base>;

    iterator_derived(const T &iter) : iter_(iter), ptr_(&*iter_) {}
    iterator_derived(const iterator_derived &other)
        : iter_(other.iter_), ptr_(&*iter_) {}

    bool operator==(const iterator_derived &other) const {
      return iter_ == other.iter_;
    }

    bool operator!=(const iterator_derived &other) const {
      return iter_ != other.iter_;
    }

    std::ptrdiff_t operator-(const iterator_derived &other) const {
      return iter_ - other.iter_;
    }

    Base &operator*() final {
      ptr_ = {&*iter_};
      return *ptr_;
    }

    const wrapper<Base> &operator->() final {
      ptr_ = {&*iter_};
      return ptr_;
    }

    std::ptrdiff_t operator-(const iterator_base *other) const final {
      return operator-(*(dynamic_cast<const iterator_derived<T> *>(other)));
    }

    bool operator==(const iterator_base *other) const final {
      return operator==(*(dynamic_cast<const iterator_derived<T> *>(other)));
    }

    bool operator!=(const iterator_base *other) const final {
      return operator!=(*(dynamic_cast<const iterator_derived<T> *>(other)));
    }
  };

private:
  std::unique_ptr<iterator_base> ptr_;

public:
  template <typename T>
  iterator_wrapper(const T &iter)
      : iterator_wrapper(new iterator_derived<T>(iter)) {}
  template <typename T>
  iterator_wrapper(iterator_derived<T> *ptr) : ptr_(ptr) {}
  template <typename T>
  iterator_wrapper(const iterator_derived<T> *ptr) : ptr_(ptr) {}
  iterator_wrapper(const iterator_wrapper &other) : ptr_(other.ptr_->copy()) {}
  iterator_wrapper(iterator_wrapper &&other) : ptr_(std::move(other.ptr_)) {}

  Base &operator*() const { return ptr_->operator*(); }
  const wrapper<Base> &operator->() const { return ptr_->operator->(); }

  bool operator==(const iterator_wrapper &other) const {
    return ptr_.get()->operator==(other.ptr_.get());
  }

  bool operator!=(const iterator_wrapper &other) const {
    return ptr_.get()->operator!=(other.ptr_.get());
  }

  iterator_wrapper &operator++() {
    ptr_.get()->move(1);
    return *this;
  }

  iterator_wrapper &operator++(int) {
    iterator_wrapper tmp(*this);
    ptr_.get()->move(1);
    return tmp;
  }

  iterator_wrapper &operator--() {
    ptr_.get()->move(-1);
    return *this;
  }

  iterator_wrapper &operator--(int) {
    iterator_wrapper tmp(*this);
    ptr_.get()->move(-1);
    return tmp;
  }

  iterator_wrapper &operator+=(std::ptrdiff_t i) {
    ptr_.get()->move(i);
    return *this;
  }

  iterator_wrapper &operator-=(std::ptrdiff_t i) {
    ptr_.get()->move(-i);
    return *this;
  }

  friend iterator_wrapper operator+(const iterator_wrapper &lhs,
                                    std::ptrdiff_t rhs) {
    iterator_wrapper tmp(lhs);
    tmp.ptr_->move(rhs);
    return tmp;
  }

  friend iterator_wrapper operator-(const iterator_wrapper &lhs,
                                    std::ptrdiff_t rhs) {
    iterator_wrapper tmp(lhs);
    tmp.ptr_->move(-rhs);
    return tmp;
  }

  friend std::ptrdiff_t operator-(const iterator_wrapper &lhs,
                                  const iterator_wrapper &rhs) {
    return lhs.ptr_.get()->operator-(rhs.ptr_.get());
  }

  friend iterator_wrapper operator+(std::ptrdiff_t lhs,
                                    const iterator_wrapper &rhs) {
    return rhs + lhs;
  }

  friend iterator_wrapper operator-(std::ptrdiff_t lhs,
                                    const iterator_wrapper &rhs) {
    return rhs - lhs;
  }
};

template <typename Base> class Constructor {
public:
  template <typename T>
  static iterator_wrapper<Base> make_wrapper(const T &iter) {
    return iter;
  }

  class Range {
  private:
    const iterator_wrapper<Base> begin_;
    const iterator_wrapper<Base> end_;

  public:
    template <typename T>
    Range(const T &begin, const T &end) : begin_(begin), end_(end) {}
    const auto &begin() const { return begin_; }
    const auto &end() const { return end_; }
    auto size() const { return end() - begin(); }
  };
};

} // namespace AbClass
