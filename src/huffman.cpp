#include "huffman.h"

using namespace Huffman;



bit_oseq::bit_oseq(std::ostream& os): 
    _os(&os), _offset(0), _size(0), _begin(os.tellp()), _byte(0)
{
    write_size();
}

void bit_oseq::write(bool value){
    if(_offset == 8){
        next_byte();
    }
    byte_t mask = 1 << _offset;
    byte_t v = (value << _offset);
    _byte = (_byte & ~mask) | (v & mask);
    _offset += 1;
    _size += 1;
}

void bit_oseq::flush(){
    if(_offset != 0)
        next_byte();
    write_size();
}

void bit_oseq::destroy(){
    flush();
    _os = nullptr;
}

seq_size_t bit_oseq::size(){
    return _size;
}

bit_oseq::~bit_oseq(){
    flush();
}


void bit_oseq::write_size(){
    if(_os == nullptr)
        return;
    _os->seekp(_begin);
    _os->write((char*)&_size, sizeof(_size));
    _os->seekp(0, _os->end);
}

void bit_oseq::next_byte(){
    if(_os == nullptr)
        return;
    _os->write((char*)&_byte, sizeof(_byte));
    _offset = 0;
}




bit_iseq::bit_iseq(std::istream& is): 
    _is(is), _offset(8), _pos(0)
{
    is.read((char*)&_size, sizeof(_size));
    if(is.fail())
        throw "bit_iseq: failed to read size of sequence";
}

bool bit_iseq::read(){
    if(end_of_seq())
        throw "bit_iseq: failed to read - the end of sequence has been reached";
    if(_offset == 8)
        next_byte();
    bool res = (_byte >> _offset) & 1;
    _offset += 1;
    _pos += 1;
    return res;
}

bool bit_iseq::end_of_seq(){
    return _pos >= _size;  
}

seq_size_t bit_iseq::size(){
    return _size;
}


void bit_iseq::next_byte(){
    _is.read((char*)&_byte, 1);
    if(_is.fail())
        throw "bit_iseq: failed to read - wrong sequence format";
    _offset = 0;
}






/*
Строит дерево Хаффмана.
Принимает на вход используемые символы и их частоты
*/
HuffmanTree::HuffmanTree(const std::map<char, double>& m){
    construct(m);
}


// Кодирует сообщение m и записывает результат в cm
void HuffmanTree::encode(std::istream& src, std::ostream& dst){
    bit_oseq bit_seq_dst(dst);
    bool arr[256];  // глубина дерева не превосходит его размера, а размер не больше количества символов - 256
    while(true){
        int j = 0;
        char symb = src.get();
        if(!src.good()){
            src.clear();
            break;
        }
        Node* node = &find_leaf_with_symbol(symb);  // тут может вылететь исключение, что символа нет в дереве
        while(node->ip != (uint16_t)-1){
            arr[j] = node->v;
            node = &nodes[node->ip];
            j++;
        }
        while(j > 0){
            j--;
            bit_seq_dst.write(arr[j]);
        }
    }
}

// Декодирует сообщение cm и записывает результат в m 
void HuffmanTree::decode(std::istream& src, std::ostream& dst){
    try{
        bit_iseq bit_seq_src(src);
        std::size_t i = 0;
        while(!bit_seq_src.end_of_seq()){
            Node* node = &nodes[nodes.size()-1];
            while(!node->is_leaf()){
                node = &nodes[node->next_node_index(bit_seq_src.read())];
                i++;
            }
            assert(node->is_leaf());
            dst.write(&node->symb, 1);
        }
    }
    catch(...){
        throw HuffmanException("data format error");
    }
}


