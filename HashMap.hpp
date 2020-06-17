#include <vector>

#define START_CAPACITY 16
#define HIGHER_BOUND_FACTOR 0.75
#define LOWER_BOUND_FACTOR 0.25

/**
 * an Exception class, derieves from std::exception
 */
class hashExceptions : public std::exception
{
public:
    /**
     *
     * @param details
     */
    hashExceptions(const std::string &details) : _details(details)
    {}

    /**
     *
     * @return
     */
    const char *what() const noexcept override
    {
        return _details.c_str();
    }

private:
    std::string _details;
};


template<typename KeyT, typename ValueT>
using bucket = std::vector<std::pair<KeyT, ValueT>>;

/**
 *
 * @tparam KeyT
 * @tparam ValueT
 */
template<typename KeyT, typename ValueT>
class HashMap
{
public:
    /**
     * default ctor
     */
    HashMap() : _capacity(START_CAPACITY), _size(0), _table(new bucket<KeyT, ValueT>[START_CAPACITY]),
                _load_factor((double) _size / _capacity)
    {}

    /**
     * specific ctor, create hash-map and inserts one element
     * @param KeyT
     * @param ValueT
     */
    HashMap(const std::vector<KeyT> &keys, const std::vector<ValueT> &values) : _capacity(START_CAPACITY), _size(0),
                                                                                _table(new bucket<KeyT, ValueT>[START_CAPACITY]),
                                                                                _load_factor((double) _size / _capacity)
    {
        //check the size of the two vectors
        if (keys.size() != values.size())
        {
            throw hashExceptions("Error: Tried to create a hashMap, number of key's and values don't match \n");
        }
        int new_size = keys.size();
        for (int i = 0; i < new_size; ++i)
        {
            if (!insert(keys[i], values[i]))
            {
                this->operator[](keys[i]) = values[i];
            }
        }
    }

    /**
     * copy ctor
     * @param other
     */
    HashMap(const HashMap &other) : _capacity(START_CAPACITY), _size(0),
                                    _table(new bucket<KeyT, ValueT>[START_CAPACITY]),
                                    _load_factor((double) _size / _capacity)
    {
        for (auto it = other.begin(); it != other.end(); ++it)
        {
            insert(it->first, it->second);
        }
    }


    /**
     * dtor - must implement since there's a copy ctor
     */
    ~HashMap()
    {
        delete[](_table);
    }

    /**
     *
     * @return the number of elements in the table
     */
    int size() const
    {
        return _size;
    }

    /**
     *
     * @return the capacity of the table
     */
    int capacity() const
    {
        return _capacity;
    }

    /**
     *
     * @return true iff size = 0
     */
    bool empty() const
    {
        return _size == 0;
    }

    /**
     *
     * @param key the key to map to the table
     * @param value the value to map to the table
     * @return true if the insertion suceeded
     */
    bool insert(const KeyT &key, const ValueT &value)
    {
        if (containsKey(key))
        {
            return false;
        }
        //check if need to rehash
        _size += 1;
        _load_factor = (double) _size / capacity();
        if (getLoadFactor() >= HIGHER_BOUND_FACTOR)
        {
            _reSize();
        }
        //calculate index
        int index = _hash(key);
        std::pair<KeyT, ValueT> pair = std::make_pair(key, value);
        _table[index].push_back(pair);
        return true;
    }

    /**
     *
     * @param key the key to search for
     * @return true iff the key was found in the table
     */
    bool containsKey(const KeyT &key) const
    {
        int index = _hash(key);
        for (auto it = _table[index].begin(); it != _table[index].end(); ++it)
        {
            if (it->first == key)
            {
                return true;
            }
        }
        return false;
    }

    /**
     * at function for a const instance
     * @param key the key to search for
     * @return the value that the key matches
     */
    const ValueT &at(const KeyT &key) const
    {
        int index = _hash(key);
        for (auto it = _table[index].begin(); it != _table[index].end(); ++it)
        {
            if (it->first == key)
            {
                return const_cast<ValueT &>(it->second);
            }
        }
        throw hashExceptions("in method at(): key not found");
    }

    /**
    *at function for a non const instance
    * @param key the key to search for
    * @return the value that the key matches
    */
    ValueT &at(const KeyT &key)
    {
        int index = _hash(key);
        for (auto it = _table[index].begin(); it != _table[index].end(); ++it)
        {
            if (it->first == key)
            {
                return const_cast<ValueT &>(it->second);
            }
        }
        throw hashExceptions("in method at(): key not found");
    }

