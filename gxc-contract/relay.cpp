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
        : contract(account_id), fund_in_table(_self, _self), eth_confirm_table(_self, _self), eth_withdraw_table(_self, _self)
        , fund_out_table(_self, _self), nonce_table(_self, _self), coin_table(_self, _self)
    {
    }

    // @abi action
    // @abi payable
    void deposit(std::string target, std::string addr)
    {
        int64_t asset_amount = get_action_asset_amount();
        uint64_t asset_id = get_action_asset_id();
        graphene_assert(asset_id != -1, "Invalid Asset");
        auto asset_itr = coin_table.find(asset_id);
        graphene_assert(asset_itr != coin_table.end(), "The asset is not supported");
        graphene_assert(asset_itr->enable_transfer == 1, "The asset is locked");
        graphene_assert(asset_amount >= asset_itr->min_deposit, "Must greater than minnumber ");
        uint64_t id_number = fund_in_table.available_primary_key();
        graphene_assert(target == "ETH", "Invalid chain name");
        uint64_t sender = get_trx_sender();
        fund_in_table.emplace(sender, [&](auto &o) {
            o.id = id_number;
            o.from = sender;
            o.asset_id = asset_id;
            o.amount = asset_amount;
            o.target = target;
            o.to = addr;
            o.state = 0;
        });
    }

    // @abi action
    // @abi payable
    void deposit2(std::string target, std::string addr, std::string nonce)
    {
        uint64_t sender = get_trx_sender();
        graphene_assert(sender == adminAccount, "You have no authority");
        for(auto id_begin = nonce_table.begin(); id_begin != nonce_table.end(); id_begin++){
                 graphene_assert((*id_begin).nonce != nonce, "The nonce is existed");
            }
        deposit(target,addr);
        auto id_number = nonce_table.available_primary_key();
        nonce_table.emplace(sender, [&](auto &o) {
            o.id = id_number;
            o.nonce = nonce;
        });
        auto begin_iterator = nonce_table.begin();
        if (id_number - (*begin_iterator).id > NONCE_LIMIT)
        {
            nonce_table.erase(begin_iterator);
        }   
    }

    //@abi action
    void withdraw(std::string to_account, contract_asset amount, std::string from_target, std::string txid, std::string from_account)
    {
        int64_t account_id = get_account_id(to_account.c_str(), to_account.size());
        uint64_t sender = get_trx_sender();
        graphene_assert(from_target == "ETH","Invalid chain name, only support ETH so far");
        graphene_assert(sender == adminAccount, "No authority");
        graphene_assert(account_id >= 0, "Invalid account_name to_account");
        graphene_assert(amount.amount > 0, "Invalid amount");
        graphene_assert(amount.asset_id != -1, "Invalid Asset");
        auto asset_itr = coin_table.find(amount.asset_id);
        graphene_assert(asset_itr != coin_table.end(), "The asset is not supported");
        graphene_assert(asset_itr->enable_transfer == 1, "The asset is locked");
        graphene_assert(amount.amount >= asset_itr->min_withdraw, "Must greater than minnumber ");
        if (from_target == "ETH")
        {
            for(auto id_begin = eth_withdraw_table.begin(); id_begin != eth_withdraw_table.end(); id_begin++){
                 graphene_assert((*id_begin).txid != txid, "The txid is existed, be honest");
            }
            auto id_number = eth_withdraw_table.available_primary_key();
            eth_withdraw_table.emplace(sender, [&](auto &o) {
                o.id = id_number;
                o.txid = txid;
            });
            auto begin_iterator = eth_withdraw_table.begin();
            if (id_number - (*begin_iterator).id > TXID_LIST_LIMIT)
            {
                eth_withdraw_table.erase(begin_iterator);
            }
            auto contract_id = current_receiver();
            auto contract_balance = get_balance(contract_id, amount.asset_id);
            graphene_assert(contract_balance > amount.amount, "Balance not enough");
            auto id_number2 = fund_out_table.available_primary_key();
            int64_t block_time = get_head_block_time();
            fund_out_table.emplace(sender, [&](auto &o){
                o.id = id_number2;
                o.to_account = account_id;
                o.asset_id = amount.asset_id;
                o.amount = amount.amount;
                o.from_target = from_target;
                o.txid = txid;
                o.from_account = from_account;
                o.block_time = block_time;
            });
        }
    }

    //@abi action
    void confirmd(uint64_t order_id, std::string target, std::string addr, contract_asset amount, std::string txid)
    {
        uint64_t sender = get_trx_sender();
        graphene_assert(sender == adminAccount, "You have no authority");
        auto idx = fund_in_table.find(order_id);
        graphene_assert(idx != fund_in_table.end(), "There is no that order_id");
        graphene_assert((*idx).target == target, "Unmatched chain name");
        graphene_assert((*idx).asset_id == amount.asset_id, "Unmatched assert id");
        graphene_assert((*idx).amount == amount.amount, "Unmatched assert amount");
        graphene_assert (target == "ETH","Invalid chain name, only support ETH so far");
        if (target == "ETH")
        {
            for(auto id_begin = eth_confirm_table.begin(); id_begin != eth_confirm_table.end(); id_begin++){
                 graphene_assert((*id_begin).txid != txid, "The txid is existed, be honest");
            }
            auto id_number = eth_confirm_table.available_primary_key();
            eth_confirm_table.emplace(sender, [&](auto &o) {
                o.id = id_number;
                o.txid = txid;
            });
            auto begin_iterator = eth_confirm_table.begin();
            if (id_number - (*begin_iterator).id > TXID_LIST_LIMIT)
            {
                eth_confirm_table.erase(begin_iterator);
            }
            fund_in_table.modify(idx, sender, [&](auto &o) {
                o.state = 1;
            });
            fund_in_table.erase(idx);
        }
    }

    //@abi action
    void confirmw()
    {
       uint64_t sender = get_trx_sender();
       graphene_assert(sender == adminAccount, "You have no authority");
       int64_t block_time_now = get_head_block_time();
       auto idx = fund_out_table.begin();
       auto number_index = 0;
       graphene_assert(idx != fund_out_table.end(), "There is nothing to withdraw");
       while((idx != fund_out_table.end()) && number_index < NUMBER_LIMIT){
           if(((*idx).block_time + TIME_GAP) > block_time_now){
               break;
           }
           withdraw_asset(_self, (*idx).to_account, (*idx).asset_id, (*idx).amount);
           idx = fund_out_table.erase(idx);
           number_index++;
     }

    }

    //@abi action
    void adjustcoin(std::string coinname, uint64_t enabletransfer, uint64_t mindeposit, uint64_t minwithdraw)
    {
       uint64_t sender = get_trx_sender();
       graphene_assert(sender == adminAccount, "You have no authority");
       auto asset_id = get_asset_id(coinname.c_str(), coinname.size());
       graphene_assert(asset_id != -1, "Invalid Asset");
       auto asset_itr = coin_table.find(asset_id);
       if(asset_itr == coin_table.end()){
           coin_table.emplace(sender, [&](auto &o){
               o.asset_id = asset_id;
               o.enable_transfer = enabletransfer;
               o.min_deposit = mindeposit;
               o.min_withdraw = minwithdraw;
           });
       } else {
           coin_table.modify(asset_itr, sender, [&](auto &o){
               o.enable_transfer = enabletransfer;
               o.min_deposit = mindeposit;
               o.min_withdraw = minwithdraw;
           });
       }
    }

    //@abi action
    void drop(std::string tablename ,uint64_t count)
    {
       uint64_t sender = get_trx_sender();
       graphene_assert(sender == adminAccount, "You have no authority");
       graphene_assert((tablename == "ctxids" || tablename == "wtxids" || tablename == "nonceids"), "The tablename is wrong!" );
       if (tablename == "ctxids") {
            auto number_index = 0;
            auto begin_iterator =  eth_confirm_table.begin();
            while((begin_iterator != eth_confirm_table.end()) && number_index < count){
                begin_iterator = eth_confirm_table.erase(begin_iterator);
                number_index++;
            }
       } else if(tablename == "wtxids") {
            auto number_index = 0;
            auto begin_iterator =  eth_withdraw_table.begin();
            while((begin_iterator != eth_withdraw_table.end()) && number_index < count){
                begin_iterator = eth_withdraw_table.erase(begin_iterator);
                number_index++;
            }  
       } else {
            auto number_index = 0;
            auto begin_iterator =  nonce_table.begin();
            while((begin_iterator != nonce_table.end()) && number_index < count){
                begin_iterator = nonce_table.erase(begin_iterator);
                number_index++;  
         }
      }
    }
