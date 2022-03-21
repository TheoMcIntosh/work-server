#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <stack>
#include <vector>
#include <list>
#include <unordered_map>

#include "lib/json.hpp"

#define CHAR_SIZE 128

using namespace std;
using json = nlohmann::json;
map<string, vector<string>> ParentTable;  
map<string, string> ChildTable;


// Trie lookup

struct Trie 
{
    bool isWord;
    unordered_map<char, Trie*> children;
    
};

Trie* GetNewTrieNode()
{
    Trie* node = new Trie;
    node->isWord = false;

    return node;
}

void insertTrie(Trie*& head, string key) 
{
    if (head == nullptr) {
        head = GetNewTrieNode();
    }
    //cout << head << "\n";
    
    Trie* curr = head;
    for (int i = 0; i < key.length(); i++)
    {
        //Create a new node 
        if (curr->children.find(key[i]) == curr->children.end()) {
            curr->children[key[i]] = GetNewTrieNode();
        }
 
        //Next node
        curr = curr->children[key[i]];
        
    }

    curr->isWord = true;
   
}

void BuildTrie(Trie*& head)
{
    if (!std::filesystem::is_empty("Json_Files/")) {
        for (const auto& entry : std::filesystem::directory_iterator("Json_Files/")) {
            ifstream File(entry);
            json j = json::parse(File);
            for (const auto& obj : j) {
                insertTrie(head, obj["rotation"]);
                
            }
        }
    }
}
void BuildDataTrie(Trie*& head)
{
    if (!std::filesystem::is_empty("MineData/")) {
        for (const auto& entry : std::filesystem::directory_iterator("MineData/")) {
            string Line = entry.path();
            insertTrie(head, Line.substr(9,64));   
        }
    }
}

vector<string> PrefixSearch(Trie*& root, string Prefix) 
{
    Trie* curr = root;
    // cout << root << "\n";
    // cout << curr << "\n";
    // cout << "HI2.1" << "\n";
    vector<string> v;
    //cout << "HI2.2" << "\n";
    for (int i = 0; i < Prefix.length(); i++) {
        // cout << "HI2.3" << "\n";
        // cout << curr << "\n";
        // cout << curr->children[Prefix[i]] << "\n";
        // cout << Prefix << "\n";
        curr = curr->children[Prefix[i]];
        
        //cout << Prefix << "\n";
        if (curr == NULL) {
            return v;
        } 
    }

    stack<string> p;
    stack<Trie*> t;
    p.push(Prefix);
    t.push(curr);

    while (!p.empty()) {
        Prefix = p.top();
        Trie* curr = t.top();
        p.pop();
        t.pop();

        if (curr->isWord) {
            v.push_back(Prefix);
        }
        for (auto c : curr->children) {   
            if (c.second == nullptr) {
                continue;
            }
            p.push(Prefix + c.first);
            t.push(c.second);
        }
    }

    return v;
}

//----------------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------------

//Tables

void DeleteMaps() 
{
  ofstream Delete;
  Delete.open("Delete.md");
  Delete << true << endl;
  Delete.close();
}

void Insert(string SourceHash, string Child, string Parent) 
{
    if (filesystem::exists("Delete.md")) {
        ParentTable.clear();
        ChildTable.clear();
        remove("Delete.md");
    }

    if (!ParentTable.count(SourceHash)) {
        ParentTable[SourceHash];//.push_back("NULL");
    }
   
    ParentTable[SourceHash].push_back(Child);

    if (!ParentTable.count(Child)) {
        ParentTable[Child];//.push_back("NULL");
    }

    if (Parent == "None") {
        ChildTable.insert(pair<string,string>(SourceHash, "NULL"));
    } 
    ChildTable.insert(pair<string,string>(Child, SourceHash));

}

void PrintTable() 
{
    for (const auto & c : ChildTable) {
        cout << c.first << " child of - " << c.second << "\n";
    }
    cout << "\n\n";

    for (const auto & p : ParentTable) {
        cout << p.first << " parent of - ";
        for (const auto &i : p.second) {
            cout << i << ", ";
        }
        cout << "\n";
    }
    cout << "\n\n";
}

void SaveTables() 
{
    ofstream ParentTableFile;
    ParentTableFile.open("HashTables/ParentRecords.dat");
    for (const auto & p : ParentTable) {
        ParentTableFile << "----------------------\n";
        ParentTableFile << p.first << " Parent of:\n";
        for (const auto & i : p.second) {
            ParentTableFile << i << "\n";
        }
    }

    ofstream ChildTableFile;
    ChildTableFile.open("HashTables/ChildRecords.dat");
    for (const auto & c : ChildTable) {
        ChildTableFile << "----------------------\n";
        ChildTableFile << c.first << " Child of:\n";
        ChildTableFile << c.second << "\n";
    }
    ParentTableFile.close();
    ChildTableFile.close();    

}

void LoadTable()
{
    if (filesystem::exists("HashTables/ParentRecords.dat")) {
        ifstream File;
        File.open("HashTables/ParentRecords.dat");
        string Line;
        getline(File, Line);
        while (!File.eof()) {
            getline(File, Line);
            string Parent = Line.substr(0,64);
            ParentTable[Parent];
            getline(File,Line);
            string Child = Line.substr(0, 64);
            while (Child.size() == 64) {
                ParentTable[Parent].push_back(Child);
                getline(File,Line);
                Child = Line.substr(0, 64);
            }
        }

        ifstream File2;
        File2.open("HashTables/ChildRecords.dat");
        string line;
        getline(File2, line);
        while (!File2.eof()) {
            getline(File2, line);
            string NChild = line.substr(0,64);
            getline(File2, line);
            string NParent = line;
            ChildTable.insert(pair<string,string>(NChild, NParent));
            getline(File2, line);
        }
    } else {
        return;
    }
    
}

//----------------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------------

// DFS and data retrival


void DFS(string SourceHash)
{
    stack<string> p;
    ofstream FileS("ReturnFile/" + SourceHash + ".json");
    json js;
    
    for (auto Children : ParentTable[SourceHash]) {
        p.push(Children);
    }
    

    while(!p.empty()) {
        string curr = p.top();
        p.pop();
        if (curr.size() == 64) {
            using json = nlohmann::json;
            json j;
            //cout << curr << "\n";
            if (filesystem::exists("Json_Files/" + ChildTable[curr] + ".json")) {
                ifstream File("Json_Files/" + ChildTable[curr] + ".json");
                j = json::parse(File);
                // for (const auto& obj : j) {
                //     if (obj["rotation"] == curr) {
                //         js[curr] = obj;
                //         break;
                //     }
                // } 
                js[curr] = j[curr];
                //cout << j.dump(4) << "\n"

                for (auto child : ParentTable[curr]) {
                    p.push(child);
                
                }
            }
        }
    }
    FileS << js.dump(4) << std::endl;
    FileS.close();
  //json object that changes as you go through 
}

void GetData(string RotOrHash, string &Return) 
{
    
    string FileName;
    string Target = "21e8";
    

    if (filesystem::exists("MineData/" + RotOrHash + ".dat")) {
        ifstream File("MineData/" + RotOrHash + ".dat");
        getline(File, Return);
        return;
    } else if (filesystem::exists("Json_Files/" + ChildTable[RotOrHash] + ".json")) {
        FileName = ChildTable[RotOrHash];
        ifstream File("Json_Files/" + FileName + ".json");
        json j = json::parse(File);
        for (const auto& obj : j) {
            if (obj["rotation"] == RotOrHash) {
                Return = obj["data"];
                return;
            }
        }
    } 

    return;
}


