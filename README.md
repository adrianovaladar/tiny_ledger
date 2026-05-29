# tiny_ledger
This project is a minimal C++ HTTP server that simulates a simple banking system. It allows the user to deposit and withdraw money, to check the balance and to view the transaction history.

## Installation
To compile the project, navigate to the root directory and execute the following commands:

    $ cmake .
    $ make

After the compilation is complete, a binary named tiny_ladder will be present in the same directory.

## API

### Deposit
Add funds to the account.
```bash
curl -X POST http://localhost:8080/deposit \
  -d "100"
```
### Withdraw
Remove funds from the account.
```bash
curl -X POST http://localhost:8080/withdraw \
  -d "50"
```
### Balance
Get the current account balance.
```bash
curl http://localhost:8080/balance
```
### Transactions
List all recorded transactions.
```bash
curl http://localhost:8080/transactions
```

