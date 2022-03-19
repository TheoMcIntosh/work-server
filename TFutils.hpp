#ifndef UTIL_H
#define UTIL_H

std::string hash_to_string(unsigned char * buff) {
    std::stringstream sstring;
    for (int i = 0; i < 32; i++) {
        sstring << std::hex << std::setfill('0') << std::setw(2) << +buff[i];
    }
    return sstring.str();
}

std::string hexStr(unsigned char *data, int len) {
     std::stringstream ss;
     ss << std::hex;

     for( int i(0) ; i < len; ++i )
         ss << std::setw(2) << std::setfill('0') << (int)data[i];

     return ss.str();
}


unsigned char char2int(char input) {
    if (input >= '0' && input <= '9')
        return input - '0';
    if (input >= 'A' && input <= 'F')
        return input - 'A' + 10;
    if (input >= 'a' && input <= 'f')
        return input - 'a' + 10;
    throw std::invalid_argument("Invalid input string");
}

std::vector<unsigned char> hex2bin(const std::string& hex) {
    if (hex.length() % 2) throw std::invalid_argument("hex string must be even!");
    std::vector<unsigned char> binary;
    binary.reserve(hex.length() / 2);
    for (int i = 0; i < hex.length(); i += 2) {
        binary.push_back(char2int(hex[i])*16 + char2int(hex[i+1]));
    }
    return binary;
}

#endif