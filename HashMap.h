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

1. HashMapa jest tablica wskazan na vectory(typu unique_ptr) - w koncu tylko tam ich uzywamy
2. Kubelek to std::vector, ktory przechowuje elementy (pary: klucz + wartosc przechowywana)
3. Allokacja pary(klucz,wartosc) w haszmapie odbywa się poprzez wyliczenie hasza z klucza
    wybranie z tablicy wektora o indeksie hasha, oraz zapisanie do niego elementu
4. Jesli w kubelku o danym hashu nie ma zadnych elementow, to unique_ptr jest niezainicjalizowany
5. Analogicznie - jesli jakis element jest usuwany, i w kubelku w ktorym sie znajdowal juz nic nie ma - kasujemy unique_ptr
6. Usuwanie z HashMapy: podajemy klucz, wyliczymy hasha, i badamy ktory element w kubełku o podanym hashu ma szukany klucz
7. Wyszukiwanie w HashMapie: jak wyżej, tyle, że zamiast usuwac z wektora wuszykany element to go zwracamy

*/


namespace aisdi
{


template <typename KeyType, typename ValueType>
class HashMap
{
public:
  using key_type = KeyType;
  using mapped_type = ValueType;
  using value_type = std::pair<const key_type, mapped_type>;
  using pair = std::pair<const key_type, mapped_type>;
  using size_type = std::size_t;
  using reference = value_type&;
  using const_reference = const value_type&;
  //using std::hash<KeyType> = hash;
  using Bucket = std::vector<pair>;
  using unique_ptr = std::unique_ptr<std::vector<pair> >;


  class ConstIterator;
  class Iterator;
  using iterator = Iterator;
  using const_iterator = ConstIterator;
private:

    std::vector<unique_ptr> hash_table;//glowna tablica hashmapy, przechowujaca wskazania na kubelki
    std::hash<KeyType> hash_function;
    const size_t table_lenght;
    const size_t number_of_elem;

    size_t generateHash(const KeyType& key)
    {
        return hash_function(key)%table_lenght;
    }
    mapped_type& findElem(const KeyType key, unique_ptr pointer)//podajemy klucz i wskaznik na kubelek
    {
        auto iter = pointer -> begin();
        auto iter_end = pointer ->end();
        for(; iter != iter_end; ++iter)
        {
         if( (*iter).first == key )
         {
            return (*iter).second;
         }
        }

        return nullptr;
    }
    typename std::vector<std::pair<KeyType, ValueType> >::const_iterator findIterToElem(const KeyType key, unique_ptr pointer)//podajemy klucz i wskazanie na kubelek
    {
        auto iter = pointer ->begin();
        auto iter_end = pointer ->end();
        for(; iter != iter_end;++iter)
        {
            if( (*iter).first == key )
                return iter;
        }
        return iter_end();
    }

public:
  HashMap(size_t map_lenght = 10000) : hash_table(map_lenght), hash_function(), table_lenght(map_lenght), number_of_elem(0)
  {}

  HashMap(std::initializer_list<value_type> list)
  {
    (void)list; // disables "unused argument" warning, can be removed when method is implemented.
    throw std::runtime_error("TODO");
  }

  HashMap(const HashMap& other)
  {
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
  }

  HashMap(HashMap&& other)
  {
    table_lenght = other.table_lenght;
    number_of_elem = other.number_of_elem;
    hash_table = std::move(other.hash_table);//byc moze nalezalo by przeniesc po kolei kazdy unique_ptr

    other.table_lenght = 0;
    other.number_of_elem = 0;
    other.hash_table.resize(0);
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
    hash_table = std::move(other.hash_table);//byc moze trzeba po kolei przenosic unique_ptry

    other.table_lenght = 0;
    other.number_of_elem = 0;
    other.hash_table.resize(0);
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
    size_t hashed_key = generateHash(key);
    if(hash_table[hashed_key] == nullptr)//jezeli nie stworzono jeszcze kubelka - to go tworzymy
        hash_table[hashed_key] = unique_ptr(new Bucket);
    auto iter = hash_table[hashed_key] -> begin();
    auto iter_end = hash_table[hashed_key] ->end();
    for(; iter != iter_end; ++iter)
    {
        if((*iter).first == key)
        {
            return (*iter).second;//referencje do elementu przechowywanego
        }
    }
    //nalezy rozszerzyc kubelem o jedno miejsce
    size_t lenght = hash_table[hashed_key] ->size();
    hash_table[hashed_key] -> resize(lenght + 1);
    iter = hash_table[hashed_key] ->end();
    --iter;
    return (*iter).second;
    //zwravamy ostatni element - w poprzsedniej linijce stworzony



    }