    /**
     *
     * @return true if erasing the key succeeded
     */
    bool erase(const KeyT &key)
    {
        if (!containsKey(key))
        {
            return false;
        }
        int index = _hash(key);
        auto it = _table[index].begin();
        for (; it != _table[index].end(); ++it)
        {
            if (it->first == key)
            {
                _table[index].erase(it);
                _size -= 1;
                break;
            }
        }
        _load_factor = (double) _size / _capacity;
        if (_load_factor <= LOWER_BOUND_FACTOR)
        {
            _reSize();
        }
        return true;
    }

    /**
     *
     * @return the load factor of the table : size/capacity
     */
    double getLoadFactor() const
    {
        return _load_factor;
    }

    /**
     *
     * @param key
     * @return the length of the bucket in the index that the given key is found in
     */
    int bucketSize(const KeyT &key) const
    {
        int index = bucketIndex(key);
        return _table[index].size();
    }

    /**
     *
     * @param key
     * @return the index of the table that the key is fouKeyT,ValueTnd in
     */
    int bucketIndex(const KeyT &key) const
    {
        if (!containsKey(key))
        {
            throw hashExceptions("Exception from bucketIndex - searched for index of non known key");
        }
        return _hash(key);
    }

    /**
     * clears the table, the capacity doesn't change
     */
    void clear()
    {
        if (_size == 0)
        {
            return;
        }
        bucket<KeyT, ValueT> *temp = _table;
        _table = new bucket<KeyT, ValueT>[_capacity];
        delete[] (temp);
        _size = 0;
        _load_factor = (double) _size / _capacity;
    }

    /**
     * iterator for HashMap, iterates on bucket<KeyT,ValueT>*
     */
    class const_iterator
    {

    public:
        typedef int difference_type;
        typedef std::pair<KeyT, ValueT> value_type;
        typedef value_type *pointer;
        typedef value_type &reference;
        typedef std::forward_iterator_tag iterator_category;

        /**
         *
         * @param hashMap
         */
        const_iterator(const HashMap<KeyT, ValueT> *hashMap) : _hashMap(hashMap), _bucket_index(0), _index_in_bucket(0)
        {
            while (_hashMap->_table[_bucket_index].empty())
            {
                ++_bucket_index;
            }
        }

        /**
         *
         * @return
         */
        const std::pair<KeyT, ValueT> &operator*() const
        {
            return _hashMap->_table[_bucket_index][_index_in_bucket];
        }

        /**
         *
         * @return
         */
        std::pair<KeyT, ValueT> *operator->() const
        {
            return &_hashMap->_table[_bucket_index][_index_in_bucket];
        }

        /**
         *
         * @return
         */
        const_iterator operator++()
        {
            //search for the next pair:
            //check if the next pair is in the bucket -
            if ((int) _hashMap->_table[_bucket_index].size() > (int) _index_in_bucket + 1)
            {
                _index_in_bucket += 1;

            }
            else  // look for next index that isn't empty
            {
                _bucket_index += 1;
                for (int i = _bucket_index; i < _hashMap->capacity(); i++)
                {
                    if (!_hashMap->_table[i].empty())
                    {
                        _bucket_index = i;
                        _index_in_bucket = 0;
                        break;
                    }
                }

            }
            return *this;
        }

        /**
         *
         * @return
         */
        const_iterator operator++(int)
        {
            const_iterator tmp = *this;
            //search for the next pair:
            //check if the next pair is in the bucket -
            if (_hashMap->_table[_bucket_index].size() > _index_in_bucket + 1)
            {
                _index_in_bucket += 1;
            }
            else  // look for next index that isn't empty
            {
                for (int i = _bucket_index + 1; i < _hashMap->_table->capacity(); ++i)
                {
                    if (!_hashMap->_table[i].empty())
                    {
                        _bucket_index = i;
                        _index_in_bucket = 0;
                        break;
                    }
                }
            }
            return tmp;
        }

        /**
         *
         * @param other
         * @return
         */
        bool operator==(const const_iterator &other) const
        {
            return _index_in_bucket == other._index_in_bucket &&
                   _bucket_index == other._bucket_index; //_hashMap == other._hashMap &&
        }

