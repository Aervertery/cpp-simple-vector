#pragma once

#include <cassert>
#include <initializer_list>
#include <stdexcept>
#include <algorithm>
#include <utility>
#include "array_ptr.h"

class ReserveProxyObj {
public:
    ReserveProxyObj(size_t capacity) :
        size_(capacity)
    {
    }

    size_t GetSize() {
        return size_;
    }

private:
    size_t size_;
};

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) :
        elements(size),
        size_(size),
        capacity_(size) {
        std::fill(begin(), end(), Type{});
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value) :
        elements(size),
        size_(size),
        capacity_(size) {
        ArrayPtr<Type> elements(size);
        size_ = size;
        capacity_ = size;
        std::fill(begin(), end(), value);
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) :
        elements(init.size()),
        size_(init.size()),
        capacity_(init.size()) {
        int i = 0;
        for (const Type elem : init) {
            elements[i] = elem;
            ++i;
        }
    }

    SimpleVector(const SimpleVector& other) : SimpleVector(other.size_) {
        SimpleVector tmp(other.size_);
        tmp.capacity_ = other.capacity_;
        int i = 0;
        /*for (const Type elem : std::move(const_cast<SimpleVector&>(other))) {
            tmp[i] = std::move(const_cast<Type&>(elem));
            ++i;
        }*/
        for (auto it = other.begin(); it != other.end(); ++it, ++i) {
            tmp[i] = std::move(const_cast<Type&>(*it));
            //++i;
        }
        //std::move(std::make_move_iterator(other.begin()), std::make_move_iterator(other.end()), begin());
        //swap(tmp);
        std::move(tmp.begin(), tmp.end(), begin());
    }

    SimpleVector(ReserveProxyObj obj) :
        elements(obj.GetSize()),
        capacity_(obj.GetSize())
    {
    }

    SimpleVector(SimpleVector&& other) noexcept :
        elements(other.size_),
        size_(other.size_),
        capacity_(other.capacity_) {
        std::move(other.begin(), other.end(), begin());
        other.Clear();
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        return elements[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        return elements[index];
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            SimpleVector copy_rhs(rhs);
            swap(copy_rhs);
        }
        return *this;
    }

    SimpleVector& operator=(SimpleVector&& rhs) noexcept {
        if (this != &rhs) {
            elements.swap(rhs.elements);
            //std::move(rhs.begin(), rhs.end(), begin());
            size_ = rhs.size_;
            capacity_ = rhs.capacity_;
            rhs.Clear();
        }
        return *this;
    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type& item) {
        if (size_ == capacity_) {
            //size_t old_size = size_;
            size_t new_capacity = capacity_ == 0 ? 1 : capacity_ * 2;
            //Resize(new_capacity);
            capacity_ = new_capacity;
            ArrayPtr<Type> tmp(new_capacity);
            std::move(begin(), end(), tmp.Get());
            elements.swap(tmp);
        }
        elements[size_] = std::move(const_cast<Type&>(item));
        ++size_;
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        size_t distance = pos - begin();
        if (size_ == capacity_) {
            capacity_ = capacity_ == 0 ? 1 : capacity_ * 2;
        }
        ArrayPtr<Type> new_array(capacity_);
        //std::copy(std::make_move_iterator(begin()), std::make_move_iterator(end()), new_array.Get());
        //std::copy_backward(std::make_move_iterator(new_array.Get() + distance), std::make_move_iterator(new_array.Get() + size_), new_array.Get() + size_ + 1);
        std::move(begin(), end(), new_array.Get());
        std::move(new_array.Get() + distance, new_array.Get() + size_, new_array.Get() + distance + 1);
        new_array[distance] = std::move(const_cast<Type&>(value));
        elements.swap(new_array);
        ++size_;
        return begin() + distance;
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        if (!IsEmpty()) {
            --size_;
        }
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        size_t distance = pos - begin();
        size_t result = distance;
        for (auto it = pos + 1; it != end(); ++it) {
            elements[distance] = std::move(const_cast<Type&>(*it));
            ++distance;
        }
        /*ArrayPtr<Type> tmp;
        std::merge(begin(), begin() + distance, begin() + distance + 1, end(), tmp.Get());
        elements.swap(tmp);*/
        --size_;
        return begin() + result;
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        elements.swap(other.elements);
        size_t old_size = size_;
        size_t old_capacity = capacity_;

        size_ = other.size_;
        capacity_ = other.capacity_;

        other.size_ = old_size;
        other.capacity_ = old_capacity;
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        if (size_ != 0) {
            return false;
        }
        return true;
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= GetSize()) {
            throw std::out_of_range("too much");
        }
        return elements[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= GetSize()) {
            throw std::out_of_range("too much");
        }
        return elements[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity <= capacity_) {
            return;
        }
        ArrayPtr<Type> new_array(new_capacity);
        //std::copy(std::make_move_iterator(begin()), std::make_move_iterator(end()), new_array.Get());
        std::move(begin(), end(), new_array.Get());
        new_array.swap(elements);
        capacity_ = new_capacity;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (new_size <= size_) {
            size_ = new_size;
        }
        else if (new_size <= capacity_) {
            //size_t dist = new_size - size_;
            //ArrayPtr<Type> tmp(dist);
            //SimpleVector<Type> tmp(dist);
            //std::fill(begin() + size_, begin() + new_size, Type{});
            /*for (size_t i = size_; i < new_size; ++i) {
                Type value = Type{};
                elements[i] = std::move(value);
            }*/
            fill_vector(begin() + size_, begin() + new_size);
            //std::move(tmp.Get(), tmp.Get() + dist, begin() + size_);
            //std::move(tmp.begin(), tmp.end(), begin() + size_);
            size_ = new_size;
        }
        else {
            ArrayPtr<Type> new_array(new_size);
            //std::copy(std::make_move_iterator(begin()), std::make_move_iterator(end()), new_array.Get());
            std::move(begin(), end(), new_array.Get());
            //std::fill(std::make_move_iterator(it), std::make_move_iterator(new_array.Get() + new_size), std::move(Type{}));
            fill_vector(new_array.Get() + size_, new_array.Get() + new_size);
            new_array.swap(elements);
            size_ = new_size;
            capacity_ = new_size;
        }
    }

    void fill_vector(Iterator start, Iterator end) {
        for (auto it = start; it != end; ++it) {
            Type value = Type{};
            *it = std::move(value);
        }
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return elements.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return elements.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return elements.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return elements.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return elements.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return elements.Get() + size_;
    }
private:
    ArrayPtr<Type> elements;
    size_t size_ = 0;
    size_t capacity_ = 0;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return lhs < rhs || lhs == rhs;
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs <= rhs);
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}