//
// STARTER CODE: util.h
//
// TODO:  Write your own header
//

#pragma once

#include <iostream>
#include <fstream>
#include <map>
#include <unordered_map>

typedef hashmap hashmapF;
typedef unordered_map <int, string> hashmapE;

struct HuffmanNode {
    int character;
    int count;
    HuffmanNode* zero;
    HuffmanNode* one;
};

struct compare
{
    bool operator()(const HuffmanNode *lhs,
        const HuffmanNode *rhs)
    {
        return lhs->count > rhs->count;
    }
};

void _free(HuffmanNode* node) {
    if (node == nullptr)
        return;
    _free(node->zero);
    _free(node->one);
    delete node;
}

//
// *This method frees the memory allocated for the Huffman tree.
//
void freeTree(HuffmanNode* node) {
    _free(node);
    node = nullptr;
}

void fileFrequencyMap(string filename, hashmap &map) {
    // Read file, counting each character's frequency (including whitespace,
    // punctuation and EOF markers)
    fstream fs;
    fs.open(filename);
    char currChar;
    streampos pos;
    HuffmanNode n;

    if (!fs.is_open()) {
        cout << "Couldn't open " << filename << endl;
        return;
    }

    while (!fs.eof()) {
        fs >> noskipws >> currChar;

        // if currChar hasn't already been maaped
        if (!map.containsKey(currChar)) {
            // Savor position in file
            pos = fs.tellg();

            // Huffman node for character
            n.character = currChar;
            n.count = 1;

            // Scan rest of file for character, incrementing n.count as needed
            while (fs >> noskipws >> currChar) {
                if (currChar == n.character) {
                    n.count++;
                    if (fs.peek() == EOF)
                        break;
                }
            }
            // Gone through file, so add the pair to the map
            map.put(n.character, n.count);

            // Return to pos
            currChar = n.character;
            fs.clear();
            fs.seekg(pos);
        }
    }
    // Insert pseudo-EOF character
    map.put(PSEUDO_EOF, 1);
    fs.close();
}

void stringFrequencyMap(string str, hashmap &map) {
    // Read string, counting each character's frequency
    stringstream ss(str);
    char currChar;
    streampos pos;
    HuffmanNode n;

    while (ss >> noskipws >> currChar) {
        // if currChar isn't in map
        if (!map.containsKey(currChar)) {
            // Save our position in the string
            pos = ss.tellg();

            n.character = currChar;
            n.count = 1;

            // Scan file for all instances of char
            while (ss >> noskipws >> currChar) {
                if (currChar == n.character) {
                    n.count++;
                    if (ss.peek() == EOF)
                        break;
                }
            }
            // Reach eof so add pair to map
            map.put(n.character, n.count);

            // backtrack
            currChar = n.character;
            ss.clear();
            ss.seekg(pos);
        }
    }
    // insert EOF
    map.put(PSEUDO_EOF, 1);
}

//
// *This function builds the frequency map.  If isFile is true, then it reads
// from filename.  If isFile is false, then it reads from a string filename.
//
void buildFrequencyMap(string filename, bool isFile, hashmapF &map) {
    (isFile) ? fileFrequencyMap(filename, map) : stringFrequencyMap(filename, map);
}

// Prioritize function for priority queue used to build encoding tree
class prioritize {
 public:
    bool operator()(const HuffmanNode* n1, const HuffmanNode* n2) const {
        return n1->count > n2->count;
    }
};

//
// *This function builds an encoding tree from the frequency map.
//
HuffmanNode* buildEncodingTree(hashmapF &map) {
    priority_queue<HuffmanNode*, vector<HuffmanNode*>, prioritize> pq;
    HuffmanNode* n;

    // Build priority queue
    for (auto&key : map.keys()) {
        n = new HuffmanNode;

        n->character = key;
        n->count = map.get(key);
        n->zero = nullptr;
        n->one = nullptr;

        pq.push(n);
    }
    // build the tree
    while (pq.size() > 1) {
        // remove two nodes
        HuffmanNode* a = pq.top();
        pq.pop();
        HuffmanNode* b = pq.top();
        pq.pop();

        // Make a new, combined node that is parent of both removed nodes
        HuffmanNode* c = new HuffmanNode;
        c->zero = a;
        c->one = b;
        c->count = a->count + b->count;
        c->character = NOT_A_CHAR;

        // add new node to queue
        pq.push(c);
    }

    // Tree now consists of one node, the root
    return pq.top();
}

