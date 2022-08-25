#include <bsmlib.hpp>
#include <iostream>

enum class ParseState {
    InFilename,
    InAction,
    InKeytype,
    InKeyname,
    InKeyvalue
};

enum class ToolError {
    FileOpenError,
    BSMReadError,
    UnkownAction,
    NoGivenAction,
    InvalidSyntax,
    Unknown
};

void PrintHelp() {
    std::cout
        << "bsmtool v1.0.0 by Colleen" << std::endl
        << std::endl
        << "Usage: bsm file (list | dump | get [keys] | remove [keys] | set {options})" << std::endl
        << "\t- When using 'list', bsmtool will list all keys and their values." << std::endl
        << "\t- When using 'dump', bsmtool will dump 'raw' keys to appropriately named files." << std::endl
        << "\t- When using 'get', bsmtool will list specified keys." << std::endl
        << "\t- When using 'remove', bsmtool will remove (delete) specified keys." << std::endl
        << std::endl
        << "Options:" << std::endl
        << "\t-i <name> <value>    Set integer value." << std::endl
        << "\t-f <name> <value>    Set float value." << std::endl
        << "\t-s <name> <value>    Set string value." << std::endl
        << "\t-r <name> <file>     Set raw value using bytes from given file." << std::endl;
}

void PrintErr(ToolError errcode, std::vector<std::string> args = std::vector<std::string>()) {
    std::cerr << "ERROR: ";
    switch(errcode) {
        case ToolError::FileOpenError:
            std::cerr << "Could not open file: \"" << args[0] << "\"." << std::endl;
            break;
        case ToolError::BSMReadError:
            std::cerr << "Could not read file as BSM: \"" << args[0] << "\"." << std::endl;
            break;
        case ToolError::UnkownAction:
            std::cerr << "Unkown action: \"" << args[0] << "\"." << std::endl;
            break;
        case ToolError::NoGivenAction:
            std::cerr << "No given action." << std::endl;
            break;
        case ToolError::InvalidSyntax:
            std::cerr << "Invalid syntax." << std::endl;
            break;
        default:
            std::cerr << "UNKOWN ERROR. THIS IS A BUG.";
            break;
    }

    std::cout << "Use option '--help' for more information." << std::endl;
}

void PrintKey(bsmlib::Key &key, std::string keyname) {
    switch(key.type) {
        case bsmlib::KeyType::Integer:
            std::cout << "(int)    \"" << keyname << "\" = " << key.value_int << std::endl;
            break;
        case bsmlib::KeyType::Float:
            std::cout << "(float)  \"" << keyname << "\" = " << key.value_float << std::endl;
            break;
        case bsmlib::KeyType::String:
            std::cout << "(string) \"" << keyname << "\" = \"" << key.value_string << "\"" << std::endl;
            break;
        case bsmlib::KeyType::Raw:
            std::cout << "(raw)    \"" << keyname << "\" = <" << key.data.size() << " bytes>" << std::endl;
            break;
        default:
            std::cout << "(unkown) \"" << keyname << "\" = <" << key.data.size() << " bytes>" << std::endl;
            break;
    }
}

void ListKeys(bsmlib::Data &data, std::string filename) {
    int keycount = data.keys.size();

    std::cout << "File \"" << filename << "\" (" << std::to_string(keycount) << " keys):" << std::endl;

    for(auto &p : data.keys) {
        auto &keyname = p.first;
        auto &keyvalue = p.second;

        PrintKey(keyvalue, keyname);
    }
}

void DumpKeys(bsmlib::Data &data, std::string filename) {
    int keycount = data.keys.size();

    std::cout << "File \"" << filename << "\" (" << std::to_string(keycount) << " keys):" << std::endl;

    for(auto &p : data.keys) {
        auto &keyname = p.first;
        auto &keyvalue = p.second;

        bool doWrite = false;

        // Filter exported types
        switch(keyvalue.type) {
            case bsmlib::KeyType::Integer:
                std::cout << "IGNORING: (int)    \"" << keyname << "\"." << std::endl;
                break;
            case bsmlib::KeyType::Float:
                std::cout << "IGNORING: (float)  \"" << keyname << "\"." << std::endl;
                break;
            case bsmlib::KeyType::String:
                std::cout << "IGNORING: (string) \"" << keyname << "\"." << std::endl;
                break;
            case bsmlib::KeyType::Raw:
                std::cout << "WRITING:  (raw)    \"" << keyname << "\" (" << keyvalue.data.size() << " bytes) -> FILE: \"" << keyname << ".bin\"" << std::endl;
                doWrite = true;
                break;
            default:
                std::cout << "WRITING:  (unkown) \"" << keyname << "\" (" << keyvalue.data.size() << " bytes) -> FILE: \"" << keyname << ".bin\"" << std::endl;
                doWrite = true;
                break;
        }

        // Write to file if raw
        if(doWrite) {
            std::ofstream file(keyname + ".bin", std::ios::out | std::ios::binary);

            if(!file.good()) {
                PrintErr(ToolError::FileOpenError, {keyname + ".bin"});
            }else {
                file.write((const char*)keyvalue.data.data(), keyvalue.data.size());
            }

            file.close();
        }
    }
}

