# Deploying the FireRed PC online servers

Live on the SoulMon VPS (74.208.169.223, AlmaLinux 9, Python 3.9, no extra packages).

    /opt/firered-server/          the server/*.py files + legality_data*.json, owned by user `firered`
    firered-master.service        master on 27100 (writes master.db + server_key.txt on first run)
    firered-game@kanto.service    game server "Kanto" on 27200 (settings in game-kanto.env)

Another world: copy game-kanto.env to game-<name>.env with a new NAME/PORT, then
`systemctl enable --now firered-game@<name>`. Each port must also be opened in the
IONOS control-panel firewall (the box itself has no firewall).

Update: copy the changed .py/.json files, then
`systemctl restart firered-master firered-game@kanto`.
Players' clients connect to the master given in online.ini (`master = host:port`);
the built-in default is 74.208.169.223:27100.
