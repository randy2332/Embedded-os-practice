# Lab 6

Multiple clients are connected at the same time to make deposits and withdrawals from the same account, and the server must be able to correctly handle the race condition problem caused by simultaneous access, so as not to cause financial losses to customers.

- Server
    - build  socket server，wait client to build connection。
    - server support multiple connection。
    - After the connection is established, you must perform **deposit** or **withdraw** actions according to the customer's requirements.
    - There will only be one account on the server side, shared by multiple people.
    - Output the balance of the account after each action is completed.
- Client
    - Specify the request (deposit or withdrawal, how much), and then pass the information to the server