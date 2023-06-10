
#include "huffman.h"
#include <iostream>
#include <fstream>


using namespace Huffman;
using namespace std;


struct command{
    enum {ENCODE, DECODE, UNDEFINED} action;
    const char* file_path;
    const char* output_path;
    command():
        action(UNDEFINED), file_path(nullptr), output_path(nullptr)
    { }
};



void make_command(const command& c){
    if(c.action == command::UNDEFINED){
        cout << "no action - encode (-c) or decode (-u)?" << endl;
        return;
    }

    if(c.file_path == nullptr){
        cout << "no source file" << endl;
        return;
    }

    if(c.output_path == nullptr){
        cout << "no output file" << endl;
        return;
    }

        
    ifstream in(c.file_path);
    if(!in){
        cout << "source file does not exist: " << c.file_path << endl;
        return;
    }

    ofstream out(c.output_path);
    if(!out){
        cout << "can't create output file" << endl;
        return;
    }
    
    try{
        auto begin_in = in.tellg();
        auto begin_out = out.tellp();
        std::size_t size_tree;
        if(c.action == command::ENCODE)
            size_tree = encode(in, out);
        else
            size_tree = decode(in, out);
        assert(in.good());
        assert(out.good());
        std::size_t size_in = in.tellg() - begin_in;
        std::size_t size_out = out.tellp() - begin_out;
        if(c.action == command::ENCODE)
            size_out -= size_tree;
        else
            size_in -= size_tree;
        cout << size_in << "\n" 
            << size_out << "\n"
            << size_tree << endl;
    }
    catch(const HuffmanException& e){
        std::cout << e.message << "\n";
        return;
    }
    catch(...){
        std::cout << "unknown error" << "\n";
        return;
    }
}


bool parse_command(int argc, char* argv[], command& c){
    if(argc != 6){
        cout << "wrong args count" << "\n";
        return false;
    }

    int i = 1;
    while(i < 6){
        std::string arg(argv[i]);
        i += 1;
        if(arg == "-c"){
            c.action = command::ENCODE;
        }
        else if(arg == "-u"){
            c.action = command::DECODE;
        }
        else if(arg == "-f" || arg == "--file"){
            c.file_path = argv[i];
            i += 1;
        }
        else if(arg == "-o" || arg == "--output"){
            c.output_path = argv[i];
            i += 1;
        }
        else{
            cout << "unknown flag: " << arg << endl;
            return false;
        }
        
    }
    return true;
}


int main(int argc, char* argv[]){
    command c;
    if(parse_command(argc, argv, c))
        make_command(c);
}