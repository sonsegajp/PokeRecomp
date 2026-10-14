"""gameserver.py — a FireRed PC game server (one "world" players join).

    python gameserver.py --name "Kanto 1" --public-host my.host --port 27200 \
                         --master 127.0.0.1:27100 --key-file server_key.txt

Registers with the master, checks every joining player's login token with it,
and then relays:
  * presence — where every player is (the game draws them on the same map),
  * chat,
  * battle / trade requests, and
  * the virtual link cable for accepted ones,
and runs the world: the day's shiny-boost type and timed outbreaks (world.py).

The link cable: the game's link.c exchanges one 8-word command per player per
transfer cycle; every unit receives the same merged set. We do exactly that
over the network: each session ticks at 60 Hz, takes the next queued command
from every member (zeros if it has none) and sends the merged frame to all of
them — so both games see identical data in identical order, as on a cable.
"""
import argparse
import asyncio
import collections
import json
import os
import struct
import time

from protocol import *  # noqa: F401,F403
import legality
import world_config
from world import World, system_chat, to_game

# The PokeRecomp master every world registers with (and players log in to).
# Not printed anywhere: hosts and players only ever see world names.
OFFICIAL_MASTER = "74.208.169.223:25565"

TICK = 1 / 60
MAX_QUEUE = 600          # commands a client may have waiting (10 s) before it is cut off
MAX_CATCHUP = 8          # extra merged frames per tick when a member is ahead


class Player:
    def __init__(self, pid, writer):
        self.id = pid
        self.writer = writer
        self.account = 0
        self.name = bytes(NAME_LEN)
        self.tag = 0
        self.guild = 0
        self.gender = 0
        self.game = GAME_FIRERED
        self.pos = (0, 0, 0, 0, 0, 0)       # group, num, x, y, facing, flags
        self.safe_pos = None                # the last spot on foot: where they come back to
        self.session = None
        self.pending_to = None              # (target id, kind) we asked
        self.link_queue = collections.deque()
        self.block = None                   # link block being sent: [size, bytearray]
        self.last_shiny = 0.0               # shiny announcements: at most one every few seconds

    def info(self):
        return frame(S_PLAYER, struct.pack("<H", self.id) + self.name + struct.pack("<HB", self.tag, self.gender)
                     + struct.pack("<BBhhBB", *self.pos) + bytes([self.game]))

    def send(self, data):
        try:
            self.writer.write(data)
        except Exception:
            pass


class Session:
    def __init__(self, members, kind):
        self.members = members              # index = link id (0 = master)
        self.kind = kind
        self.open = [False] * len(members)  # each member's end of the cable

    def all_open(self):
        return all(self.open)


