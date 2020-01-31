# eosio.delay

Delayed transactions are currently used for 2 reasons:

- Sending transactions which need to be executed at a later date.
- Sending transactions which enforce a permission level wait.

Support for delayed transactions has been deprecated and we need a way to support
protocol-level enforced delays (particularly for transfers sent by semi-trusted accounts)

This contract allows a user to submit a transaction for later execution and checks
the provided permissions and `delay_sec` provided in the transaction against the supplied
`executer` permission.  Once the transaction is recorded in the proposals table, we know 
that the transaction would normally be authenticated by `provided` AND the delay. 

## propose(name proposal_name, permission_level executer, permission_level provided, permission_level canceller, transaction trx)

Proposes a new delayed transaction, permissions will be checked at this stage.

## cancel(name executer, name proposal_name)

Cancels a previously submitted transaction, must supply permissions of the executer.

## exec(name executer, name proposal_name)

Executes a delayed transaction that has been previously registered, will check that the
required amount of time has passed since the transaction was submitted.
