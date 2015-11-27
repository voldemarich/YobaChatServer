#include <iostream>
#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>

using namespace std;

string streamBufferToString(boost::asio::streambuf* n)
{
    string s((istreambuf_iterator<char>(n)),
             istreambuf_iterator<char>());
    return s;
}

void readStringToStreamBuffer(string* s, boost::asio::streambuf* n){
    ostream os(n);
    os << *s;
}

string genRandomAlphanumericStr(int len) {
    string chars(
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "1234567890");
    boost::random::random_device rng;
    boost::random::uniform_int_distribution<> index_dist(0, chars.size() - 1);
    string a = "";
    for(int i = 0; i < len; ++i) {
         a += chars[index_dist(rng)];
    }
    return a;
}
