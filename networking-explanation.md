# PokeRecomp Networking — How It Works

This document explains the online multiplayer layer of the PokeRecomp PC port:
the wire protocol, the client, the dedicated server, and how link-cable battles
and trades are tunneled over TCP.

All of it is custom code written for the PC port. None of it exists in the
original GBA game or in `pret/pokefirered`.

---

## 1. Components

| Piece | File(s) | Role |
|---|---|---|
| Wire protocol | `server/protocol.h` | Packed structs + message IDs shared by both sides |
| Client | `src/platform/net_client.c` | Socket, packet pump, remote-player state |
| Remote avatars | `src/platform/net_player_sprites.c` | Turns remote player state into real object events |
| Link shim | `src/platform/net_link.c` | Fakes the GBA link cable so battle/trade code works unmodified |
| Chat overlay | `src/platform/net_chat_display.c` | Draws chat + nametags |
| Connect UI | `src/platform/imgui_bridge.cpp` | ImGui "Online" menu: IP, port, name, password |
| Save upload | `src/platform/sdl2.c` (`Platform_FlushSaveToServer`) | Pushes flash save back to server |
| Server | `server/server.c` | Standalone `PokeServer.exe`, single-threaded relay |

Transport is plain **TCP** (`SOCK_STREAM`), default port **27015**, `TCP_NODELAY`
on. No encryption, no compression, no UDP.

---

## 2. Wire protocol

`server/protocol.h` is compiled into both the game and the server, so the two
always agree on layout. Every struct is inside `#pragma pack(push, 1)` — packets
are the raw struct bytes, sent and cast back with no serialization step. This
means the protocol is **host-endian and ABI-dependent**: it works because both
ends are little-endian 32-bit MinGW builds.

Every packet begins with:

```c
struct PacketHeader {
    uint8_t  type;   // message ID
    uint16_t size;   // total packet size, header included
};
```

`size` is what makes stream framing possible (see §3.2). Message IDs are split
by direction: client to server is `0x01`–`0x0F`, server to client is `0x81`–`0x92`.

Key packets:

- `ConnectPacket` — protocol version, username, chosen trainer sprite, spawn
  position, and a 32-byte password hash.
- `PositionPacket` — map group/number, x, y, facing direction, and a
  `movementAction` byte used as status flags: bit 0 = running, bit 1 = busy.
- `ChatPacket` — username + 128-byte message.
- `InteractPacket` / `InteractResponsePacket` — battle/trade request and accept/decline.
- `PartyDataPacket` — 200 bytes of party data per chunk (2 Pokémon at 100 bytes each), up to 3 chunks.
- `LinkDataPacket` — up to 256 bytes of raw link-cable block data (§6).
- `SaveDataPacket` — 4 KB chunk of the 128 KB flash save.
- `LoginResultPacket` — `LOGIN_OK` / `LOGIN_BAD_PASSWORD` / `LOGIN_NEW_ACCOUNT`, plus assigned player ID.

Caps: 64 players, 16-char usernames, 128-char chat.

---

## 3. Client

### 3.1 Connecting (`NetClient_Connect`)

The connect handshake is deliberately **blocking**, and only afterwards does the
socket switch to non-blocking. Sequence:

1. `WSAStartup` (Windows), create socket, `TCP_NODELAY`, `connect()`.
2. Send `ConnectPacket` with username + SHA-256 of the password (hashed in
   `imgui_bridge.cpp` before it ever leaves the process; the plaintext password
   is never sent).
3. Block on `recv()` for `LoginResultPacket`.
   - `LOGIN_BAD_PASSWORD` — close socket, connection refused.
   - `LOGIN_OK` — the server streams the account's save; the client blocks
     reading 32 x 4 KB `SaveDataPacket`s straight into `FLASH_BASE[]` (the
     emulated cartridge flash), then calls `LoadGameSave(0)`,
     `LoadSaveblockMapHeader()`, `LoadSaveblockObjEventScripts()` and jumps the
     main callbacks to `CB1_Overworld` / `CB2_LoadMap` — i.e. it warps you
     straight into your saved game.
   - `LOGIN_NEW_ACCOUNT` — `FLASH_BASE` is filled with `0xFF` (blank cart) and
     the game is sent to `CB2_InitCopyrightScreenAfterBootup` to start fresh.
4. Switch the socket to non-blocking (`FIONBIO`), drain one more read for the
   existing-player list, server config (shiny rate), and MOTD.

The consequence worth knowing: **your save lives on the server**, not on disk,
when you play online. The local flash image is overwritten at login.

### 3.2 The packet pump (`NetClient_Poll`)

