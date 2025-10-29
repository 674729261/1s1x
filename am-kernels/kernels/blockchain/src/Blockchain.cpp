#include "Blockchain.h"
#include <klib.h>

Blockchain::Blockchain(int difficulty) {
  _vChain[0].block_init(0, "Genesis Block", "");
  _nrBlock = 1;
  assert(difficulty <= MAX_DIFFICULTY);
  memset(_diffStr, '0', difficulty);
  _diffStr[difficulty] = '\0';
}

void Blockchain::AddBlock(uint32_t nIndexIn, const char *sDataIn) {
  assert(_nrBlock < MAXBLOCK);
  Block *b = &_vChain[_nrBlock];
  b->block_init(nIndexIn, sDataIn, _vChain[_nrBlock - 1].GetHash());
  b->MineBlock(_diffStr);
  _nrBlock++;
}
