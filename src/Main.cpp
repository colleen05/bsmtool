#include <bsmlib.hpp>
#include <iostream>
#include <filesystem>

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

void PrintVersion(bool verbose = false) {
    std::cout << "bsmtool v1.0.0 by Colleen (colleen05 on GitHub)." << std::endl;

    if(verbose) {
        std::cout << "Based on bsmlib version: 1.5.0." << std::endl;
        std::cout << "This software is distributed under the MIT license." << std::endl;
    }
}

void PrintHelp() {
    PrintVersion();
    std::cout
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
        << "\t-r <name> <file>     Set raw value using bytes from given file." << std::endl
        << std::endl
        << "Universal options (other arguments will be ignored):" << std::endl
        << "\t--help or -h       Display help." << std::endl
        << "\t--version or -v    Display version info." << std::endl;
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
            std::cerr << "No given action for file \"" << args[0] << "\"." << std::endl;
            break;
        case ToolError::InvalidSyntax:
            if(args.empty()) {
                std::cerr << "Invalid syntax." << std::endl;
            }else {
                std::cerr << "Invalid syntax: " << args[0] << std::endl;
            }
            break;
        default:
            std::cerr << "UNKOWN ERROR. THIS IS A BUG.";
            break;
    }

    std::cout << "Use option '--help' or '-h' for help with using bsmtool." << std::endl;
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
        (std::find(args.begin(), args.end(), "--help") != args.end()) ||
        (std::find(args.begin(), args.end(), "-h") != args.end()) ) {

        PrintHelp();
        return 0;
    }else if( (args.empty()) ||
        (std::find(args.begin(), args.end(), "--version") != args.end()) ||
        (std::find(args.begin(), args.end(), "-v") != args.end()) ) {

        PrintVersion(true);
        return 0;
    }

    // Parse
    auto state = ParseState::InFilename;
    auto curtype = bsmlib::KeyType::Null;
    std::string curname = "";
    std::ifstream infile;
    std::vector<uint8_t> infile_bytes;
    bsmlib::Key tmpkey;

    for(int i = 0; i < args.size(); i++) {
        auto &arg = args[i];

        switch(state) {
            case ParseState::InFilename:
                filename = arg;
                file.open(filename);

                // Load BSM if file exists. Create new BSM if file does not.
                if(args.size() >= 2) {
                    // Does file open?
                    if(file.good()) {
                        // Check for BSM read error
                        if(!data.Load(filename)) {
                            PrintErr(ToolError::BSMReadError, {arg});
                            file.close();
                            return 1;
                        }
                    }else {
                        if(args[1] != "set") {
                            PrintErr(ToolError::FileOpenError, {arg});
                            file.close();
                            return 1;
                        }
                    }
                }else {
                    // Does file not open?
                    if(!file.good()) {
                        PrintErr(ToolError::FileOpenError, {arg});
                        file.close();
                        return 1;
                    }
                }

                // Make sure an action is actually given
                if(i == args.size() - 1) {
                    PrintErr(ToolError::NoGivenAction, {arg});
                    file.close();
                    return 1;
                }

                // Update state & close file
                state = ParseState::InAction;
                file.close();

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
                    data.Save(filename);
                    return 0;
                }else if(arg == "set") {
                    state = ParseState::InKeytype;
                }else {
                    PrintErr(ToolError::UnkownAction, {arg});
                    return 1;
                }
                break;
            case ParseState::InKeytype:
                // Look for keytype
                if(arg == "-i") {       curtype = bsmlib::KeyType::Integer;
                }else if(arg == "-f") { curtype = bsmlib::KeyType::Float;
                }else if(arg == "-s") { curtype = bsmlib::KeyType::String;
                }else if(arg == "-r") { curtype = bsmlib::KeyType::Raw;
                }else {
                    PrintErr(ToolError::InvalidSyntax, {"Unknown key type option, \"" + arg + "\"."});
                    return 1;
                }

                // Make sure a keyname is actually given.
                if(i == args.size() - 1) {
                    PrintErr(ToolError::InvalidSyntax, {"Key name was not given."});
                    return 1;
                }

                state = ParseState::InKeyname;
                break;
            case ParseState::InKeyname:
                // Make sure a value is actually given
                if(i == args.size() - 1) {
                    PrintErr(ToolError::InvalidSyntax, {"No value given for key, \"" + arg + "\"."});
                    return 1;
                }

                // Set key name
                curname = arg;
                state = ParseState::InKeyvalue;
                break;
            case ParseState::InKeyvalue:
                // Write data to structure
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
                            infile.close();
                            return 1;
                        }

                        infile_bytes = std::vector<uint8_t>(
                            std::istreambuf_iterator<char>(infile),
                            std::istreambuf_iterator<char>()
                        );

                        data.SetRaw(curname, infile_bytes);

                        // Clear & close file
                        infile_bytes.clear();
                        infile.close();
                        break;
                    default:
                        break;
                }

                tmpkey = data.GetKey(curname);
                std::cout << "SET: ";
                PrintKey(tmpkey, curname);

                // Advance state if more arguments are given
                if(i != args.size() - 1) {
                    state = ParseState::InKeytype;
                }
                break;
            default:
                PrintErr(ToolError::Unknown);
                break;
        }
    }

    data.Save(filename);

    return 0;
}