#include "Block.h"
#include "sha256.h"
#include <klib-macros.h>
#include <klib.h>

void Block::block_init(uint32_t nIndexIn, const char *sDataIn,
                       const char *sPrevHashIn) {
  _nIndex = nIndexIn;
  _sData = sDataIn;
  _sPrevHash = sPrevHashIn;
  _nNonce = -1;
  _tTime = io_read(AM_TIMER_UPTIME).us / 1000000;
}

const char *Block::GetHash() { return _sHash; }

void Block::MineBlock(const char *diffStr) {
  int nDifficulty = strlen(diffStr);
  do {
    _nNonce++;
    _CalculateHash(_sHash);
  } while (memcmp(_sHash, diffStr, nDifficulty) != 0);

  printf("Block mined:%s\n", _sHash);
}

inline void Block::_CalculateHash(char *buf) {
  static char str[1024];
  sprintf(str, "%d%d%s%d%s", _nIndex, _tTime, _sData, _nNonce, _sPrevHash);
  sha256(buf, str);
  sha256(buf, buf);
}
