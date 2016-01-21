#include <iostream>
#include <thread>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>

#include <cpr/cpr.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/range/irange.hpp>

// A lot of the following will be removed with a config file or passed in from
// the command line ultimately

static const int num_threads = 50;
const char* size = "10240";
const char* host = "myhost.mydomain.com";
const char* port = "8080";
const char* account = "myaccount";
const char* user = "myusername";
const char* password = "mypassword";
const char* container = "mycontainer";
std::vector<char> ipayload;
auto credentials = std::string(account) + ":" + std::string(user);
auto url = std::string(host) + ":" + std::string(port) + "/auth/v1.0";

std::string ihexbyte() {
    std::random_device r;
    std::uniform_int_distribution<int> dist(0,255);
    std::stringstream ss;
    ss << std::hex << std::setfill('0') << std::setw(2);
    ss << dist(r);
    return ss.str();
}

void iwork() {
    auto r = cpr::Get(cpr::Url{url},
             cpr::Header{{"X-Storage-User", credentials},
                         {"X-Storage-Pass", password}});
    auto auth_token = r.header["X-Auth-Token"];

    while(true)
    {
        std::stringstream ssmd5;
        std::generate_n(std::ostream_iterator<std::string>(ssmd5), 16, ihexbyte);
        std::string sr1(ihexbyte());
        std::string sr2(ihexbyte());
        auto nuuid = boost::uuids::random_generator()();
        std::stringstream path;
        path << host << ":" << port << "/v1/AUTH_" << account << "/" << container
            << "/" << sr1 << "/" << sr2 << "/" << boost::uuids::to_string(nuuid)
            << "/" << ssmd5.str() << ".dat";

        auto rtoo = cpr::Put(cpr::Url{path.str()},
                cpr::Header{{"X-Auth-Token", auth_token}, {"Content-Length", size}},
                cpr::Body{&ipayload[0], boost::lexical_cast<size_t>(size)});

        std::cout << rtoo.url << std::endl;
        std::cout << rtoo.status_code << std::endl;
        for(auto iiter : rtoo.header)
            std::cout << iiter.first << ": " << iiter.second << std::endl;
        std::cout << std::endl;
    }

}

int main(int argc, char** argv) {

    ipayload.resize(boost::lexical_cast<size_t>(size));
    std::ifstream urandom("/dev/urandom", std::ios::in|std::ios::binary);
    urandom.read(&ipayload.front(), boost::lexical_cast<size_t>(size));
    urandom.close();

    std::vector<std::thread> threads;
    for (auto i : boost::irange(0, num_threads))
       threads.push_back(std::thread(iwork));
    for(auto& thread : threads)
        thread.join();

    return 0;
}