class GameServer:
    def __init__(self, args):
        self.args = args
        self.players = {}
        self.next_id = 1
        self.sessions = set()
        self.master_writer = None
        self.verify_waits = {}
        self.next_req = 1
        self.world = World(args)
        self.positions_file = args.positions
        try:
            with open(self.positions_file) as f:
                self.positions = json.load(f)  # account id -> [group, num, x, y, facing, time]
        except (OSError, ValueError):
            self.positions = {}

    def log_position(self, player):
        """A player left: remember where, so they come back to the same spot."""
        if not player.account or not player.safe_pos:
            return
        g, n, x, y, facing = player.safe_pos
        self.positions[str(player.account)] = [g, n, x, y, facing, int(time.time())]
        print(f"game: player {player.id} (account {player.account}) left at map {g}.{n} ({x},{y})")
        try:
            tmp = self.positions_file + ".tmp"
            with open(tmp, "w") as f:
                json.dump(self.positions, f)
            os.replace(tmp, self.positions_file)
        except OSError as e:
            print("game: could not write", self.positions_file, e)

    def welcome(self, player):
        last = self.positions.get(str(player.account))
        body = struct.pack("<H", player.id) + text(self.args.name, 24)
        if last:
            body += struct.pack("<BBBhhB", 1, *last[:5])
        else:
            body += bytes(8)
        return frame(S_WELCOME, body)

    # ---- master link ---------------------------------------------------------

    async def master_loop(self):
        host, port = self.args.master.rsplit(":", 1)
        try:                      # our own worlds have the server key; anyone else hosts a community world
            key = open(self.args.key_file).read().strip().encode()
        except OSError:
            key = b""
        while True:
            try:
                reader, writer = await asyncio.open_connection(host, int(port))
                writer.write(frame(G_REGISTER, fixed(key, 32) + text(self.args.name, 24)
                                   + text(self.args.public_host, 64) + struct.pack("<HH", self.args.port, self.args.max)
                                   + bytes([REGION_NAMES[self.args.region]])))
                self.master_writer = writer
                for p in self.players.values():
                    writer.write(frame(G_PRESENCE, struct.pack("<IB", p.account, 1)))
                print("game: registered with the PokeRecomp master"
                      + ("" if key else " as a community world: players connect to this PC on TCP port %d"
                                        " (forward it on your router)" % self.args.port))
                beat = asyncio.ensure_future(self.heartbeat(writer))
                while True:
                    msg = await read_frame(reader)
                    if msg is None:
                        break
                    t, p = msg
                    if t == G_VERIFIED:
                        req, ok, aid = struct.unpack_from("<IBI", p)
                        tag, guild = struct.unpack_from("<HI", p, 9 + NAME_LEN)
                        fut = self.verify_waits.pop(req, None)
                        if fut and not fut.done():
                            fut.set_result((ok, aid, bytes(p[9:9 + NAME_LEN]), tag, guild))
                    elif t == G_GUILD_SET:
                        aid, guild = struct.unpack_from("<II", p)
                        for pl in self.players.values():
                            if pl.account == aid:
                                pl.guild = guild
                    elif t == G_GUILD_CHAT:
                        (guild,) = struct.unpack_from("<I", p)
                        data = frame(S_GUILD_CHAT_FROM, bytes(p[4:]))
                        for pl in self.players.values():
                            if guild and pl.guild == guild:
                                pl.send(data)
                    elif t == M_ERROR:
                        code = p[0] if p else 0
                        print("game: the master refused this world:",
                              "too many worlds from this address" if code == ERR_FULL else "wrong server key")
                beat.cancel()
            except OSError:
                print("game: can't reach the PokeRecomp master; retrying")
            self.master_writer = None
            await asyncio.sleep(5)

    async def heartbeat(self, writer):
        while True:
            writer.write(frame(G_HEARTBEAT, struct.pack("<H", len(self.players))))
            await asyncio.sleep(10)

    async def verify(self, token):
        if not self.master_writer:
            return None
        req = self.next_req
        self.next_req += 1
        fut = asyncio.get_event_loop().create_future()
        self.verify_waits[req] = fut
        self.master_writer.write(frame(G_VERIFY, struct.pack("<I", req) + token))
        try:
            ok, aid, name, tag, guild = await asyncio.wait_for(fut, 10)
        except asyncio.TimeoutError:
            return None
        return (aid, name, tag, guild) if ok else None

    def presence(self, player, online):
        if self.master_writer and player.account:
            self.master_writer.write(frame(G_PRESENCE, struct.pack("<IB", player.account, online)))

    # ---- players -------------------------------------------------------------------

    def broadcast(self, data, skip=None):
        for p in self.players.values():
            if p is not skip:
                p.send(data)

    async def handle(self, reader, writer):
        player = None
        try:
            msg = await read_frame(reader)
            if not msg or msg[0] != S_JOIN or len(msg[1]) < TOKEN_LEN + 3:
                return
            token = bytes(msg[1][:TOKEN_LEN])
            gender, version = struct.unpack_from("<BH", msg[1], TOKEN_LEN)
            who = await self.verify(token) if version == VERSION else None
            if not who or len(self.players) >= self.args.max:
                writer.write(frame(S_ERROR, bytes([ERR_BAD_LOGIN if not who else ERR_FULL])))
                await writer.drain()
                return
            for other in list(self.players.values()):      # same account twice: drop the old one
                if other.account == who[0]:
                    other.writer.close()
            player = Player(self.next_id, writer)
            self.next_id = self.next_id % 60000 + 1
            player.account, player.name, player.tag, player.guild = who
            player.gender = gender
            if len(msg[1]) >= TOKEN_LEN + 4:
                player.game = msg[1][TOKEN_LEN + 3]
            self.players[player.id] = player
            self.presence(player, 1)
            player.send(self.welcome(player))
            for other in self.players.values():
                if other is not player:
                    player.send(other.info())
            player.send(self.world.state_frame())
            # the world's message of the day, then what's on, then everyone hears who arrived
            player.send(system_chat(self.args.welcome or f"Welcome to {self.args.name}!"))
            for line in self.world.announcements():
                player.send(line)
            name = player.name.split(b"\xff")[0].rstrip(b"\0")
            self.broadcast(frame(S_CHAT_FROM, struct.pack("<H", 0) + name + to_game(" has joined!")))
            print(f"game: player {player.id} joined ({len(self.players)} online)")
            while True:
                msg = await read_frame(reader)
                if msg is None:
                    break
                self.dispatch(player, *msg)
                await writer.drain()
        except Exception as e:
            print("game: client error:", e)
        finally:
            if player and self.players.get(player.id) is player:
                del self.players[player.id]
                self.log_position(player)
                self.end_session(player, 2)
                self.presence(player, 0)
                self.broadcast(frame(S_PLAYER_GONE, struct.pack("<H", player.id)))
                print(f"game: player {player.id} left ({len(self.players)} online)")
            writer.close()

    def dispatch(self, player, t, p):
        if t == S_POS and len(p) >= 8:
            player.pos = struct.unpack_from("<BBhhBB", p)
            if player.pos[5] & POS_SAFE:
                player.safe_pos = player.pos[:5]
            self.broadcast(player.info(), skip=player)
        elif t == S_CHAT:
            self.broadcast(frame(S_CHAT_FROM, struct.pack("<H", player.id) + bytes(p[:64])))
        elif t == S_SHINY and len(p) >= 4:
            # "NAME found a shiny PIKACHU!" / "NAME caught a shiny PIKACHU!", to everyone
            now = time.monotonic()
            if now - player.last_shiny < 3:
                return
            player.last_shiny = now
            name = player.name.split(b"\xff")[0].rstrip(b"\0")
            species = bytes(p[3:14]).split(b"\xff")[0][:10]
            if not species:
                return
            verb = " caught a shiny " if p[0] else " found a shiny "
            line = name + to_game(verb)[:-1] + species + to_game("!")
            self.broadcast(frame(S_CHAT_FROM, struct.pack("<H", 0) + line[:63] + b"\xff"))
        elif t == S_GUILD_CHAT and player.guild and self.master_writer:
            self.master_writer.write(frame(G_GUILD_CHAT, struct.pack("<I", player.account) + bytes(p[:64])))
        elif t == S_REQUEST and len(p) >= 3:
            target_id, kind = struct.unpack_from("<HB", p)
            target = self.players.get(target_id)
            if not target:
                player.send(frame(S_DECLINED, struct.pack("<HB", target_id, DECLINE_GONE)))
            elif target.session or player.session or target.pending_to:
                player.send(frame(S_DECLINED, struct.pack("<HB", target_id, DECLINE_BUSY)))
            elif (kind == KIND_TRADE and not self.args.trades) or (kind != KIND_TRADE and not self.args.battles):
                # this world's settings: no battles / no trades
                player.send(system_chat("Trades are off on this world." if kind == KIND_TRADE
                                        else "Battles are off on this world."))
                player.send(frame(S_DECLINED, struct.pack("<HB", target_id, DECLINE_NO)))
            else:
                player.pending_to = (target_id, kind)
                target.send(frame(S_REQUESTED, struct.pack("<HB", player.id, kind)))
        elif t == S_RESPOND and len(p) >= 3:
            from_id, accept = struct.unpack_from("<HB", p)
            asker = self.players.get(from_id)
            if not asker or not asker.pending_to or asker.pending_to[0] != player.id:
                return
            kind = asker.pending_to[1]
            asker.pending_to = None
            if not accept or asker.session or player.session:
                asker.send(frame(S_DECLINED, struct.pack("<HB", player.id, DECLINE_NO if not accept else DECLINE_BUSY)))
                return
            self.start_session([asker, player], kind)
        elif t == S_LINK_CMD and player.session and len(p) >= CMD_LENGTH * 2:
            if not player.session.all_open():
                pass                                  # cable not connected: dropped, as on hardware
            elif self.args.legality and self.inspect(player, bytes(p[:CMD_LENGTH * 2])):
                return                                # session ended: illegal Pokémon
            elif len(player.link_queue) >= MAX_QUEUE:
                self.end_session(player, 3)
            else:
                player.link_queue.append(bytes(p[:CMD_LENGTH * 2]))
        elif t == S_LINK_END:
            self.end_session(player, 0)
        elif t == S_LINK_OPEN and player.session and len(p) >= 1:
            # The game closes and reopens its link inside one activity (e.g.
            # Colosseum -> battle). Like the real handshake, the cable is only
            # "connected" while every member has its end open.
            s = player.session
            was = s.all_open()
            s.open[s.members.index(player)] = bool(p[0])
            print(f"game: player {player.id} link {'open' if p[0] else 'closed'}; connected={s.all_open()}")
            if s.all_open() != was:
                for m in s.members:
                    m.link_queue.clear()
                    m.send(frame(S_LINK_STATE, bytes([s.all_open()])))

    # ---- legality -------------------------------------------------------------------------

    def inspect(self, player, cmd):
        """Rebuild the player's link blocks (link.c: INIT_BLOCK 0xBBBB size, then
        CONT_BLOCK 0x8888 + 14 bytes) and check every Pokémon party block.
        Returns True if the session was ended for an illegal Pokémon."""
        word0, size = struct.unpack_from("<HH", cmd)
        if word0 == 0xBBBB:
            player.block = [size, bytearray()]
        elif word0 == 0x8888 and player.block:
            player.block[1] += cmd[2:16]
            if len(player.block[1]) >= player.block[0]:
                data = bytes(player.block[1][:player.block[0]])
                player.block = None
                why = legality.check_block(data)
                if len(data) == 200:
                    print(f"game: checked party block from player {player.id}: {why or 'legal'}")
                    if not why and not os.path.exists("sample_party_block.bin"):
                        open("sample_party_block.bin", "wb").write(data)   # for legality tests
                if why:
                    print(f"game: player {player.id} sent an illegal Pokemon ({why}); ending the session")
                    self.end_session_all(player, 4)
                    return True
        return False

    def end_session_all(self, player, reason):
        s = player.session
        if not s:
            return
        self.end_session(player, reason)
        player.send(frame(S_SESSION_END, bytes([reason])))

    # ---- link sessions -----------------------------------------------------------------

    def start_session(self, members, kind):
        s = Session(members, kind)
        self.sessions.add(s)
        for i, m in enumerate(members):
            m.session = s
            m.link_queue.clear()
            partner = members[1 - i] if len(members) == 2 else members[0]
            m.send(frame(S_SESSION, struct.pack("<BBBH", i, len(members), kind, partner.id)))
        print("game: link session", [m.id for m in members], "kind", kind)

    def end_session(self, player, reason):
        s = player.session
        if not s:
            return
        print(f"game: link session {[m.id for m in s.members]} ended by player {player.id} (reason {reason})")
        self.sessions.discard(s)
        for m in s.members:
            m.session = None
            m.link_queue.clear()
            if m is not player:
                m.send(frame(S_SESSION_END, bytes([reason])))

    async def tick_loop(self):
        # A game sends one command per frame (59.73 Hz) and sleep() ticks a bit
        # slower than 60 Hz, so a tick merges extra frames while anyone is
        # ahead instead of letting their queue grow.
        zero = bytes(CMD_LENGTH * 2)
        while True:
            await asyncio.sleep(TICK)
            for s in list(self.sessions):
                if not s.all_open():
                    continue
                backlog = max(len(m.link_queue) for m in s.members)
                for _ in range(min(max(1, backlog - 1), MAX_CATCHUP)):
                    cmds = [m.link_queue.popleft() if m.link_queue else zero for m in s.members]
                    if any(c != zero for c in cmds):
                        data = frame(S_LINK_FRAME, bytes([len(cmds)]) + b"".join(cmds))
                        for m in s.members:
                            m.send(data)


    async def world_loop(self):
        while True:
            for data in self.world.update():
                self.broadcast(data)
            await asyncio.sleep(5)