//
// *Recursive helper function for building the encoding map.
//
void _buildEncodingMap(HuffmanNode* node, hashmapE &encodingMap, string str,
                       HuffmanNode* prev) {
    string zeroStr, oneStr;

    if (node->character != NOT_A_CHAR) {
        encodingMap.emplace(node->character, str);
        return;
    }

    zeroStr = str + '0';
    _buildEncodingMap(node->zero, encodingMap, zeroStr, node);

    oneStr = str + '1';
    _buildEncodingMap(node->one, encodingMap, oneStr, node);
}

//
// *This function builds the encoding map from an encoding tree.
//
hashmapE buildEncodingMap(HuffmanNode* tree) {
    hashmapE encodingMap;
    string str;

    _buildEncodingMap(tree, encodingMap, str, nullptr);

    return encodingMap;
}

//
// *This function encodes the data in the input stream into the output stream
// using the encodingMap.  This function calculates the number of bits
// written to the output stream and sets result to the size parameter, which is
// passed by reference.  This function also returns a string representation of
// the output file, which is particularly useful for testing.
//
string encode(ifstream& input, hashmapE &encodingMap, ofbitstream& output,
              int &size, bool makeFile) {
    char c;
    string str = "";
    int bin;

    // Read one char at a time from input file
    while (input.get(c)) {
        // Use encoding map to encode char to binary
        str += encodingMap.at(c);
    }

    // Write PSEUDO_EOF at the end
    str += encodingMap.at(PSEUDO_EOF);

    if (makeFile) {
        for (char c : str) {
            bin = (c == '1') ? 1 : 0;
            output.writeBit(bin);
        }
    }
    size = str.size();
    return str;
}


//
// *This function decodes the input stream and writes the result to the output
// stream using the encodingTree.  This function also returns a string
// representation of the output file, which is particularly useful for testing.
//
string decode(ifbitstream &input, HuffmanNode* encodingTree, ofstream &output) {
    int bit;
    string str;
    HuffmanNode* curr;

    curr = encodingTree;
    str = "";
    bit = 0;

    // Read one bit at a time
    while (bit != EOF) {
        bit = input.readBit();

        // Traverse tree according to bits till we reach a leaf node
        if (bit == 0)
            curr = curr->zero;
        else
            curr = curr->one;

        if (curr->character != NOT_A_CHAR) {
            // If node is EOF, we're done
            if (curr->character == PSEUDO_EOF)
                break;
            // Else, we've reached a leaf node, so write to output and restart
            output.put(curr->character);
            str += curr->character;
            curr = encodingTree;
        }
    }
    return str;
}

//
// *This function completes the entire compression process.  Given a file,
// filename, this function (1) builds a frequency map; (2) builds an encoding
// tree; (3) builds an encoding map; (4) encodes the file (don't forget to
// include the frequency map in the header of the output file).  This function
// should create a compressed file named (filename + ".huf") and should also
// return a string version of the bit pattern.
//
string compress(string filename) {
    hashmapF fMap;
    hashmapE eMap;
    HuffmanNode* tree = nullptr;
    ofbitstream output(filename + ".huf");
    ifstream input(filename);
    int size = 0;
    string codeStr;

    // Build frequency map
    buildFrequencyMap(filename, true, fMap);

    // Build encoding tree
    tree = buildEncodingTree(fMap);

    // Build encoding map
    eMap = buildEncodingMap(tree);

    // Encode file w/ frequency map header
    output << fMap;
    codeStr = encode(input, eMap, output, size, true);
    output.close();
    freeTree(tree);

    return codeStr;
}

//
// *This function completes the entire decompression process.  Given the file,
// filename (which should end with ".huf"), (1) extract the header and build
// the frequency map; (2) build an encoding tree from the frequency map; (3)
// using the encoding tree to decode the file.  This function should create a
// compressed file using the following convention.
// If filename = "example.txt.huf", then the uncompressed file should be named
// "example_unc.txt".  The function should return a string version of the
// uncompressed file.  Note: this function should reverse what the compress
// function did.
//
string decompress(string filename) {
    hashmapF fMap;
    hashmapE eMap;
    HuffmanNode* tree = nullptr;
    string contents, ext;

    // File names and i/o stream
    size_t pos = filename.find(".huf");
    if ((int) pos >= 0) {
        filename = filename.substr(0, pos);
    }
    pos = filename.find(".");
    ext = filename.substr(pos, filename.length() - pos);
    filename = filename.substr(0, pos);

    ifbitstream input(filename + ext + ".huf");
    ofstream output(filename + "_unc" + ext);

    // Extract header and build frequency map
    input >> fMap;

    // Build encoding tree from frequency map
    tree = buildEncodingTree(fMap);

    // Use encoding tree to decode file
    contents = decode(input, tree, output);
    freeTree(tree);

    return contents;
}