        /**
         *
         * @param other
         * @return
         */
        bool operator!=(const const_iterator &other) const
        {
            return _index_in_bucket != other._index_in_bucket ||
                   _bucket_index != other._bucket_index; //_hashMap == other._hashMap ||
        }

        /**
         *
         * @param i
         */
        void setBucketIndex(int i)
        {
            _bucket_index = i;
        }

        /**
         *
         * @return
         */
        int getBucketIndex()
        {
            return _bucket_index;
        }

        /**
         *
         * @param i
         */
        void setIndexInBucket(int i)
        {
            _index_in_bucket = i;
        }

        /**
         *
         * @return
         */
        int getIndexInBucket()
        {
            return _index_in_bucket;
        }

    private:
        const HashMap *_hashMap;
        int _bucket_index;
        int _index_in_bucket;
    };

    /**
     *
     * @return forward const iterator for the hashMap - at the first pair
     */
    const_iterator begin() const
    {
        const_iterator it(this);
        for (int j = 0; j < capacity(); ++j)
        {
            if (!_table[j].empty())
            {
                it.setBucketIndex(j);
                it.setIndexInBucket(0);
                break;
            }
        }
        return it;
    }

    /**
     *
     * @return forward const iterator for the hashMap - at the last pair
     */
    const_iterator end() const
    {
        const_iterator it(this);
        int i = 0;
        while (i != _size)
        {
            ++it;
            ++i;
        }
        return it;
    }

    /**
    *
    * @return forward const iterator for the hashMap - at the first pair - const
    */
    const_iterator cbegin() const
    {
        return begin();
    }

    /**
    *
    * @return forward const iterator for the hashMap - at the last pair - const
    */
    const_iterator cend() const
    {
        return end();
    }

    /**
     * @param key - key witch is in the table
     * @return the value that matches the key
     */
    ValueT &operator[](const KeyT &key) noexcept
    {
        if (containsKey(key))
        {
            return at(key);
        }
        else //if the key isn't in the HashMap - make newpair with the given key and add it
        {
            insert(key, ValueT());
            return (at(key));
        }
    }


    /**
     *
     * @param key - key witch is in the table
     * @return the value that matches the key
     */
    const ValueT &operator[](const KeyT &key) const noexcept
    {
        if (containsKey(key))
        {
            return at(key);
        }
        else //if the key isn't in the HashMap - make newpair with the given key and add it
        {
            return ValueT();
        }
    }

    /**
     *
     * @param other hashmap to compare
     * @return true iff the other hashtable matches this hashtable
     */
    bool operator==(const HashMap &other) const
    {
        if (size() != other.size() || capacity() != other.capacity() || getLoadFactor() != other.getLoadFactor())
        {
            return false;
        }
        auto it1 = begin();
        auto it2 = other.begin();
        for (; it1 != end(); ++it1, ++it2)
        {
            if (*it1 != *it2)
            {
                return false;
            }
        }
        return true;
    }

    /**
     *
     * @param other hashmap to compare
     * @return true iff the other hashtable doesn't matches this hashtable
     */
    bool operator!=(const HashMap &other) const
    {
        return !(*this == other);
    }

    /**
     *
     * @param other
     * @return
     */
    HashMap &operator=(HashMap other)
    {
        std::swap(_table, other._table);
        std::swap(_capacity, other._capacity);
        std::swap(_load_factor, other._load_factor);
        std::swap(_size, other._size);
        return *this;
    }

private:
    int _capacity;
    int _size;
    bucket<KeyT, ValueT> *_table;
    double _load_factor;

    /**
     *
     * @param key
     * @return
     */
    int _hash(const KeyT &key) const
    {
        return std::hash<KeyT>{}(key) & (_capacity - 1);
    }

    /**
     *
     */
    void _reSize()
    {
        int oldCap = _capacity;
        if (getLoadFactor() > HIGHER_BOUND_FACTOR)
        {
            //make new table times 2 bigger
            _capacity *= 2;
        }
        else //make new table times 2 smaller
        {
            _capacity /= 2;
        }
        auto *temp = new bucket<KeyT, ValueT>[_capacity];
        for (int i = 0; i < oldCap; ++i)
        {
            if (!_table[i].empty())
            {
                for (auto it = _table[i].begin(); it != _table[i].end(); ++it)
                {
                    int new_index = _hash(it->first);
                    temp[new_index].push_back(*it);
                }
            }
        }
        delete[](_table);
        _table = temp;
    }
};