Called once per frame from `CallCallbacks()` in [main.c:277](src/main.c#L277) —
on the game thread, after `callback1`/`callback2` have run, so packet handling
never races with game logic. (An earlier version polled from the SDL thread;
`sdl2.c` still carries the comment noting it moved.)

`NetClient_Poll` is a classic accumulate-and-frame loop over a persistent 8 KB
`sRecvBuf`:

1. One non-blocking `recv()` appends whatever arrived to the buffer.
2. Walk the buffer: if fewer than 3 bytes remain, stop; read `header.size`; if
   the full packet hasn't arrived yet, stop.
3. Dispatch on `header.type`.
4. `memmove` the unconsumed tail to the front.

This correctly handles TCP coalescing and split packets. Sanity check: a
`size` of 0 or over 4096 flushes the whole buffer rather than trusting it.

Handlers mutate a plain array `sRemotePlayers[64]`, each entry holding position,
map, sprite, direction and `needsSpawn`/`needsDespawn` edge flags. Nothing is
drawn here — the poll only updates state.

### 3.3 Sending position

Position is pushed from three places, all guarded by `NetClient_IsConnected()`:

- [overworld.c:1492](src/overworld.c#L1492) — the main path. A counter sends
  **every 4th frame** (~15 Hz), carrying running state and a "busy" flag derived
  from `ScriptContext_IsEnabled() || ArePlayerFieldControlsLocked()`.
- [field_control_avatar.c:701](src/field_control_avatar.c#L701) — an immediate
  send on step events so turns/steps aren't delayed by the throttle.
- [main.c:286](src/main.c#L286) — a slow keepalive (every 8th frame) while
  *not* in the overworld, so other players still see you (flagged busy) while
  you sit in menus or battles.

---

## 4. Remote players as real NPCs

`net_player_sprites.c` is the trick that makes other players feel native: remote
players are not a custom renderer, they are **actual object events** spawned
into the live map through `SpawnSpecialObjectEventParameterized()`, using local
IDs from a reserved `NET_LOCAL_ID_START` range.

`NetPlayerSprites_Update()` runs each frame from the overworld callback and
reconciles the 64-slot network state against up to `MAX_NET_SPRITES` live object
events:

- **Same map, no sprite** — spawn one, graphics chosen by `trainerSprite`.
- **Same map, has sprite** — update target position, then interpolate.
- **Different map, has sprite** — `RemoveObjectEventByLocalIdAndMap`, despawn.
- **Player gone from the server list** — despawn.

Movement interpolation is intentionally simple and always tile-aligned:

- Delta over 3 tiles in any axis — **teleport** (snap `currentCoords`). Covers
  warps and packet loss during long stalls.
- Otherwise — issue one held movement per tile toward the target
  (`ObjectEventSetHeldMovement`), picking run vs. walk actions from the
  `isRunning` flag, and only starting the next step once
  `ObjectEventClearHeldMovementIfFinished()` reports the previous one done.
- Already at target — just `ObjectEventTurn` to the reported facing.

Because remote players walk with the engine's own movement actions, they get
correct animation, sprite priority, and grass/water behavior for free — the cost
is that they lag behind reality by the queue depth of the tiles they're catching
up on.

Extras layered on top: an exclamation-mark field effect (`FLDEFF_EXCLAMATION_MARK_ICON`)
fires on the rising edge of a player becoming "busy", nametags are exposed via
`NetPlayerSprites_GetScreenPositions()` for the overlay, and
`NetPlayerSprites_GetFacingPlayerId()` tells the interaction code which player
you're standing in front of when you press A.

---

## 5. Battles and trades: request flow

Interaction is a small state machine on the client (`sInteractState`) plus an
`interactPartner` pairing on the server.

```
A presses A on B  ->  MSG_C_BATTLE_REQUEST --> server --> MSG_S_BATTLE_REQUEST --> B
                      (A: REQUESTING)                       (B: REQUESTED, prompt shown)
B accepts         ->  MSG_C_BATTLE_ACCEPT  --> server pairs A<->B
                      server --> MSG_S_BATTLE_START --> both
both              ->  MSG_C_PARTY_DATA (up to 3 x 200-byte chunks)
                      server relays to partner only
when all chunks in -> state = BATTLE_READY -> fade out, start battle
```

Trades follow the identical shape with `MSG_*_TRADE_*`, plus a
`TradeSelectPacket` exchange so each side learns the other's chosen party slot;
when both selections are known the state becomes `TRADE_READY`.

The server enforces very little here: it checks the target is online and that
neither side is already paired (`interactPartner >= 0`), then relays. Party data
and trade selections are only forwarded to the *paired* partner, never broadcast.

Cancellation (`MSG_S_INTERACT_CANCEL`) is emitted on decline and, importantly,
on partner disconnect — `HandleDisconnect` clears the pairing and notifies the
surviving player so nobody is stuck in a dead battle.

---

## 6. The link-cable shim — how real PvP battles work

This is the most interesting part. The battle engine, trade engine, and all
their scripts are the original GBA code, which talks to the Game Link Cable via
`SendBlock`, `GetBlockReceivedStatus`, `GetMultiplayerId`, etc. Rather than
rewrite the battle system for netplay, `net_link.c` **impersonates the link
hardware**.

`src/link.c` is patched at ~10 call sites with the same pattern:

```c
extern bool8 NetLink_IsActive(void);
if (NetLink_IsActive())
    return NetLink_GetMultiplayerId();   // ...or SendBlock, block status, etc.
```

so when a net battle is running, every link primitive is rerouted to the TCP
implementation, and when it isn't, the original serial code path is untouched.

`NetLink_Init(partnerId, isMaster)` fabricates the state the engine expects:

- Sets `gLinkStatus` to "2 players, connection established", with the master bit
  from whoever initiated, and `gReceivedRemoteLinkPlayers = TRUE`.
- Clears the `gRemoteLinkPlayersNotReceived` / `gReadyToCloseLink` /
  `gReadyToExitStandby` handshake arrays that the real cable would have driven.
- Fills both `gLinkPlayers[]` entries by hand: trainer ID from the save block,
  gender and game version inferred from the trainer sprite (sprite >= 2 means the
  Ruby/Hoenn side, so `VERSION_RUBY`), `linkType = LINKTYPE_BATTLE`, and names
  converted from ASCII to the game's character encoding inline
  (`A-Z` to `0xBB+`, `a-z` to `0xD5+`, `0-9` to `0xA1+`, terminated with `0xFF`).
- `SetSuppressLinkErrorMessage(TRUE)` so a stalled network read never triggers
  the GBA's "link error" screen.

Block exchange (`SendBlock` to `NetLink_SendBlock`):

1. The outgoing block is copied straight into **your own**
   `gBlockRecvBuffer[myId]` and your received-bit is set — the engine expects to
   see its own block echoed back by the cable, so the shim loops it back locally.
2. The same bytes go out as a `LinkDataPacket` (`MSG_C_LINK_DATA`, 256 bytes max).
3. The server relays it to the paired partner as `MSG_S_LINK_DATA`.
4. The receiving client's poll calls `NetLink_OnReceiveBlock`, which **queues**
   the block into a 16-deep FIFO rather than writing it immediately.
5. `NetLink_GetBlockReceivedStatus()` — polled by the engine every frame —
   pops one block from the FIFO into `gBlockRecvBuffer[partnerId]` and sets the
   partner's received-bit.

The FIFO is what makes this work: the network delivers blocks whenever TCP feels
like it, but the battle engine consumes exactly one block per exchange and only
after it has cleared the previous flag with `NetLink_ResetBlockReceivedFlag()`.
The queue absorbs the mismatch. `NetLink_RequestSelfSend()` handles the cases
where the engine expects its own block to appear without an explicit send.

Master/slave is decided by who initiated the interaction, and
`NetLink_GetMultiplayerId()` returns 0 for master / 1 for slave — the same
convention the cable uses, so turn order and battle-controller assignment work
unchanged.

---

## 7. Server (`PokeServer.exe`)

~750 lines of single-threaded C, Winsock/BSD sockets, no dependencies beyond
`ws2_32`. Built by one line in `Makefile_pc`:

```
$(CC) -m32 -pipe -O2 -Wall -o $@ server/server.c -lws2_32
```

**Config** — `server.cfg`, `key = value` lines: `localip`, `hostport`, `motd`,
`server-name`, `max-players`, `shiny-rate`. The shiny rate is pushed to clients
in `ServerConfigDataPacket` so the server controls shiny odds server-wide.

**Main loop** — non-blocking accept + per-player non-blocking `recv()` into a
256 KB buffer, framed by `header.size` exactly like the client. `WSAEWOULDBLOCK`
/ `EAGAIN` means "no data", anything else means the player dropped.

**Accounts** — `accounts/<username>.dat`, format:

```
[32-byte SHA-256 password hash][128 KB save data]
```

First login with a given username **creates** the account with whatever hash was
supplied; subsequent logins `memcmp` the hash. Saves are streamed down at login
and streamed back up by `Platform_FlushSaveToServer()`, which fires on a delayed
counter after the game writes flash and pushes all 32 chunks as `MSG_C_SAVE_DATA`.

**Relay policy** — position and chat are broadcast (`BroadcastExcept` /
`BroadcastAll`); party data, trade selections and link blocks are point-to-point
to the paired partner only. The server is a dumb relay: it does not simulate
battles, validate party data, or check movement.

---

## 8. Design notes and known limits

- **Server-authoritative saves, client-authoritative everything else.** The
  server stores your save but never validates it. A modified client can report
  any position, any party, any link block. This is a friends-and-community
  design, not a hardened MMO.
- **Password handling is minimal**: unsalted SHA-256, sent over plaintext TCP,
  and the first login silently claims the username. Fine for a private server,
  not for an open one.
- **Raw struct wire format** means client and server must be built from the same
  `protocol.h` with the same compiler/ABI. `PROTOCOL_VERSION` is sent but not
  currently rejected on mismatch.
- **No reconnect logic.** A dropped TCP connection ends the session; an
  in-progress battle is cancelled for the survivor.
- **Buffer sizes are the real caps**: 8 KB client receive buffer, 16-block link
  FIFO, 3 x 200-byte party chunks (6 Pokémon at 100 bytes).
- **Interpolation is tile-quantized**, so remote players can visibly lag under
  latency and then catch up; deltas over 3 tiles snap instead of catching up.
