In order to communicate with daemon, use an interface with socket.
This daemon understand some orders.

TCP datagrams are :

Interface >> Dlift

- Initialize connection => Dlift respond "OK"
- "alive" => test mode sended on towers and send the aswer
- "repair" => restart the daemon
- "reboot" => restart the machine who's hosting the daemon
- "stats" => send uptime
- any other datagram : parse it, and send to the right tower. If order is unknown for towers, aswer "order unknown".


 