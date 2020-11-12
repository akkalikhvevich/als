#include <iostream>
#include <map>
#include <fstream>

class CukooFilter {
public:
    static constexpr int p = 2539;
    static constexpr int q = 2293;
    static constexpr int t = 2749;
    static constexpr int maxKicks = 50;
    static constexpr int maxFinger = 128;
    static constexpr double fpr = 0.06;
    static constexpr int b = 4;
    int n;
    int m;

    class Bucket;

    Bucket **buckets = new Bucket *[m];

    static int getSize(int num) {
        int sz = 1;
        double bound = num * (fpr + 1);
        while (sz < bound) {
            sz = sz << 1;
        }
        return sz;
    };

    CukooFilter(int n) : m(getSize(n)), n(n) {

        for (int i = 0; i < m; ++i) {
            buckets[i] = new Bucket();
        }
    }


    template<typename T>
    size_t getHash(T x) {
        return std::hash<T>{}(x) % m;
    }

    bool insert(std::string id) {
        char fing = fingerprint(id);
        int hash1 = getHash(id);
        int hash2 = hash1 ^getHash(fing);
        if (buckets[hash1]->insert(fing))
            return true;
        else if (buckets[hash2]->insert(fing))
            return true;
        int num;
        if (std::rand() % 2) { num = hash1; }
        else { num = hash2; }
        for (int i = 0; i < maxKicks; ++i) {
            fing = buckets[num]->swap(fing);
            num = num ^ getHash(fing);
            if (buckets[num]->insert(fing))
                return true;
        }
        return false;
    }

    bool lookup(std::string id) {
        char fing = fingerprint(id);
        int hash1 = getHash(id);
        int hash2 = hash1 ^getHash(fing);
        return (buckets[hash1]->lookup(fing) || buckets[hash2]->lookup(fing));
    }

    static char fingerprint(std::string& id) {
        char hash = 0;
        int size = 0;
        for (char i : id) {
            size += i;
        }
        for (int i = 0; i < 7; ++i) {
            size = (size * size * p + q) % t;
            char bit = size % 2;
            hash = (hash << 1) | bit;
        }
        return (char)((hash + maxFinger) % maxFinger);
    }

    ~CukooFilter() {
        for (size_t i = 0; i < m; i++) {
            delete buckets[i];
        }
        delete[] buckets;
    }

    CukooFilter(const CukooFilter &other) : n(other.n), m(other.m) {
        for (size_t i = 0; i < m; i++) {
            buckets[i] = new Bucket(*(other.buckets[i]));
        }
    }

    class Bucket {
    public:
        char bucket[b];
        size_t count = 0;

        Bucket() {
        }

        Bucket(const Bucket &other) : count(other.count) {
            for (size_t i = 0; i < b; i++) {
                bucket[i] = other.bucket[i];
            }
        }

        char swap(char c) {
            int index = std::rand() % count;
            char out = bucket[index];
            bucket[index] = c;
            return out;
        }

        bool insert(const char c) {
            if (lookup(c)) { return true; }
            if (count < b) {
                bucket[count++] = c;
                return true;
            }
            return false;
        }

        bool lookup(const char c) {
            for (int i = 0; i < count; ++i) {
                if (c == bucket[i])
                    return true;
            }
            return false;
        }

    };
};


std::ifstream fin;
std::ofstream fout;
std::ofstream dbg;

void watchVideo(std::map<std::string, CukooFilter> &filter, std::string &id, int n, std::string &video) {
    if (filter.count(id)) {
        filter.find(id)->second.insert(video);
    } else {
        CukooFilter newUser(n);
        newUser.insert(video);
        filter.insert({id, newUser});
    }
    fout << "Ok" << std::endl;
}

void checkVideo(std::map<std::string, CukooFilter> &filter, std::string &id, std::string &video) {
    if (filter.count(id) && (filter.find(id)->second.lookup(video))) {
        fout << "Probably" << std::endl;
    } else {
        fout << "No" << std::endl;
    }
}

int main(int argc, char *argv[]) {
    fin = std::ifstream(argv[1]);
    fout = std::ofstream(argv[2]);
    dbg = std::ofstream("../dbg.txt");


    std::string video, id, type;
    int n;

    fin >> video >> n;
    fout << "Ok" << std::endl;

    std::map<std::string, CukooFilter> filter;

    while (fin >> type >> id >> video) {
        dbg << type << " " << id << " " << video << "\n";
        if (type == "watch") {
            watchVideo(filter, id, n,video);
        } else if (type == "check") {
            checkVideo(filter, id, video);
        }
    }
}


