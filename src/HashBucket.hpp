#ifndef HASH_BUCKET_H_
#define HASH_BUCKET_H_

#include "HashNode.hpp"
#include <shared_mutex>

namespace ds
{
    template <typename K, typename V> class HashBucket
    {
    public:
        HashBucket()
        {
        }

        ~HashBucket() // delete the bucket
        {
            clear();
        }

        bool find(const K& key, V& value) const
        {
            // A shared mutex is used to enable mutiple concurrent reads
            std::shared_lock lock(mutex_);
            HashNode<K, V>* node = head;

            while (node != nullptr)
            {
                if (node->getKey() == key)
                {
                    value = node->getValue();
                    return true;
                }
                node = node->next;
            }
            return false;
        }

        void insert(const K& key, const V& value)
        {
            // Exclusive lock to enable single write in the bucket
            std::unique_lock lock(mutex_);
            HashNode<K, V>* prev = nullptr;
            HashNode<K, V>* node = head;

            while (node != nullptr && node->getKey() != key)
            {
                prev = node;
                node = node->next;
            }

            if (nullptr == node) // New entry, create a node and add to bucket
            {
                if (nullptr == head)
                {
                    head = new HashNode<K, V>(key, value);
                }
                else
                {
                    prev->next = new HashNode<K, V>(key, value);
                }
            }
            else
            {
                node->setValue(value); // Key found in bucket, update the value
            }
        }

        void erase(const K& key)
        {
            // Exclusive lock to enable single write in the bucket
            std::unique_lock lock(mutex_);
            HashNode<K, V>* prev = nullptr;
            HashNode<K, V>* node = head;

            while (node != nullptr && node->getKey() != key)
            {
                prev = node;
                node = node->next;
            }

            if (nullptr == node) // Key not found, nothing to be done
            {
                return;
            }
            else // Remove the node from the bucket
            {
                if (head == node)
                {
                    head = node->next;
                }
                else
                {
                    prev->next = node->next;
                }
                delete node; // Free up the memory
            }
        }

        void clear()
        {
            std::unique_lock lock(mutex_);
            HashNode<K, V>* prev = nullptr;
            HashNode<K, V>* node = head;
            while (node != nullptr)
            {
                prev = node;
                node = node->next;
                delete prev;
            }
            head = nullptr;
        }

    private:
        HashNode<K, V>* head = nullptr;         // The head node of the bucket
        mutable std::shared_timed_mutex mutex_; // The mutex for this bucket
    };
}

#endif