  const mapped_type& valueOf(const key_type& key) const
  {
    size_t hashed_key = generateHash(key);
    if(hash_table[hashed_key] == nullptr)
        throw std::out_of_range("HashMap::valueOf(Key) - Object of this key doesn't exist");

    const mapped_type& returned = findElem(key);
    if( returned == nullptr)
        throw std::out_of_range("HashMap::valueOf(Key) - Object of this key doesn't exist");

    return returned;

  }

  mapped_type& valueOf(const key_type& key)
  {
    size_t hashed_key = generateHash(key);
    if(hash_table[hashed_key] == nullptr)
        throw std::out_of_range("HashMap::valueOf(Key) - Object of this key doesn't exist");

    mapped_type& returned = findElem(key);
    if( returned == nullptr)
        throw std::out_of_range("HashMap::valueOf(Key) - Object of this key doesn't exist");

    return returned;

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
    size_t hashed_key = generateHash(key);
    if(hash_table[hashed_key] == nullptr)//jesli nie ma kubelka o danym kluczu
        throw std::out_of_range("HashMap::remove(Key) - Object of this key doesn't exist");
    auto iter = findIterToElem(key);
    if(iter == hash_table[hashed_key] -> end())
        throw std::out_of_range("HashMap::remove(Key) - Object of this key doesn't exist");
    hash_table[hashed_key] ->erase(iter);//kasujemy element o naszym kluczu

    //jesli wektor nie ma zadnych elementow - kasujemy unique_ptra do niego
    if(hash_table[hashed_key] ->size() == 0)
    {
        hash_table[hashed_key].reset();
    }
  }

  void remove(const const_iterator& it)
  {
    (void)it;
    throw std::runtime_error("TODO");
  }

  size_type getSize() const
  {
    return table_lenght;
  }
  size_type getNumberElem() const
  {
    return number_of_elem;
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



private:
    HashMap<KeyType, ValueType> *map_ptr;
    size_t bucket_num;
    size_t in_bucket_index;


public:
  using reference = typename HashMap::const_reference;
  using iterator_category = std::bidirectional_iterator_tag;
  using value_type = typename HashMap::value_type;
  using pointer = const typename HashMap::value_type*;

  explicit ConstIterator()
  {}

  ConstIterator(const ConstIterator& other): map_ptr(other.map_ptr),  bucket_num(other.bucket_num), in_bucket_index(other.in_bucket_index)
  {}

  ConstIterator& operator=(const ConstIterator &other)
  {
    if(&other == this)
        return *this;
    map_ptr = other.map_ptr;
    bucket_num = other.bucket_num;
    in_bucket_index = other.in_bucket_index;
    return *this;
  }

  ConstIterator& operator++()
  {
  throw std::runtime_error("TODO");
    if(bucket_num + 1 == map_ptr->table_lenght  &&  in_bucket_index + 1 == (map_ptr->hash_table[bucket_num]).size() )//gdy jestesmy w ostatnim kubelku i jego ostatnim elemencie
        throw std::out_of_range("HashMap::ConstIterator::operator++() - iter went out of range");

    if(in_bucket_index + 1 == (map_ptr->hash_table[bucket_num]).size()) //gdy jestesmy na koncu kubelka
    {
        ++bucket_num;
        in_bucket_index = 0;
        return *this;
    }

    ++in_bucket_index;
    return *this;
  }


  ConstIterator operator++(int)
  {
  throw std::runtime_error("TODO");
    ConstIterator temp = *this;
    ++*this;//to moze zglaszac wyjatek gdy wyjdzie poza zakres - patrz operator++()

    return temp;
  }

  ConstIterator& operator--()
  {

  throw std::runtime_error("TODO");
    if(bucket_num == 0  && in_bucket_index == 0)
        throw std::out_of_range("HashMapa::ConstIterator::operator-- : Iterator went out of range");
    if(in_bucket_index == 0)
    {
        --bucket_num;
        in_bucket_index = (map_ptr->hash_table[bucket_num]).size() - 1;//ostatni element w tym kubelku
        return *this;
    }

    --in_bucket_index;
    return *this;
  }

  ConstIterator operator--(int)
  {

  throw std::runtime_error("TODO");
    ConstIterator temp = *this;
    --*this;//to zglasza wyjatek, gdy wyjdzie poz zakres - patrz operator--()
    return temp;
  }

  reference operator*() const //reference - std::pair
  {
    return (map_ptr->hash_table[bucket_num])[in_bucket_index];
  }

  pointer operator->() const
  {
    return &this->operator*();
  }

  bool operator==(const ConstIterator& other) const
  {
    if(map_ptr == other.map_ptr && bucket_num == other.bucket_num && in_bucket_index == other.in_bucket_index)
        return true;
    else
        return false;
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

  Iterator& operator=(const Iterator& other)
  {
    return ConstIterator::operator=(other);
  }

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
