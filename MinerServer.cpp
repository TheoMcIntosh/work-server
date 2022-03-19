#include <stdlib.h>
#include <string>
#include <iostream>
#include <time.h>
#include <ctime>
#include <fstream>
#include <filesystem>
#include <vector>

#include "lib/sha256.cpp"
#include "lib/httplib.h"
#include "lib/json.hpp"
#include "ServerUtils.hpp"
#include "TaskFlowMine.hpp"



using namespace httplib;
using json = nlohmann::json;

struct Block
{
    std::string Source;
    std::string Target;
    std::string Data;
    std::string Iterations;
};

void mine(Block x);
string Validate(json j);

int main()
{
    Server svr;
    LoadTable();
    //RebuildTrie();
    svr.Post("/api/v2/data", [](const Request& req, Response& res, const ContentReader &content_reader) {
        std::string body;

        content_reader([&](const char *data, size_t data_length) {
            body.append(data, data_length);
        return true;
        });

        std::string name;
        json j = json::parse(body);
        for (const auto& obj : j) {
            name = obj["rotation"];
            if (filesystem::exists("Json_Files/" + name + ".json")) {
                res.status = 500;
                return;
            }
        } 
        std::string result = Validate(j);

        std::string datahash;
        std::string line;
        std::string file;
        

        if (result == "unlucky") {
            res.status = 500;
        } else {
            std::fstream FileAdd;
            FileAdd.open("Json_Files/" + ChildTable[result] + ".json");
            json jo = json::parse(FileAdd);
            for (const auto& obj : jo) {
                if (obj["rotation"] == result) {
                    datahash = obj["datahash"];
                }
            }
            std::fstream File;
            File.open("MineData/" + datahash + ".dat");
            getline(File, line);
            
            for (const auto& obj : j) {
                file = obj["source"];
                if (filesystem::exists("Json_Files/" + file + ".json")) {
                    std::ofstream File;
                    File.open("Json_Files/" + file + ".json");
                    json ja = json::parse(FileAdd);
                    ja[file] = j[file];
                    File << ja.dump(4) << std::endl;
                    File.close();
                } else {
                    std::ofstream File2;
                    File2.open("Json_Files/" + file + ".json");
                    File2 << j.dump(4) << std::endl;
                    File2.close();
                }
                Insert(obj["source"], obj["rotation"], "yes");
                SaveTables();
            }
            std::cout << "Rotation added from client and data sent back \n" << std::endl;
            res.set_content(line, "text/plain");
            res.status = 200;
        }

    });

    svr.Post("/api/v2/mine", [](const Request& req, Response& res, const ContentReader &content_reader) {
        std::string source;
        std::string target = "";
        std::string data = "";
    
        string iterations = "1";
        if (req.has_param("source")) {
            source = req.get_param_value("source");
            if (req.has_param("target")) target = req.get_param_value("target");
            if (req.has_param("data")) data = req.get_param_value("data");
            if (req.has_param("iterations")) iterations = req.get_param_value("iterations");

            if ((target == "") || (data == "")) {
                ifstream File1("HashTables/ChildRecords.dat");
                File1.open("HashTables/ChildRecords.dat");
                std::string Parent = ChildTable[source];
                ifstream File2("Json_Files/" + Parent + ".json");
                json jb = json::parse(File2);
                for (const auto& obj : jb) {
                    if (obj["rotation"] == source) {
                            data = obj["datahash"];
                            target = obj["target"];
                    }
                }
            }
            
            struct Block ToMine;
            ToMine.Source = source;
            ToMine.Target = target;
            ToMine.Data = data;
            ToMine.Iterations = iterations;
            mine(ToMine);
            
        } else if (req.has_param("Source")) {
            source = req.get_param_value("Source");
            std::string iterations = "1";
         
            ifstream File1("HashTables/ChildRecords.dat");
            File1.open("HashTables/ChildRecords.dat");
            std::string Parent = ChildTable[source];
            ifstream File2("Json_Files/" + Parent + ".json");
            json jb = json::parse(File2);
            for (const auto& obj : jb) {
                if (obj["rotation"] == source) {
                        data = obj["datahash"];
                        target = obj["target"];
                }
            }
            struct Block ToMine;
            ToMine.Source = source;
            ToMine.Target = target;
            ToMine.Data = data;
            ToMine.Iterations = iterations;

            mine(ToMine);

        } else {
            std::string body;
            res.set_header("Access-Control-Allow-Origin", "*");

            content_reader([&](const char *data, size_t data_length) {
                body.append(data, data_length);
            return true;
            });
            
            json j = json::parse(body);
            

            struct Block ToMine;
        
            ToMine.Source = j["Source"];
            ToMine.Target = j["Target"];
            ToMine.Data = j["Data"];
            ToMine.Iterations = std::to_string((int)j["Iterations"]);
            
            mine(ToMine);
        }
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content("Mine succesfully begun", "text/plain");
        res.status = 202;

    });

    svr.Get(R"(/api/v2/index/(.*))", [](const Request& req, Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        SHA256 sh;
        std::string body = req.matches[1]; 
        nlohmann::ordered_json jr;
        std::ifstream FileAdd;

        if (body.length() != 64) {
            body = sh(body);
        }

        if (filesystem::exists("Json_Files/" + body + ".json")) {
            DFS(body);
            FileAdd.open("ReturnFile/" + body + ".json");
            jr = json::parse(FileAdd);
            FileAdd.close();
        } else if (ChildTable.count(body) == 1 && filesystem::exists("Json_Files/" + ChildTable[body] + ".json")) {
            FileAdd.open("Json_Files/" + ChildTable[body] + ".json");
            json j = json::parse(FileAdd);
            FileAdd.close();
            jr[body] = j[body];
        } else {
            jr = json({});
        }


        res.set_content(jr.dump(4) ,"application/json");
    
    }); 

    svr.Get(R"(/api/v2/data/(.*))", [](const Request& req, Response& res) {
      
        res.set_header("Access-Control-Allow-Origin", "*");

        res.set_content("Sorry you have not payed for this data yet. Please mine to get access.", "text/plain");
        res.status = 200;
        return;

        const std::string body = req.matches[1];
        string Return = "";
        string ReturnFinal = "";

        if (body.size() != 64) {
            Trie* head = nullptr;
            BuildDataTrie(head);

            if (head == nullptr) {
                res.status = 404;
                res.set_content("", "text/plain");
                return;
            }

            vector<string> vt = PrefixSearch(head, body);
            vector<string> r; 
            
            if (vt.size() == 0) {
                res.status = 404;
                res.set_content("", "text/plain");
                return;
            }

            for (auto i : vt) {
                Return = i;
                GetData(Return, ReturnFinal);
                r.push_back(ReturnFinal);
            }
            ReturnFinal = "";
            for (auto j : r) {
                ReturnFinal += j + ", ";
            }
            ReturnFinal.resize(ReturnFinal.size() - 2);
            res.set_content(ReturnFinal, "text/plain");
            return;
        }

        GetData(body, Return);

        if (Return.size() == 0) {
            res.status = 404;
            res.set_content("", "text/plain");
        } else {
            res.set_content(Return,"text/plain");
        }
    
    }); 
    
    svr.Get(R"(/api/v2/filter/(.*))", [](const Request& req, Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");

        const std::string body = req.matches[1];
        
        Trie* head = nullptr;
        BuildTrie(head);

        if (head == nullptr) {
            res.status = 404;
            res.set_content("", "text/plain");
            return;
        }

        vector<string> vt = PrefixSearch(head, body);

        if (vt.size() == 0) {
            res.status = 404;
            res.set_content("", "text/plain");
            return;
        }

        string Return = "";
        for (auto i : vt) {
            Return += i + "\n";
        }
        res.set_content(Return, "text/plain");
        
    });

//----------------------------this is nessacary-----------------------------//

    svr.Get(R"(/api/v2/prefix/(.*))", [](const Request& req, Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
    
        const std::string body = req.matches[1]; 
        nlohmann::ordered_json jr;
        std::ifstream FileAdd;
        
        if (ChildTable.count(body) == 1 && filesystem::exists("Json_Files/" + ChildTable[body] + ".json")) {
            FileAdd.open("Json_Files/" + ChildTable[body] + ".json");
            json j = json::parse(FileAdd);
            FileAdd.close();
            jr[body] = j[body];
        } else {
            jr = json({});
        }


        res.set_content(jr.dump(4) ,"application/json");
    
    }); 
//---------------------------------------------------------------------------//

    // pugi::xml_document doc;
    // pugi::xml_parse_result result = doc.load_file("http://www.sportingnews.com/us/rss");
    // std::cout << "Load result: " << result.description() << ", mesh name: " << doc.child("mesh").attribute("name").value() << std::endl;

    svr.listen("0.0.0.0", 2180);

}


