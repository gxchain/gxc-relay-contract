#include <graphenelib/asset.h>
#include <graphenelib/contract.hpp>
#include <graphenelib/contract_asset.hpp>
#include <graphenelib/global.h>
#include <graphenelib/multi_index.hpp>
#include <graphenelib/system.h>
#include <graphenelib/dispatcher.hpp>
#include <vector>

using namespace graphene;

class relay : public contract
{
public:
    relay(uint64_t account_id)
        : contract(account_id), records_table(_self, _self), Eth_confirm_table(_self, _self), Eth_withdraw_table(_self, _self)
    {
    }

    // @abi action
    // @abi payable
    void deposit(std::string target, std::string addr)
    {
        int64_t asset_amount = get_action_asset_amount();
        uint64_t asset_id = get_action_asset_id();
        graphene_assert(asset_id == 1, "Only support GXC ");
        graphene_assert(asset_amount >= MIN_DEPOSIT, "Must greater than minnumber ");
        contract_asset amount{asset_amount, asset_id};
        uint64_t id_number = records_table.available_primary_key();
        auto coin_kind = find(TARGETS.begin(), TARGETS.end(), target);
        graphene_assert(coin_kind != TARGETS.end(), "invalid chain name");
        uint64_t sender = get_trx_sender();
        records_table.emplace(sender, [&](auto &o) {
            o.id = id_number;
            o.from = sender;
            o.asset_id = asset_id;
            o.amount = asset_amount;
            o.target = target;
            o.to = addr;
            o.state = 0;
        });
    }

    //@abi action
    void withdraw(std::string to_account, contract_asset amount, std::string from_target, std::string txid, std::string from_account)
    {
        int64_t account_id = get_account_id(to_account.c_str(), to_account.size());
        uint64_t sender = get_trx_sender();
        auto coin_kind = find(TARGETS.begin(), TARGETS.end(), from_target);
        graphene_assert(amount.asset_id == 1, "Only support GXC");
        graphene_assert(amount.amount >= MIN_WITHDRAW, "Must greater than min number");
        graphene_assert(coin_kind != TARGETS.end(), "Invalid target");
        graphene_assert(sender == adminAccount, "No authority");
        graphene_assert(account_id >= 0, "Invalid account_name to_account");
        graphene_assert(amount.amount > 0, "Invalid amount");
        if (from_target == "ETH")
        {
            auto txid_uint = graphenelib::string_to_name(txid.c_str());
            auto txid_iterator = Eth_withdraw_table.find(txid_uint);
            graphene_assert(txid_iterator == Eth_withdraw_table.end(), "The txid is existed, be honest");
            auto id_number = Eth_withdraw_table.available_primary_key();
            Eth_withdraw_table.emplace(sender, [&](auto &o) {
                o.txid = txid_uint;
                o.id = id_number;
            });
            auto begin_iterator = Eth_withdraw_table.begin();
            if (id_number - (*begin_iterator).id > TXID_LIST_LIMIT)
            {
                Eth_withdraw_table.erase(begin_iterator);
            }
            auto contract_id = current_receiver();
            auto contract_balance = get_balance(contract_id, amount.asset_id);
            graphene_assert(contract_balance > amount.amount, "balance not enough");
            withdraw_asset(_self, account_id, amount.asset_id, amount.amount);
        }
    }

    //@abi action
    void confirm(uint64_t order_id, std::string target, std::string addr, contract_asset amount, std::string txid)
    {
        uint64_t sender = get_trx_sender();
        graphene_assert(sender == adminAccount, "You have no authority");
        auto idx = records_table.find(order_id);
        graphene_assert(idx != records_table.end(), "There is no that order_id");
        graphene_assert((*idx).target == target, "Unmatched chain name");
        if (target == "ETH")
        {
            auto txid_uint = graphenelib::string_to_name(txid.c_str());
            auto txid_iterator = Eth_confirm_table.find(txid_uint);
            graphene_assert(txid_iterator == Eth_confirm_table.end(), "The txid is existed, be honest");
            auto id_number = Eth_confirm_table.available_primary_key();
            Eth_confirm_table.emplace(sender, [&](auto &o) {
                o.txid = txid_uint;
                o.id = id_number;
            });
            auto begin_iterator = Eth_confirm_table.begin();
            if (id_number - (*begin_iterator).id > TXID_LIST_LIMIT)
            {
                Eth_confirm_table.erase(begin_iterator);
            }
            records_table.modify(idx, sender, [&](auto &o) {
                o.state = 1;
            });
            records_table.erase(idx);
        }
    }

private:
    const uint64_t adminAccount = 4707;
    const std::vector<std::string> TARGETS = {"ETH"};
    const uint64_t MIN_DEPOSIT = 50000;
    const uint64_t MIN_WITHDRAW = 50000;
    const uint64_t TXID_LIST_LIMIT = 10000;

    //@abi table ctxids i64
    struct ctxids
    {
        uint64_t txid;
        uint64_t id;

        uint64_t primary_key() const { return txid; }
        GRAPHENE_SERIALIZE(ctxids, (txid)(id))
    };
    typedef multi_index<N(ctxids), ctxids> ctxids_index;

    //@abi table wtxids i64
    struct wtxids
    {
        uint64_t txid;
        uint64_t id;

        uint64_t primary_key() const { return txid; }
        GRAPHENE_SERIALIZE(wtxids, (txid)(id))
    };
    typedef multi_index<N(wtxids), wtxids> wtxids_index;

    //@abi table record i64
    struct record
    {
        uint64_t id;
        uint64_t from;
        uint64_t asset_id;
        int64_t amount;
        std::string target;
        std::string to;
        uint64_t state;

        uint64_t primary_key() const { return id; }
        uint64_t by_sender() const { return from; }

        GRAPHENE_SERIALIZE(record, (id)(from)(asset_id)(amount)(target)(to)(state))
    };

    typedef multi_index<N(record), record,
                        indexed_by<N(sender), const_mem_fun<record, uint64_t, &record::by_sender>>>
        record_index;

    record_index records_table;
    ctxids_index Eth_confirm_table;
    wtxids_index Eth_withdraw_table;
};

GRAPHENE_ABI(relay, (deposit)(withdraw)(confirm))