void GetKeys(bsmlib::Data &data, std::string filename, std::vector<std::string> keynames) {
    std::cout << "In file \"" << filename << "\":" << std::endl;

    for(auto &name : keynames) {
        if(data.KeyExists(name)) {
            auto key = data.GetKey(name);
            PrintKey(key, name);
        }else {
            std::cout << "Key not found: \"" << name << "\"." << std::endl;
        }
    }
}

void RemoveKeys(bsmlib::Data &data, std::string filename, std::vector<std::string> keynames) {
    std::cout << "In file \"" << filename << "\":" << std::endl;

    for(auto &name : keynames) {
        if(data.KeyExists(name)) {
            auto key = data.GetKey(name);
            std::cout << "DELETING: ";
            PrintKey(key, name);
            data.DeleteKey(name);
        }else {
            std::cout << "Key not found: \"" << name << "\"." << std::endl;
        }
    }
}

int main(int argc, char *argv[]) {
    std::vector<std::string> args(argv + 1, argv + argc);
    bsmlib::Data data;
    std::ifstream file;
    std::string filename;

    // Command validity check
    if( (args.empty()) ||
        (std::find(args.begin(), args.end(), "--help") != args.end())
    ) {
        PrintHelp();
        return 0;
    }

    // Parse
    auto state = ParseState::InFilename;
    auto curtype = bsmlib::KeyType::Null;
    std::string curname = "";
    std::ifstream infile;
    std::vector<uint8_t> infile_bytes;

    for(int i = 0; i < args.size(); i++) {
        auto &arg = args[i];

        switch(state) {
            case ParseState::InFilename:
                filename = arg;
                file.open(filename);

                // Make sure an action is actually given
                if(i == args.size() - 1) {
                    PrintErr(ToolError::NoGivenAction);
                    file.close();
                    return 1;
                }

                // Load BSM if file exists. Create new BSM if file does not.
                if(args.size() >= 2) {
                    // Does file open?
                    if(file.good()) {
                        // Check for BSM read error
                        if(!data.Load(filename)) {
                            PrintErr(ToolError::BSMReadError, {arg});
                            return 1;
                        }
                    }else {
                        if(args[1] != "set") {
                            PrintErr(ToolError::FileOpenError, {arg});
                            return 1;
                        }
                    }
                }

                // Update state
                state = ParseState::InAction;

                break;
            case ParseState::InAction:
                if(arg == "list") {
                    ListKeys(data, filename);
                    return 0;
                }else if(arg == "dump") {
                    DumpKeys(data, filename);
                    return 0;
                }else if(arg == "get") {
                    GetKeys(data, filename, std::vector(args.begin() + 2, args.end()));
                    return 0;
                }else if(arg == "remove") {
                    RemoveKeys(data, filename, std::vector(args.begin() + 2, args.end()));
                    return 0;
                }else if(arg == "set") {
                    state = ParseState::InKeytype;
                }else {
                    PrintErr(ToolError::UnkownAction, {arg});
                    return 1;
                }
                break;
            case ParseState::InKeytype:
                if(arg == "-i") {       curtype = bsmlib::KeyType::Integer;
                }else if(arg == "-f") { curtype = bsmlib::KeyType::Float;
                }else if(arg == "-s") { curtype = bsmlib::KeyType::String;
                }else if(arg == "-r") { curtype = bsmlib::KeyType::Raw;
                }else {
                    PrintErr(ToolError::InvalidSyntax);
                    return 1;
                }

                state = ParseState::InKeyname;
                break;
            case ParseState::InKeyname:
                curname = arg;
                state = ParseState::InKeyvalue;
                break;
            case ParseState::InKeyvalue:
                switch(curtype) {
                    case bsmlib::KeyType::Integer:  data.SetInt(curname, std::atoi(arg.c_str()));   break;
                    case bsmlib::KeyType::Float:    data.SetFloat(curname, std::atof(arg.c_str())); break;
                    case bsmlib::KeyType::String:   data.SetString(curname, arg);                   break;
                    case bsmlib::KeyType::Raw:
                        // Open file for reading into key value
                        infile.open(arg, std::ios::in | std::ios::binary);

                        // Test file opened
                        if(!infile.good()) {
                            PrintErr(ToolError::FileOpenError, {arg});
                            return 1;
                        }

                        infile_bytes = std::vector<uint8_t>(
                            std::istreambuf_iterator<char>(infile),
                            std::istreambuf_iterator<char>()
                        );

                        data.SetRaw(curname, infile_bytes);

                        // Clear & reset state
                        infile_bytes.clear();
                        state = ParseState::InKeytype;
                        break;
                    default:
                        break;
                }
                break;
            default:
                PrintErr(ToolError::Unknown);
                break;
        }
    }

    data.Save(filename);
    file.close();

    return 0;
}