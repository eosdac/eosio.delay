#include <eosio/action.hpp>
#include <eosio/permission.hpp>

#include <eosio.delay/eosio.delay.hpp>

namespace eosio {

    void
    delay::propose(ignore <name> proposal_name, ignore <permission_level> executer, ignore <permission_level> provided,
                   ignore <permission_level> canceller, ignore <transaction> trx) {
        name _proposal_name;
        permission_level _executer;
        permission_level _provided;
        permission_level _canceller;
        transaction_header _trx_header;
        std::vector <permission_level> _executers;
        std::vector <permission_level> _provideds;

        _ds >> _proposal_name >> _executer >> _provided >> _canceller;

        name proposer = _executer.actor;

        _executers.push_back(_executer);
        _provideds.push_back(_provided);

        const char *trx_pos = _ds.pos();
        size_t size = _ds.remaining();
        _ds >> _trx_header;

        // If we have provided and the transaction delay is correct then this transaction is already authorized
        // The delay is enforced in `exec`
        require_auth(_provided);

        check(_trx_header.expiration >= eosio::time_point_sec(current_time_point()), "transaction expired");

        proposals proptable(get_self(), proposer.value);
        check(proptable.find(_proposal_name.value) == proptable.end(), "delayed transaction with the same name exists");

        auto packed_executers = pack(_executers);
        auto packed_provided = pack(_provideds);
        auto resperm = internal_use_do_not_use::check_permission_authorization(
                _executer.actor.value, _executer.permission.value,  // this permission level is satisfied by:
                (const char *) 0, 0,
                packed_provided.data(), packed_provided.size(),  // this permission level
                static_cast<uint64_t>(_trx_header.delay_sec) * 1'000'000  // PLUS this delay (seconds to us)
        );
        check(resperm > 0, "permission authorization failed, delays and provided permissions do not satisfy executer permission");

        // TODO: Remove internal_use_do_not_use namespace after minimum eosio.cdt dependency becomes 1.7.x
        auto res = internal_use_do_not_use::check_transaction_authorization(
                trx_pos, size,
                (const char *) 0, 0,
                packed_executers.data(), packed_executers.size()
        );

        check(res > 0, "transaction authorization failed");

        std::vector<char> pkd_trans;
        pkd_trans.resize(size);
        memcpy((char *) pkd_trans.data(), trx_pos, size);

        proptable.emplace(_provided.actor, [&](auto &prop) {
            prop.proposal_time = current_time_point();
            prop.proposal_name = _proposal_name;
            prop.executer = _executer;
            prop.canceller = _canceller;
            prop.packed_transaction = pkd_trans;
        });
    }

    void delay::cancel(name proposer, name proposal_name) {
        proposals proptable(get_self(), proposer.value);
        auto prop = proptable.find(proposal_name.value);
        check(prop != proptable.end(), "delayed transaction not found");

        require_auth(prop->canceller);
        proptable.erase(prop);
    }

    void delay::exec(name proposer, name proposal_name) {
        proposals proptable(get_self(), proposer.value);
        auto prop = proptable.get(proposal_name.value, "delayed transaction not found");

        transaction_header trx_header;
        datastream<const char*> ds( prop.packed_transaction.data(), prop.packed_transaction.size() );
        ds >> trx_header;
        check( trx_header.expiration >= eosio::time_point_sec(current_time_point()), "transaction expired" );

        uint32_t time_now = current_time_point().sec_since_epoch();
        uint32_t delay_sec = static_cast<uint32_t>(trx_header.delay_sec);
        check(prop.proposal_time.sec_since_epoch() + delay_sec <= time_now, "not ready to execute");

        auto trx = unpack<transaction>(prop.packed_transaction);
        for (action act: trx.actions){
            print("\nsending ", act.account, "::", act.name);
            act.send();
        }

        proptable.erase(prop);
    }

} /// namespace eosio
