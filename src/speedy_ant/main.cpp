#include <string>
#include <iostream>
#include <set>
#include <map>
#include <algorithm>
#include <chrono>
#include <memory>

/*
Baseline:
Step 4999999: (-95976,-95952) facing south on a white square. Bounding box: -95979, -95958 -> 29, 22
Done. 5000000 iterations in 3014ms, or 1.65892e+07 it/s.
*/

/*
Baseline + improved running with crosschecks.
Step 4999999: (-95976,-95952) facing south on a black square.   Bounding box: -95979, -95958 -> 29, 22   Supertile: -188, -375   Tile: 7, 6   Offset in Tile: 8, 15   Word: 6383   Bit: 8   Word in map: 00000000000000000000111000100110
Done. 5000000 iterations in 3614ms, or 1.38351e+07 it/s.
*/

/*
Improved implementation with baseline stripped out.
Step 4999999: (-95976,-95952) facing south on a black square.   Bounding box: -95979, -95958 -> 29, 22   Supertile: -188, -375   Tile: 7, 6   Offset in Tile: 8, 15   Word: 6383   Bit: 8   Word in map: 00000000000000000000111000100110
Done. 5000000 iterations in 477ms, or 1.04822e+08 it/s.
*/

/*
Use raw bitwise ops instead of std::bitset
Step 4999999: (-95976,-95952) facing south on a black square.   Bounding box: -95979, -95958 -> 29, 22   Supertile: -188, -375   Tile: 7, 6   Offset in Tile: 8, 15   Word: 6383   Bit: 8   Word in map: 00000000000000000000111000100110
Done. 5000000 iterations in 339ms, or 1.47493e+08 it/s.
*/

/*
Don't bother maintaining the bounding box. 
Step 4999999: (-95976,-95952) facing south on a black square.   Supertile: -188, -375   Tile: 7, 6   Offset in Tile: 8, 15   Word: 6383   Bit: 8   Word in map: 00000000000000000000111000100110
Done. 5000000 iterations in 219ms, or 2.2831e+08 it/s.
*/

/*
Strategy:

Allocate board space in square tiles the size of a cache line, which on my M2 is 128 bytes (though it's probably 64 bytes on an AMD64 machine). Each tile is square, or as close as possible. 128 bytes is 1024 bits, or 32*32 squares. This is enough space to cover most of the bounding box for the first several thousand steps. Further, allocate tiles in supertile increments matching the page size, ensuring that we minimize cache misses that go all the way to main memory. That 16384 bytes on the M2, or 128 tiles, or 8*16 tiles of 32*32 bits each.

hw.cachelinesize: 128
hw.l1dcachesize: 65536
hw.l2cachesize: 4194304
hw.pagesize: 16384

Start by allocating an aligned 16384 byte supertile, and arranging the space within it to minimize cache misses, first at the cache line level and then at the page level. Each tile spans a certain spatial bounding box, and when we need to go outside that box we'll allocate a new page. There should be no advantage in managing allocations above the page size, since we don't care about contiguity. We can, for instance, use a normal map to manage supertiles.

One tile:  128 bytes wrapped to 32x32 bits    (each x is a bit)

                    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x   
                    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x   
                    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x   
                    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x   
                    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x   
                    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x   
                    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x   
                    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x   
                    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x   
                    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x   
                    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x   
                    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x   
                    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x   
                    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x   
                    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x   
                    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x   
                    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x   
                    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x   
                    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x   
                    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x   
                    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x   
                    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x   
                    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x   
                    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x   
                    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x   
                    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x   
                    x  o  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x   
H_TILE              x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x   
  ^                 x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x   
  |                 x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x   
  |                 x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x   
  x --> W_TILE      x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x   


One supertile: 16384 bytes wrapped to 16x8 tiles    (each X is a tile)

                    X X X X X X X X  X X X X X X X X
                    X X X X X X X X  X X X X X X X X
                    X X X X X X X X  X X X X X X X X
                    X X X X X X X X  X X X X X X X X
                    X X X X X X X X  X X X X X X X X
                    X X X X X X X X  X X X X X X X X
                    X O X X X X X X  X X X X X X X X


This supertile is h_st = 8x32 = 256 bits tall and w_st = 16x32 = 512 bits wide.
Tiles within are h_t = 32 bits and w_t = 32 bits

We want to map this onto Cartesian space. What is the location in memory of the Cartesian point (x, y)? 
It's located on supertile (floor(x / w_st), floor(y / h_st))
    within that supertile it's on tile at location (floor((x % w_st) / w_t), floor((y % h_st) / h_t)).
    within that tile it's ?

Example: Cartesian point (2342, 12345) 
(Indicated in the above map)

Supertile is (2342/512, 12345/256) = (4, 48)
Offset within supertile is (2342%512, 12345%256) = (294, 57)
Tile is (294/32, 57/32) = (9, 1)
Position within tile is (294%32, 57%32) = (6, 25)
int32 offset in tile is (floor(6/32)) * (32/32) + 25 * (32/32) = 25
bit offset within word is 6%32 = 6


supertile_address = 4, 48
tile_address = 9, 1
point_address = 6, 25

Offset within tile buffer is (9*w_st + 1*h_st) + (6*w_t + 25*h_t) = (4608 + 256) + (3072 + 800) = 4864 + 3872 = 8736

Offset within tile buffer is 294 * 32 + 57, or 9465.
*/

