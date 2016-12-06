#ifndef AISDI_MAPS_HASHMAP_H
#define AISDI_MAPS_HASHMAP_H

#include <cstddef>
#include <initializer_list>
#include <stdexcept>
#include <utility>
#include<memory>
#include<vector>
#include<functional>
/*
Zalozenia projektowe:

1. HashMapa jest tablica wskazan na kubelki(byc moze wskazan typu unique_ptr) - w koncu tylko tam ich uzywamy
2. Kubelek zawiera std::vector, ktory przechowuje elementy (pary: klucz + wartosc przechowywana)
3. Allokacja pary(klucz,wartosc) w haszmapie odbywa się poprzez wyliczenie hasza z klucza
    wybranie z tablicy wektora o indeksie hasha, oraz zapisanie do niego elementu
4. Jesli w kubelku o danym hashu nie ma zadnych elementow, to unique_ptr jest niezainicjalizowany
5. Analogicznie - jesli jakis element jest usuwany, i w kubelku w ktorym sie znajdowal juz nic nie ma - kasujemy unique_ptr
6. Usuwanie z HashMapy: podajemy klucz, wyliczymy hasha, i badamy ktory element w kubełku o podanym hashu ma szukany klucz
7. Wyszukiwanie w HashMapie: jak wyżej, tyle, że zamiast usuwac z wektora wuszykany element to go zwracamy

*/


namespace aisdi
{


template<typename KeyType, typename ValueType>
struct Bucket
{
using pair = std::pair<const KeyType, ValueType>;
using vector = std::vector<pair>;
using iterator = std::vector<pair>::iterator;
//pola

    std::vector<pair> vector_bucket;
//metody
    Bucket(size_t n=1): vector_bucket(n)
    {
        //vector_bucket.resize(n);//nustalamy dlugosc poczatkowa by nie zajmowal za duzo miejsca
    }

    Bucket(const Bucket &other) : vector_bucket(other.vector_bucket)
    {}

    Bucket(const Bucket &&other) : vector_bucket(std::move(other.vector_bucket))
    {}

    Bucket& operator=(const Bucket &other)
    {
        if(&other == this)
            return *this;
        this->vector_bucket = other.vector_bucket;
        return *this;
    }

    Bucket& operator=(const Bucket &&other)
    {
        vector_bucket = std::move(other.vector_bucket);
    }

    vector::iterator& returnIterOnPair(const KeyType &key) const
    {
        vector::iterator iter_found;
        for( iter_found = vector_bucket.begin(); iter_found != vector_bucket.end(); ++iter)
        {
            if( (*iter_found).first == key)
                break;
        }
        return  iter_found;
    }

    void deletePair(const KeyType & key)
    {
        auto iter_found = returnIterOnPair(key);

        if(iter_found != vector_bucket.end())
            vector_bucket.erase(iter_found);
    }

    void insertPair(const pair &elem)
    {
        vector_bucket.push_back(elem);
    }
    bool isEmpty()
    {
        return vector_bucket.empty();
    }
    size_t numberOfElem()
    {
        return vector_bucket.size();
    }


};






template <typename KeyType, typename ValueType>
class HashMap
{
public:
  using key_type = KeyType;
  using mapped_type = ValueType;
  using value_type = std::pair<const key_type, mapped_type>;
  using size_type = std::size_t;
  using reference = value_type&;
  using const_reference = const value_type&;
  //using std::hash<KeyType> = hash;
  using Bucket = aisdi::Bucket<KeyType, ValueType>;
  using unique_ptr = std::unique_ptr<Bucket>;


  class ConstIterator;
  class Iterator;
  using iterator = Iterator;
  using const_iterator = ConstIterator;
private:

    std::vector<unique_ptr > hash_table;//glowna tablica hashmapy, przechowujaca wskazania na kubelki
    std::hash<KeyType> hash_function;
    const size_t table_lenght;
    const size_t number_of_elem;

    size_t generateHash(const KeyType& key)
    {
        return hash_function(key)%table_lenght;
    }


public:
  HashMap(size_t map_lenght = 1024) : table_lenght(map_lenght), number_of_elem(0), hash_table(map_lenght)
  {
    //table_lenght = map_lenght;
    //hash_table.resize(table_lenght, nullptr);//pusta hash_table o zadanej dlugosci

  }

  HashMap(std::initializer_list<value_type> list)
  {
    (void)list; // disables "unused argument" warning, can be removed when method is implemented.
    throw std::runtime_error("TODO");
  }

  HashMap(const HashMap& other)
  {
    table_lenght = other.table_lenght;
    number_of_elem = other.number_of_elem;
    for(size_t i = 0; i < table_lenght; ++i)
    {
        if(other.hash_table[i].get() != nullptr)//jesli other.unique_ptr - zainicjowany
        {
            hash_table[i] = unique_ptr(new Bucket);//przydzielamy pamiec kubelkowi
            *hash_table[i] = *other.hash_table[i];//kopiujemy kubelek
        }
        else
        {
            hash_table[i] = unique_ptr(nullptr);
        }
    }
  }

