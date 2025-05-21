#include <iostream>
#include <vector>
#include <iomanip>
#include <functional>
#include <cmath>
#include <string>

using namespace std;

#define DRAM_SIZE (64 * 1024 * 1024)
#define CACHE_SIZE (64 * 1024)
#define NUM_ITERATIONS 1000000

enum cacheResType { MISS = 0, HIT = 1 };
const char* msg[2] = { "Miss", "Hit" };

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

//Uniformly random within a 24 KB range (0 … 24 * 1024–1).
unsigned int memGen2() {
    return rand_() % (24 * 1024);
}

//Uniformly random over the full 64 MB.
unsigned int memGen3() {
    return rand_() % DRAM_SIZE;
}

//Strictly sequential but confined to a small 4 KB buffer
unsigned int memGen4() {
    static unsigned int addr = 0;
    return (addr++) % (4 * 1024);
}

//Sequential over 64 KB
unsigned int memGen5() {
    static unsigned int addr = 0;
    return (addr++) % (64 * 1024);
}

//Strided access: each address jumps ahead by 32 bytes, modulo 256 KB 
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
        lruCounter.resize(numSets, 0); // Initialize LRU counter for each set
    }

    cacheResType access(unsigned int addr) {
        unsigned int blockAddr = addr / lineSize;
        unsigned int setIndex = blockAddr % numSets;
        unsigned int tag = blockAddr / numSets;

        // Search for hit
        for (unsigned int i = 0; i < ways; i++) {
            if (cache[setIndex][i].valid && cache[setIndex][i].tag == tag) {
                return HIT;
            }
        }

        // Miss: find an invalid line or evict the LRU line
        unsigned int replaceIndex = ways; // Default to invalid value

        // First, try to find an invalid line
        for (unsigned int i = 0; i < ways; i++) {
            if (!cache[setIndex][i].valid) {
                replaceIndex = i;
                break;
            }
        }

        // If all lines are valid, use the round-robin counter
        if (replaceIndex == ways) {
            replaceIndex = lruCounter[setIndex];
            lruCounter[setIndex] = (lruCounter[setIndex] + 1) % ways;
        }

        // Replace the selected line
        cache[setIndex][replaceIndex].valid = true;
        cache[setIndex][replaceIndex].tag = tag;

        return MISS;
    }

private:
    unsigned int lineSize, ways, numSets;
    vector<vector<CacheLine>> cache;
    vector<unsigned int> lruCounter; // For round-robin replacement
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

//We developed this function to make sure our simulator is working correctly
void testCorrectness(string label, const vector<unsigned int>& addresses, unsigned int lineSize, unsigned int ways, unsigned int expectedHits, unsigned int expectedMisses) {
    CacheSimulator sim(lineSize, ways);
    unsigned int hits = 0, misses = 0;

    cout << "Test details for configuration: " << label << "\n";

    for (size_t i = 0; i < addresses.size(); i++) {
        unsigned int addr = addresses[i];
        cacheResType result = sim.access(addr);
        if (result == HIT) hits++;
        else misses++;

        unsigned int block = addr / lineSize;
        unsigned int set = block % (CACHE_SIZE / (lineSize * ways));
        cout << "  Access #" << (i + 1) << ": Address " << addr
            << " -> " << msg[result] << " (Set: " << set << ")\n";

    }

    cout << "[Test Summary] " << label
        << " | Line Size: " << lineSize
        << " | Ways: " << ways
        << " | Hits: " << hits << " (Expected: " << expectedHits << ")"
        << " | Misses: " << misses << " (Expected: " << expectedMisses << ")"
        << " | Result: " << ((hits == expectedHits && misses == expectedMisses) ? "PASS" : "FAIL") << "\n";

}

void runCustomTests() {
    cout << "\n===== Custom Line Size Tests =====\n";

    vector<unsigned int> lineSizes = { 16, 32, 64, 128 };
    unsigned int waysFixed = 4;

    for (unsigned int lineSize : lineSizes) {
        vector<unsigned int> addresses;
        for (unsigned int i = 0; i < lineSize; ++i)
            addresses.push_back(i);
        string label = "LineSize=" + to_string(lineSize);
        testCorrectness(label, addresses, lineSize, waysFixed, lineSize - 1, 1);
    }

    cout << "\n===== Custom Way Associativity Tests (Eviction) =====\n";

    unsigned int lineSize = 64;
    vector<unsigned int> waysList = { 1, 2, 4, 8 };

    for (unsigned int ways : waysList) {
        unsigned int numSets = CACHE_SIZE / (lineSize * ways);
        unsigned int stride = numSets * lineSize;

        vector<unsigned int> addresses;
        // Generate (ways + 1) addresses that map to same set (here set 0)
        for (unsigned int i = 0; i <= ways; ++i)
            addresses.push_back(i * stride);

        // Re-access the first address to test if it was replaced
        addresses.push_back(0);

        string label = "Ways=" + to_string(ways);
        testCorrectness(label, addresses, lineSize, ways, 0, ways + 2);
    }
}

int main() {
    //comment or uncomment this function to see the output of our testing
    runCustomTests();
    //this part is for experiments 1 and 2
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