// Алгоритм создания дерева. Принимает на вход используемые символы и их частоты
void HuffmanTree::construct(const std::map<char, double>& m){
    int N = m.size();
    nodes.resize(2*N - 1);
    std::multiset<CNode> cnodes;
    int i = 0;
    auto iter = m.begin();
    while(iter != m.end()){
        nodes[i] = {
            .i0 = (uint16_t)-1,
            .i1 = (uint16_t)-1,
            .symb = iter->first,
        };
        cnodes.insert(CNode(i, iter->second));
        ++iter;
        ++i;
    }
    assert(i == N);
    assert(cnodes.size() == N);
    while(cnodes.size() != 1){
        CNode cn0 = *cnodes.begin();
        CNode cn1 = *(++cnodes.begin());
        cnodes.erase(cnodes.begin());
        cnodes.erase(cnodes.begin());

        Node& new_node = nodes[i];
        Node& node0 = nodes[cn0.index];
        Node& node1 = nodes[cn1.index];

        new_node = {
            .i0 = cn0.index,
            .i1 = cn1.index,
            .ip = (uint16_t)-1,
        };

        node0.ip = (uint16_t)i,
        node0.v = 0,

        node1.ip = (uint16_t)i,
        node1.v = 1,

        cnodes.insert(CNode(i, cn0.p + cn1.p));
        ++i;
    }
    assert(cnodes.size() == 1);
    assert(i == 2*N - 1);
}


// Запись в файл и чтение из файла в бинарном виде

void HuffmanTree::save(std::ostream& dst){
    uint16_t size = nodes.size();
    dst.write((char*)&size, sizeof(size));
    dst.write((char*)&nodes[0], nodes.size() * sizeof(Node));
}

void HuffmanTree::load(std::istream& src){
    uint16_t size;
    src.read((char*)&size, sizeof(size));
    if(!src.good())
        throw HuffmanException("file is too small");
    nodes.resize(size);
    src.read((char*)&nodes[0], nodes.size() * sizeof(Node));
    if(!src.good())
        throw HuffmanException("file is too small");
}


std::size_t HuffmanTree::additional_data_size(){
    return sizeof(uint16_t) + nodes.size() * sizeof(Node) + sizeof(seq_size_t);
}


bool HuffmanTree::operator==(const HuffmanTree& t) const{
    return nodes == t.nodes;
}


bool HuffmanTree::Node::is_leaf(){
    return i0 == (uint16_t)-1 && i1 == (uint16_t)-1;
}
uint16_t HuffmanTree::Node::next_node_index(bool bit){
    uint16_t res = bit? i1 : i0;
    if(res == -1)
        throw 0;
    return res;
}
bool HuffmanTree::Node::operator==(const Node& n) const{
    return i0 == n.i0 && 
        i1 == n.i1 && 
        ip == n.ip && 
        v == n.v && 
        symb == n.symb;
}




HuffmanTree::Node& HuffmanTree::find_leaf_with_symbol(char symb){
    int N = (nodes.size() + 1) / 2;
    for(int i = 0; i < N; i++){
        Node& node = nodes[i];
        assert(node.is_leaf());
        if(node.symb == symb){
            return node;
        }
    }
    throw HuffmanException("symbol '" + std::to_string(symb) + "' does not exist in the huffman tree");
}



// Подсчитывает количества всех входящих в текст src символов. Осталяет курсор потока на месте.
std::map<char, double> Huffman::counts(std::istream& src){
    std::map<char, double> p;
    auto state = src.rdstate();
    auto pos = src.tellg();
    while(true){
        char symb = src.get();
        if(!src.good())
            break;
        if(p.count(symb) == 0)
            p[symb] = 1;
        else
            p[symb] += 1;
    }
    src.clear(state);
    src.seekg(pos);
    return p;
}


std::size_t Huffman::encode(std::istream& src, std::ostream& dst){
    auto p = counts(src);
    if(p.size() <= 1){
        p[0] = 0;
        p[1] = 0;
    }
    HuffmanTree tree(p);
    tree.save(dst);
    tree.encode(src, dst);
    return tree.additional_data_size();
}

std::size_t Huffman::decode(std::istream& src, std::ostream& dst){
    HuffmanTree tree;
    tree.load(src);
    tree.decode(src, dst);
    return tree.additional_data_size();
}