private:
    const uint64_t adminAccount = 22;
    const uint64_t TXID_LIST_LIMIT = 100;
    const int64_t TIME_GAP = 3600;
    const uint64_t NUMBER_LIMIT = 10;
    const uint64_t NONCE_LIMIT = 10;

    //@abi table nonceids i64
    struct nonceids
    {
        uint64_t id;
        std::string nonce;

        uint64_t primary_key() const { return id; }
        GRAPHENE_SERIALIZE(nonceids, (id)(nonce))
    };
    typedef multi_index<N(nonceids), nonceids> nonceids_index;

    //@abi table ctxids i64
    struct ctxids
    {
        uint64_t id;
        std::string txid;

        uint64_t primary_key() const { return id; }
        GRAPHENE_SERIALIZE(ctxids, (id)(txid))
    };
    typedef multi_index<N(ctxids), ctxids> ctxids_index;

    //@abi table wtxids i64
    struct wtxids
    {
        uint64_t id;
        std::string txid;

        uint64_t primary_key() const { return id; }
        GRAPHENE_SERIALIZE(wtxids, (id)(txid))
    };
    typedef multi_index<N(wtxids), wtxids> wtxids_index;

    //@abi table fundin i64
    struct fundin
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

        GRAPHENE_SERIALIZE(fundin, (id)(from)(asset_id)(amount)(target)(to)(state))
    };

    typedef multi_index<N(fundin), fundin,
                        indexed_by<N(sender), const_mem_fun<fundin, uint64_t, &fundin::by_sender>>>
        fund_in_index;

    //@abi table fundout i64
    struct fundout{
        uint64_t id;
        uint64_t to_account;
        uint64_t asset_id;
        int64_t amount;
        std::string from_target;
        std::string txid;
        std::string from_account;
        int64_t block_time;

        uint64_t primary_key() const { return id; }

        GRAPHENE_SERIALIZE(fundout, (id)(to_account)(asset_id)(amount)(from_target)(txid)(from_account)(block_time))
    };
    typedef multi_index<N(fundout), fundout> fund_out_index;

    //@abi table coin i64
    struct coin{
        uint64_t asset_id;
        uint64_t enable_transfer;//1为开启状态，0为locked.
        uint64_t min_deposit;
        uint64_t min_withdraw;

        uint64_t primary_key() const {return asset_id;}

        GRAPHENE_SERIALIZE(coin, (asset_id)(enable_transfer)(min_deposit)(min_withdraw))
    };
    typedef multi_index<N(coin), coin> coin_index;

    fund_in_index fund_in_table;
    ctxids_index eth_confirm_table;
    wtxids_index eth_withdraw_table;
    fund_out_index fund_out_table;
    nonceids_index nonce_table;
    coin_index coin_table;
};

GRAPHENE_ABI(relay, (deposit)(deposit2)(withdraw)(confirmd)(confirmw)(adjustcoin)(drop))