string Validate(json j) 
{
    SHA256 sha256;

    std::string PreHash;
    std::string Hash;
    std::string datause;
    std::string ThisOne;
    std::string Source;
    std::string DataHash;
    std::string Target;
    std::string User;
    std::string Nonce;

    for (const auto& obj : j) {
        Source = obj["source"];
        DataHash = obj["datahash"];
        Target = obj["target"];
        User = obj["user"];
        Nonce = obj["n"];
    }
    
    PreHash = Source + DataHash + Target + User + Nonce;
    Hash = sha256(PreHash);
    
    std::fstream FileAdd;
    FileAdd.open("Json_Files/" + ChildTable[Source] + ".json");
    json jo = json::parse(FileAdd);
    for (const auto& obj : jo) {
        if (obj["rotation"] == Source) {
            datause = obj["cost"];
        }
    }

    cout << datause << endl;
    cout << Hash << endl;

    while (ChildTable[Source] != "NULL") {
        ThisOne = Source;
        Source = ChildTable[Source];
    }
    
    if (Hash.substr(0, datause.size()) == datause) {
        return ThisOne;
    } else {
        return "unlucky";
    }
}



void mine(Block x)
{
    
    // if (argc != 5) {
    //     std::cout << "Usage: <Source> <Data> <Target> <Iterations>/n";
    //     return 0;
    // }
   
    SHA256 sha256;


    // x.Source = std::string Source;
    // x.Data = std::string Data;
    // x.Target = std::string Target;
    // x.Iterations = std::string Iterations;
    int n = stoi(x.Iterations);
    int i = 0;

    std::string SourceHash;
    if (x.Source.length() != 64) {
        SourceHash = sha256(x.Source);
    } else {
        SourceHash = x.Source;
    }
    
    std::string DataHash;
    if (x.Data.length() != 64) {
        DataHash = sha256(x.Data);
    } else {
        DataHash = x.Data;
    }

    std::string User = "5468656f";
    uint64_t Nonce;

    random_device rd;
    default_random_engine generator(rd());
    uniform_int_distribution<long long unsigned> distribution(0,0xFFFFFFFFFFFFFFFF);
    //Nonce = distribution(generator);

    

    std::string Hash;
    std::string PreHash;
    std::string OriginalSourceHash = SourceHash;
    std::string Parent = "None";
    
    //Trie* head = nullptr;


    //Fork means they use same starting souce data etc however the nonce starts off from where it last finished
    //Rotate means that the source starts off at where it left off and everything else is the same (nonce = 1 etc)

    if (!filesystem::exists("MineData/" + DataHash + ".dat")) {

        std::ofstream DataFile("MineData/" + DataHash + ".dat");
        DataFile << x.Data << std::endl;
    } 
    
    //PrintTable();
    
    while (i < n) {
        // do {
        //     PreHash = SourceHash + DataHash + x.Target + User + std::to_string(++Nonce);
        //     Hash = sha256(PreHash);
        // } while (Hash.substr(0, x.Target.size()) != x.Target);
        
        // do {
        //     PreHash = SourceHash + DataHash + x.Target + User + std::to_string(++Nonce);;
        //     unsigned char* hashuse = (unsigned char*) PreHash.c_str();
        //     FastMine(hashuse, PreHash.length(), result);
        //     Hash = hash_to_string(result);
        
        // } while (Hash.substr(0, x.Target.size()) != x.Target);
        const std::string constant = SourceHash + DataHash + x.Target + User;

        Nonce = parallel_mine(constant, x.Target);
        PreHash = constant + std::to_string(Nonce);
        Hash = sha256(PreHash);
        
        std::cout << "server nonce: " << Nonce << std::endl;
        std::cout << "server prehash: " << PreHash << std::endl;
        std::cout << "server hash: " << Hash << "\n" << std::endl;
        

        std::time_t t = std::time(0);  // t is an integer type

        
        
        if (filesystem::exists("Json_Files/" + SourceHash + ".json")) {
            std::fstream FileAdd;
            FileAdd.open("Json_Files/" + SourceHash + ".json");
            json jo = json::parse(FileAdd);
            FileAdd.close();

            ofstream File("Json_Files/" + SourceHash + ".json");

            jo[Hash] = {
                {"datahash",DataHash},
                {"n",std::to_string((uint64_t)Nonce)},
                {"rotation", Hash},
                {"source", SourceHash},
                {"target", x.Target},
                {"timestamp", t},
                {"user", User},
                {"cost", x.Target}
            };

            File << jo.dump(4) << std::endl;
            File.close();
        } else {
            std::ofstream FileAdd("Json_Files/" + SourceHash + ".json");
            json j;

            j = {
            {Hash,{
                {"datahash",DataHash},
                {"n",std::to_string((uint64_t)Nonce)},
                {"rotation", Hash},
                {"source", SourceHash},
                {"target", x.Target},
                {"timestamp", t},
                {"user", User},
                {"cost", x.Target}
                }}
            };
       
            FileAdd << j.dump(4) << std::endl;
            FileAdd.close();
        }

        
        Insert(SourceHash, Hash, Parent);
        //insertTrie(head, Hash);
        //cout << head << "\n";
        //PrintTable();
        std::cout << SourceHash << " loaded. " << "Nonce - " << Nonce << "\n"; 
        
        Parent = SourceHash;
        SourceHash = Hash;
        //Nonce = distribution(generator);
        ++i;
    }
    cout << "\n";
    SaveTables();
    //DFS(OriginalSourceHash);
}

//https://super.21e8.nz
//http://127.0.0.1:2180
