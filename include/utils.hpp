#include <iostream>

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