async def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--config", help="a world settings file (world_config.py); the command line wins over it")
    ap.add_argument("--write-config", metavar="FILE", help="write a settings file with every setting, then exit")
    ap.add_argument("--welcome", default="", help="chat line everyone sees on arrival")
    ap.add_argument("--base-shiny-odds", type=int, default=8192, help="1 in N for every wild Pokemon")
    ap.add_argument("--boost-type", default="daily", help="daily, off, or a type name (FIRE)")
    ap.add_argument("--outbreaks", type=world_config.parse_bool, default=True, help="on/off")
    ap.add_argument("--legality", type=world_config.parse_bool, default=True, help="on/off: check Pokemon")
    ap.add_argument("--battles", type=world_config.parse_bool, default=True, help="on/off")
    ap.add_argument("--trades", type=world_config.parse_bool, default=True, help="on/off")
    ap.add_argument("--name", default="Kanto")
    ap.add_argument("--region", default="kanto", choices=sorted(REGION_NAMES),
                    help="kanto: FireRed/LeafGreen world; hoenn: Ruby/Sapphire/Emerald world")
    ap.add_argument("--port", type=int, default=27200)
    ap.add_argument("--public-host", default="", help="address players use to reach this server (default: the one it connects from)")
    ap.add_argument("--master", default=OFFICIAL_MASTER, help=argparse.SUPPRESS)   # tests: a local master
    ap.add_argument("--key-file", default="server_key.txt", help="our own worlds only; without it this is a community world")
    ap.add_argument("--max", type=int, default=100)
    ap.add_argument("--positions", help="file keeping each player's last position (default: positions_PORT.json)")
    ap.add_argument("--shiny-odds", type=int, default=512, help="1 in N for the day's boosted type (normally 8192)")
    ap.add_argument("--event-every", type=float, default=60, help="minutes between outbreaks")
    ap.add_argument("--event-length", type=float, default=20, help="minutes an outbreak lasts")
    ap.add_argument("--first-event", type=float, default=1, help="minutes until the first outbreak")
    ap.add_argument("--outbreak-percent", type=int, default=50, help="share of wild encounters there")
    ap.add_argument("--outbreak", help='start with this outbreak: "GROUP.NUM:SPECIES" (e.g. 3.19:25)')
    first, _ = ap.parse_known_args()
    if first.config and os.path.exists(first.config):
        ap.set_defaults(**world_config.read(first.config))   # the file, then the command line over it
    args = ap.parse_args()
    args.max = max(1, min(args.max, 200))
    if args.write_config:
        world_config.write_template(args.write_config, args)
        print("game: wrote", args.write_config)
        return
    args.positions = args.positions or os.path.join(os.path.dirname(os.path.abspath(__file__)), f"positions_{args.port}.json")
    gs = GameServer(args)
    srv = await asyncio.start_server(gs.handle, "0.0.0.0", args.port)
    print("game: listening on", args.port)
    asyncio.ensure_future(gs.master_loop())
    asyncio.ensure_future(gs.tick_loop())
    asyncio.ensure_future(gs.world_loop())
    async with srv:
        await srv.serve_forever()


if __name__ == "__main__":
    asyncio.run(main())
