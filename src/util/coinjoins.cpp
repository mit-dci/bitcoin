// Copyright (c) 2024 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include <iostream>

#include <coins.h>
#include <consensus/validation.h>
#include <util/coinjoins.h>
#include <undo.h>

bool WhirlpoolTransactions::isWhirlpool(const CTransactionRef& tx) {
    if (tx->vin.size() == 5 && tx->vout.size() == 5) {
        CAmount amount = tx->vout.at(0).nValue;

        // These are the only whirlpool pools
        if (amount != 5000000 && amount != 1000000 && amount != 50000000) {
            return false;
        }

        for (const auto& tx_out : tx->vout) {
            if (tx_out.nValue != amount) return false;
        }

        for (const CTxIn& tx_in : tx->vin) {
            if (cj_transactions.contains(tx_in.prevout.hash)) return true;
        }
        return false;
    }

    return false;
}

// getting some errors in CFeerate initialization (division by zero?) so displaying raw fees for now
void GetMedianFeeRateFromBlock(const CBlock& block, const CBlockUndo &undo, int block_height) {
     // 1 skips coinbase
     std::cout << "all fees in block of height=" << block_height << ":";
    for(size_t txindex = 1; txindex < block.vtx.size(); txindex++) {
      const auto& tx = block.vtx[txindex];
      // vtxundo is offset by 1 because the coinbase tx is not present.
      const auto& undotx = undo.vtxundo[txindex - 1];
      CAmount value_in = 0;
      for (const auto& prevout : undotx.vprevout) {
           value_in += prevout.out.nValue;
      }
      CAmount value_out = tx->GetValueOut();
      // tx->GetHash().ToString()
      std::cout << " " << (value_in - value_out);
    }
     std::cout << std::endl;
}

void WhirlpoolTransactions::Update(const CTransactionRef& tx, int block_height) {
    if (isWhirlpool(tx)) {
        cj_transactions.insert(tx->GetHash());
        for (const CTxIn& tx_in : tx->vin) {
            if (!cj_transactions.contains(tx_in.prevout.hash)) {
	      tx0s.Update(tx_in.prevout.hash, tx->vout.at(0).nValue);
            }
        }
    }
}