using Word = uint32_t;

constexpr int64_t PAGE_SIZE = 16384;
constexpr int64_t PAGE_SIZE_WORDS = PAGE_SIZE / sizeof(Word);

constexpr int64_t CACHE_LINE_SIZE = 128;

constexpr int64_t W_TILE_WORDS = 1;                         // simplest if this just stays 1, and that's fine for any common cache line size
constexpr int64_t W_TILE = 8 * W_TILE_WORDS * sizeof(Word); // should always be size of word in bits
constexpr int64_t H_TILE_WORDS = (CACHE_LINE_SIZE / W_TILE_WORDS) / sizeof(Word);
constexpr int64_t H_TILE = CACHE_LINE_SIZE * 8 / W_TILE;    // W_TILE * H_TILE is the size of a cache line

constexpr int64_t TILE_WORDS = W_TILE_WORDS * H_TILE_WORDS;

constexpr int64_t W_SUPERTILE_TILES = 16; 
constexpr int64_t H_SUPERTILE_TILES = (PAGE_SIZE / W_SUPERTILE_TILES) / CACHE_LINE_SIZE;

constexpr int64_t W_SUPERTILE = W_SUPERTILE_TILES * W_TILE;
constexpr int64_t H_SUPERTILE = H_SUPERTILE_TILES * H_TILE;



constexpr size_t SupertileWordCount = W_SUPERTILE/sizeof(Word) * H_SUPERTILE;
using Supertile = std::array<Word, SupertileWordCount>;

using TileAddress = std::pair<int64_t, int64_t>;
using XYPair = std::pair<int64_t, int64_t>;
using SuperTileMap = std::map<TileAddress, std::unique_ptr<Supertile>>;

TileAddress supertileAddress(const XYPair& loc) {
    return {std::floor(loc.first / W_SUPERTILE), std::floor(loc.second / H_SUPERTILE)};
} 

enum Direction {
    N,
    E,
    S,
    W
};

const char* directionName(uint8_t d) {
    switch (d) {
        case 0:
            return "north";
            break;
        case 1:
            return "east";
            break;
        case 2:
            return "south";
            break;
        case 3:
            return "west";
            break;
        default:
            return "invalid direction";     
        }
}

const char* colorName(bool color) {
    return color ? "black" : "white";
}






class AntMap {

public:

    bool getAndSet(const XYPair& loc) {
        
        currSupertileAddress = {
            std::floor(float(loc.first) / W_SUPERTILE), 
            std::floor(float(loc.second) / H_SUPERTILE)
        };

        tileAddressInSupertile = {
            std::floor(std::abs(loc.first % W_SUPERTILE) / W_TILE), 
            std::floor(std::abs(loc.second % H_SUPERTILE) / H_TILE)
        };

        addressInTile = {
            std::abs(loc.first % W_SUPERTILE) % W_TILE,
            std::abs(loc.second % H_SUPERTILE) % H_TILE
        };

        wordAddressInSupertile = 
            (tileAddressInSupertile.first * TILE_WORDS + 
            tileAddressInSupertile.second * TILE_WORDS * TILE_WORDS) // offset to tile base
            + std::abs(loc.second % H_SUPERTILE) % H_TILE;
        
        bitAddressInWord = addressInTile.first;

        Word mask = 0x1 << bitAddressInWord;

        if (currSupertileAddress != lastSupertileAddress) {
            // try swapping with the cached supertile in case that fits better
            std::swap(lastSupertileAddress, cachedSupertileAddress);
            std::swap(currentSuperTile, cachedSupertile);
            if (currSupertileAddress != lastSupertileAddress) {
                lastSupertileAddress = currSupertileAddress;
                auto existingTileIt = supertiles.find(currSupertileAddress);
                if (existingTileIt == supertiles.end()) {
                    supertiles[currSupertileAddress] = std::make_unique<Supertile>();
                    auto newTileIt = supertiles.find(currSupertileAddress);
                    currentSuperTile = newTileIt->second.get();
                    std::memset(currentSuperTile, 0, SupertileWordCount);
                    superTilesCreated++;
                } else {
                    currentSuperTile = existingTileIt->second.get();
                    superTilesRetrieved++;
                }
            } else {
                superTilesRetrieveAvoided++;
            }
        }

        wordInMap = currentSuperTile->data()[wordAddressInSupertile];
        bool color = wordInMap & mask;
        wordInMap ^= mask;
        currentSuperTile->data()[wordAddressInSupertile] = wordInMap;

        return color;
    }


