#pragma once

#include <iostream>

#define LCHILD(x) 2 * x + 1
#define RCHILD(x) 2 * x + 2
#define PARENT(x) (x - 1) / 2

namespace ds
{
    template<class T>
    class MinHeap {
    public:
        MinHeap() = default;
        MinHeap(const T* arr, const size_t size) { build(arr, size); };
        ~MinHeap() { destroy(); };

        void build(const T* arr, const size_t size)
        {
            int i = NULL;

            // Insertion into the heap without violating the shape property
            for (i = 0; i < size; i++) {
                if (this->size) {
                    this->elem = (T*)realloc((void*)this->elem, (this->size + 1) * sizeof(T));
                }
                else {
                    this->elem = (T*)malloc(sizeof(T));
                }
                this->elem[(this->size)++] = arr[i];
            }

            // Making sure that heap property is also satisfied
            for (i = (this->size - 1) / 2; i >= 0; i--) {
                heapify(i);
            }
        }

        void insert(const T data)
        {
            if (this->size) {
                this->elem = (T*)realloc(this->elem, (this->size + 1) * sizeof(T));
            }
            else {
                this->elem = (T*)malloc(sizeof(T));
            }

            int i = (this->size)++;
            while (i && data < this->elem[PARENT(i)]) {
                this->elem[i] = this->elem[PARENT(i)];
                i = PARENT(i);
            }

            if(this->elem != nullptr)
                this->elem[i] = data;
        }

        void pop()
        {
            if (this->size) {
                this->elem[0] = this->elem[--(this->size)];
                this->elem = (T*)realloc(this->elem, this->size * sizeof(T));
                heapify(0);
            }
            else {
                free(this->elem);
            }
        }

        bool empty()
        {
            return this->size == 0;
        }

        T front()
        {
            return this->elem[0];
        }

        void destroy() {
            free(this->elem);
        }

    private:
        inline void swap(T* e1, T* e2)
        {
            T tmp = *e1;
            *e1 = *e2;
            *e2 = tmp;
        }

        inline void heapify(int i) {
            int smallest = (LCHILD(i) < this->size && this->elem[LCHILD(i)] < this->elem[i]) ? LCHILD(i) : i;
            if (RCHILD(i) < this->size && this->elem[RCHILD(i)] < this->elem[smallest])
            {
                smallest = RCHILD(i);
            }
            if (smallest != i) {
                swap(&this->elem[i], &this->elem[smallest]);
                heapify(smallest);
            }
        }

    private:
        size_t size;
        T* elem;
    };
}