  HashMap(HashMap&& other)
  {
    table_lenght = other.table_lenght;
    number_of_elem = other.number_of_elem;
    hash_table = std::move(other.hash_table);
    //chyba wszystko
  }

  HashMap& operator=(const HashMap& other)
  {
    if(&other == this)
        return *this;
    table_lenght = other.table_lenght;
    number_of_elem = other.number_of_elem;
    hash_table.resize(table_lenght);
    for(size_t i = 0; i < table_lenght; ++i)
    {
        if(other.hash_table[i].get() != nullptr)//jesli other.unique_ptr - zainicjowany
        {
            hash_table[i] = unique_ptr(new Bucket);//przydzielamy pamiec kubelkowi
            *hash_table[i] = *other.hash_table[i];//kopiujemy kubelek
        }
        else
        {
            hash_table[i] = unique_ptr(nullptr);
        }
    }
    return *this;
  }

  HashMap& operator=(HashMap&& other)
  {
    if(&other == this)
        return *this;
    table_lenght = other.table_lenght;
    number_of_elem = other.number_of_elem;
    hash_table = std::move(other.hash_table);
    return *this;
  }

  bool isEmpty() const
  {
        if(number_of_elem == 0)
            return true;
        else
            return false;
  }

  mapped_type& operator[](const key_type& key)
  {
    size_t hash = hash_function(key);
    if(hash_table[hash] == nullptr)//jezeli nie stworzono jeszcze kubelka - to go tworzymy
        hash_table[hash] = unique_ptr(new Bucket);

    throw std::runtime_error("TODO");

  }

  const mapped_type& valueOf(const key_type& key) const
  {
    (void)key;
    throw std::runtime_error("TODO");
  }

  mapped_type& valueOf(const key_type& key)
  {
    (void)key;
    throw std::runtime_error("TODO");
  }

  const_iterator find(const key_type& key) const
  {
    (void)key;
    throw std::runtime_error("TODO");
  }

  iterator find(const key_type& key)
  {
    (void)key;
    throw std::runtime_error("TODO");
  }

  void remove(const key_type& key)
  {
    (void)key;
    throw std::runtime_error("TODO");
  }

  void remove(const const_iterator& it)
  {
    (void)it;
    throw std::runtime_error("TODO");
  }

  size_type getSize() const
  {
    throw std::runtime_error("TODO");
  }

  bool operator==(const HashMap& other) const
  {
    (void)other;
    throw std::runtime_error("TODO");
  }

  bool operator!=(const HashMap& other) const
  {
    return !(*this == other);
  }

  iterator begin()
  {
    throw std::runtime_error("TODO");
  }

  iterator end()
  {
    throw std::runtime_error("TODO");
  }

  const_iterator cbegin() const
  {
    throw std::runtime_error("TODO");
  }

  const_iterator cend() const
  {
    throw std::runtime_error("TODO");
  }

  const_iterator begin() const
  {
    return cbegin();
  }

  const_iterator end() const
  {
    return cend();
  }
};

template <typename KeyType, typename ValueType>
class HashMap<KeyType, ValueType>::ConstIterator
{
public:
  using reference = typename HashMap::const_reference;
  using iterator_category = std::bidirectional_iterator_tag;
  using value_type = typename HashMap::value_type;
  using pointer = const typename HashMap::value_type*;

  explicit ConstIterator()
  {}

  ConstIterator(const ConstIterator& other)
  {
    (void)other;
    throw std::runtime_error("TODO");
  }

  ConstIterator& operator++()
  {
    throw std::runtime_error("TODO");
  }

  ConstIterator operator++(int)
  {
    throw std::runtime_error("TODO");
  }

  ConstIterator& operator--()
  {
    throw std::runtime_error("TODO");
  }

  ConstIterator operator--(int)
  {
    throw std::runtime_error("TODO");
  }

  reference operator*() const
  {
    throw std::runtime_error("TODO");
  }

  pointer operator->() const
  {
    return &this->operator*();
  }

  bool operator==(const ConstIterator& other) const
  {
    (void)other;
    throw std::runtime_error("TODO");
  }

  bool operator!=(const ConstIterator& other) const
  {
    return !(*this == other);
  }
};

template <typename KeyType, typename ValueType>
class HashMap<KeyType, ValueType>::Iterator : public HashMap<KeyType, ValueType>::ConstIterator
{
public:
  using reference = typename HashMap::reference;
  using pointer = typename HashMap::value_type*;

  explicit Iterator()
  {}

  Iterator(const ConstIterator& other)
    : ConstIterator(other)
  {}

  Iterator& operator++()
  {
    ConstIterator::operator++();
    return *this;
  }

  Iterator operator++(int)
  {
    auto result = *this;
    ConstIterator::operator++();
    return result;
  }

  Iterator& operator--()
  {
    ConstIterator::operator--();
    return *this;
  }

  Iterator operator--(int)
  {
    auto result = *this;
    ConstIterator::operator--();
    return result;
  }

  pointer operator->() const
  {
    return &this->operator*();
  }

  reference operator*() const
  {
    // ugly cast, yet reduces code duplication.
    return const_cast<reference>(ConstIterator::operator*());
  }
};

}

#endif /* AISDI_MAPS_HASHMAP_H */