    // map of allocated supertiles, keyed on TileAddress
    SuperTileMap supertiles;

    // supertile address of current ant location
    TileAddress currSupertileAddress  = {0,0};
    // pointer to supertile of current ant location
    Supertile* currentSuperTile;

    // the supertile address of the last ant location
    TileAddress lastSupertileAddress = {INT32_MAX, INT32_MAX};

    // a cached supertile pointer to avoid repeated lookups when the ant is meandering near a supertile boundary
    TileAddress cachedSupertileAddress = {INT32_MAX, INT32_MAX};
    Supertile* cachedSupertile;

    // nasty public leftovers, here to make logging easy
    TileAddress tileAddressInSupertile;
    TileAddress addressInTile;
    uint64_t wordAddressInSupertile;
    size_t bitAddressInWord;
    Word wordInMap;

    size_t superTilesCreated = 0;
    size_t superTilesRetrieved = 0;
    size_t superTilesRetrieveAvoided = 0;
};




void gogoant(uint64_t n, XYPair saltLoc) {
    using namespace std::chrono;

    std::cout << "Running ant for " << n << " iterations. Placing salt at (" << saltLoc.first << ", " << saltLoc.second << ")." << std::endl;

    uint64_t start = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

    // current ant position, direction, and color
    XYPair loc = {0,0};
    uint8_t dir = 0;
    bool color = 0;

    AntMap map;

    // place the salt
    map.getAndSet(saltLoc);

    for (uint64_t i = 1; i <= n; i++) {

        color = map.getAndSet(loc);

        if (i % 1000000 == 0 || i == n) {
            std::cout << "Step " << i << ": (" << loc.first << "," << loc.second << ") facing ";
            std::cout << directionName(dir) << " on a " << colorName(color) << " square.";
            std::cout << "   Supertile: " << map.currSupertileAddress.first << ", " << map.currSupertileAddress.second;
            std::cout << "   Tile: " << map.tileAddressInSupertile.first << ", " << map.tileAddressInSupertile.second;
            std::cout << "   Offset in Tile: " << map.addressInTile.first << ", " << map.addressInTile.second;
            std::cout << "   Word: " << map.wordAddressInSupertile;
            std::cout << "   Bit: " << map.bitAddressInWord;
            std::cout << "   Word in map: " << std::bitset<32>(map.wordInMap) << std::endl;
        }

        if (color) {
            dir -= 1;
        } else {
            dir += 1;
        }
        dir = dir % 4;

        switch (dir) {
        case 0:
            loc.second++;
            break;
        case 1:
            loc.first++;
            break;
        case 2:
            loc.second--;
            break;
        case 3:
            loc.first--;
            break;     
        }
    }

    uint64_t end = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

    float speed = (n*10000.0) / (end - start);
    std::cout << "Done. " << n << " iterations in " << end - start << "ms, or " << speed << " it/s." << std::endl;
    std::cout << "Supertiles created: " << map.superTilesCreated << "  retrieved: " << map.superTilesRetrieved << "  avoided retrievals: " << map.superTilesRetrieveAvoided << std::endl;
}

int main(int argc, char** argv) {

    uint64_t n = 1000000;
    XYPair saltLoc = {1000, 1000};  // default salt will never be encountered

    if (argc > 1) {
        n = atol(argv[1]);
    }
    if (argc > 3) {
        saltLoc.first = atol(argv[2]);
        saltLoc.second = atol(argv[3]);
    }

    std::cout << "CACHE_LINE_SIZE: " << CACHE_LINE_SIZE << std::endl;
    std::cout << "PAGE_SIZE: " << PAGE_SIZE << std::endl;
    std::cout << "PAGE_SIZE_WORDS: " << PAGE_SIZE_WORDS << std::endl;
    std::cout << "TILE_WORDS: " << TILE_WORDS << std::endl;
    std::cout << "W_SUPERTILE_TILES: " << W_SUPERTILE_TILES << std::endl;
    std::cout << "H_SUPERTILE_TILES: " << H_SUPERTILE_TILES << std::endl;
    std::cout << "W_SUPERTILE: " << W_SUPERTILE << std::endl;
    std::cout << "H_SUPERTILE: " << H_SUPERTILE << std::endl;
    std::cout << "W_TILE_WORDS: " << W_TILE_WORDS << std::endl;
    std::cout << "W_TILE: " << W_TILE << std::endl;
    std::cout << "H_TILE_WORDS: " << H_TILE_WORDS << std::endl;
    std::cout << "H_TILE: " << H_TILE << std::endl;

    gogoant(n, saltLoc);

    return 0;
}