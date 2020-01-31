#pragma once

#include <eosio/eosio.hpp>
#include <eosio/ignore.hpp>
#include <eosio/transaction.hpp>

namespace eosio {

    class [[eosio::contract("eosio.delay")]] delay : public contract {
    public:
        using contract::contract;

        /**
         * Create proposal
         *
         * @details Creates a proposal with a transaction which should be executed with a delay
         *
         *
         * @param proposal_name - The name of the proposal (should be unique for proposer)
         * @param executer - The permission which is being used for the transaction, this along with the delay should be
         * all that is needed to authenticate the transaction
         * @param provided - Permission level which is provided, this combined with a delay must be the executer
         * permission.  If a contract is being protected then this will be `yourcontract@eosio.code`
         * @param trx - Proposed transaction
         */
        [[eosio::action]]
        void
        propose(ignore <name> proposal_name, ignore <permission_level> executer, ignore <permission_level> provided,
                ignore <transaction> trx);

        [[eosio::action]]
        void cancel(name executer, name proposal_name);

        [[eosio::action]]
        void exec(name executer, name proposal_name);

        using propose_action = eosio::action_wrapper<"propose"_n, &delay::propose>;
        using cancel_action = eosio::action_wrapper<"cancel"_n, &delay::cancel>;
        using exec_action = eosio::action_wrapper<"exec"_n, &delay::exec>;

    private:
        struct [[eosio::table]] proposal {
            // Time that this is proposed, used with delay_sec to ensure the delay has passed before executing
            time_point_sec    proposal_time;
            // Unique name (within scope of executer account)
            name              proposal_name;
            // The final permission that will be used to execute the transaction
            permission_level  executer;
            // Packed transaction to execute, actions are sent inline on `exec`
            std::vector<char> packed_transaction;

            uint64_t primary_key() const { return proposal_name.value; }
        };

        typedef eosio::multi_index<"proposal"_n, proposal> proposals;

    };
} /// namespace eosio
