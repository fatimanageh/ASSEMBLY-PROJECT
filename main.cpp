#include <iostream>
#include <vector>
#include <iomanip>
#include <functional>
#include <cmath>

using namespace std;

#define DRAM_SIZE (64 * 1024 * 1024)
#define CACHE_SIZE (64 * 1024)
#define NUM_ITERATIONS 1000000

enum cacheResType { MISS = 0, HIT = 1 };
const char *msg[2] = {"Miss", "Hit"};

/* The following implements a random number generator */
unsigned int m_w = 0xABABAB55;    /* must not be zero, nor 0x464fffff */
unsigned int m_z = 0x05080902;    /* must not be zero, nor 0x9068ffff */
unsigned int rand_() {
    m_z = 36969 * (m_z & 65535) + (m_z >> 16);
    m_w = 18000 * (m_w & 65535) + (m_w >> 16);
    return (m_z << 16) + m_w;  /* 32-bit result */
}

//Strictly sequential through the entire 64 MB address space
unsigned int memGen1() {
    static unsigned int addr = 0;
    return (addr++) % DRAM_SIZE;
}

//Uniformly random within a 24 KB range (0 … 24 * 1024–1).
unsigned int memGen2() {
    return rand_() % (24 * 1024);
}

//Uniformly random over the full 64 MB.
unsigned int memGen3() {
    return rand_() % DRAM_SIZE;
}

//Strictly sequential but confined to a small 4 KB buffer
unsigned int memGen4() {
    static unsigned int addr = 0;
    return (addr++) % (4 * 1024);
}

//Sequential over 64 KB
unsigned int memGen5() {
    static unsigned int addr = 0;
    return (addr++) % (64 * 1024);
}

//Strided access: each address jumps ahead by 32 bytes, modulo 256 KB 
unsigned int memGen6() {
    static unsigned int addr = 0;
    return (addr += 32) % (64 * 4 * 1024);
}

// ---- Set-Associative Cache Simulation ---- //

// Cache line structure
struct CacheLine {
    bool valid;
    unsigned int tag;
    CacheLine() : valid(false), tag(0) {}
};

// Set-Associative Cache Simulator Class
class CacheSimulator {
public:
    CacheSimulator(unsigned int lineSize, unsigned int ways) {
        this->lineSize = lineSize;
        this->ways = ways;
        numSets = CACHE_SIZE / (lineSize * ways);
        cache.resize(numSets, vector<CacheLine>(ways));
        lruIndex.resize(numSets, 0); // round-robin index
    }

    cacheResType access(unsigned int addr) {
        unsigned int blockAddr = addr / lineSize;
        unsigned int setIndex = blockAddr % numSets;
        unsigned int tag = blockAddr / numSets;

        // Search for hit
        for (auto &line : cache[setIndex]) {
            if (line.valid && line.tag == tag) {
                return HIT;
            }
        }

        // Miss: replace using round-robin
        cache[setIndex][lruIndex[setIndex]].valid = true;
        cache[setIndex][lruIndex[setIndex]].tag = tag;
        lruIndex[setIndex] = (lruIndex[setIndex] + 1) % ways;
        return MISS;
    }

private:
    unsigned int lineSize, ways, numSets;
    vector<vector<CacheLine>> cache;
    vector<unsigned int> lruIndex;
};

// Run a test experiment
void runExperiment(string label, function<unsigned int()> memGen, unsigned int lineSize, unsigned int ways) {
    CacheSimulator sim(lineSize, ways);
    unsigned int hits = 0;
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        unsigned int addr = memGen();
        if (sim.access(addr) == HIT) hits++;
    }
    double hitRatio = 100.0 * hits / NUM_ITERATIONS;
    cout << label << " | Line Size: " << lineSize << " | Ways: " << ways
         << " | Hit Ratio: " << fixed << setprecision(2) << hitRatio << "%\n";
}
int main()
{
    vector<function<unsigned int()>> memGens = { memGen1, memGen2, memGen3, memGen4, memGen5, memGen6 };
    vector<string> labels = { "memGen1", "memGen2", "memGen3", "memGen4", "memGen5", "memGen6" };

    cout << "===== Experiment 1: Varying Line Size (Fixed Sets = 4) =====\n";
    for (int i = 0; i < memGens.size(); i++) {
        for (unsigned int lineSize : {16, 32, 64, 128}) {
            unsigned int ways = CACHE_SIZE / (lineSize * 4); // Fix sets = 4
            runExperiment(labels[i], memGens[i], lineSize, ways);
        }
        cout << "--------------------------\n";
    }

    cout << "\n===== Experiment 2: Varying Ways (Line Size = 64) =====\n";
    for (int i = 0; i < memGens.size(); i++) {
        unsigned int lineSize = 64;
        for (unsigned int ways : {1, 2, 4, 8, 16}) {
            runExperiment(labels[i], memGens[i], lineSize, ways);
        }
        cout << "--------------------------\n";
    }

    return 0;
}

