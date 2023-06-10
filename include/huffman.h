#pragma once 

#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <stddef.h>
#include <cassert>


namespace Huffman{

using seq_size_t = uint32_t;
using byte_t = uint8_t;

struct HuffmanException{
    std::string message;
    HuffmanException(const std::string& message){
        this->message = message;
    }
};


class bit_oseq{
public:
    bit_oseq(std::ostream& os);

    void write(bool value);

    void flush();

    void destroy();

    seq_size_t size();

    ~bit_oseq();

private:
    void write_size();

    void next_byte();

    std::ostream* _os;
    std::streampos _begin;
    seq_size_t _size;
    byte_t _byte;
    int _offset;
};


class bit_iseq{
public:
    bit_iseq(std::istream& is);

    bool read();
    
    bool end_of_seq();

    seq_size_t size();

private:
    void next_byte();

    std::istream& _is;
    seq_size_t _size;
    std::size_t _pos;
    byte_t _byte;
    int _offset;
};




class HuffmanTree{
public:
    HuffmanTree(){}

    /*
    Строит дерево Хаффмана.
    Принимает на вход используемые символы и их частоты
    */
    HuffmanTree(const std::map<char, double>& m);


    // Кодирует сообщение m и записывает результат в cm
    void encode(std::istream& src, std::ostream& dst);

    // Декодирует сообщение cm и записывает результат в m 
    void decode(std::istream& src, std::ostream& dst);


    // Алгоритм создания дерева. Принимает на вход используемые символы и их частоты
    void construct(const std::map<char, double>& m);

    
    // Запись в файл и чтение из файла в бинарном виде  
    void save(std::ostream& dst);
    void load(std::istream& src);

    std::size_t additional_data_size();

    bool operator==(const HuffmanTree& t) const;

protected:
    /*
    Узел или лист в префиксном дереве.
    Все они хранятся в векторе nodes.
    Они не содержат никаких указателей, ссылки на другие узлы
    содержатся в виде их индексов в векторе nodes.
    Таким образом, содержимое этого дерева можно прямо записать в файл.
    */
    struct Node{
        uint16_t i0; // индекс следующего узла в векторе nodes, если новый бит в коде символа равен 0 (i0 = -1, если такого нет)
        uint16_t i1; // индекс следующего узла в векторе nodes, если новый бит в коде символа равен 1 (i1 = -1, если такого нет)
        uint16_t ip; // индекс родительского узла в векторе nodes, -1 если данный узел корень
        bool v;      // это правый или левый потомок родителя?
        char symb; // если i1 = i2 = -1, то этот узел - лист, и symb - символ в нём 

        bool is_leaf();
        uint16_t next_node_index(bool bit);
        bool operator==(const Node& n) const;
    };


    /*
    Construct Node - эта структура используется исключительно для
    алгоритма построения префиксного дерева.
    */
    struct CNode{
        uint16_t index; // Ссылка на Node в массиве nodes
        float p; // Частота встречи символов, у которых путь по дереву от корня содержит соответствующий узел

        bool operator<(const CNode& other) const { return p < other.p; }
        CNode(uint16_t index, float p): p(p), index(index) {}
    };

    Node& find_leaf_with_symbol(char symb);

    /*
    Узлы дерева Хаффмана. Корневой узел всегда последний, листья идут в начале.
    Если число листьев N, то nodes.size() = 2*N-1.
    */
    std::vector<Node> nodes;
};


// Подсчитывает количества всех входящих в текст src символов. Осталяет курсор потока на месте.
std::map<char, double> counts(std::istream& src);

// Сжимает информацию. Возвращает объём дополнительных данных
std::size_t encode(std::istream& src, std::ostream& dst);

// Разжимает информацию. Возвращает объём дополнительных данных
std::size_t decode(std::istream& src, std::ostream& dst);

}
