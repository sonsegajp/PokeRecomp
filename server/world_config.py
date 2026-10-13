"""world_config.py — a world's settings file (what gameserver.py --config reads).

    # lines starting with # are notes
    name = My Kanto World
    max_players = 50
    shiny_odds = 4096

Every setting is one of gameserver.py's options under a friendlier name;
anything given on the command line wins over the file. write_template()
makes a commented file with every setting at its current value.
"""

# setting -> (gameserver option, kind, note)
SETTINGS = [
    ("name", "name", str, "shown in every player's ONLINE CLUB server browser (24 letters max)"),
    ("region", "region", str, "kanto (FireRed / LeafGreen) or hoenn (Emerald): set by the .bat you host with"),
    ("port", "port", int, "TCP port players connect to: forward it on your router"),
    ("max_players", "max", int, "how many trainers can be on at once (up to 200)"),
    ("welcome", "welcome", str, "a chat line everyone sees when they arrive (empty = none)"),
    None,
    ("shiny_odds", "base_shiny_odds", int, "1 in N for every wild Pokemon (the games' own rate is 8192; best 32)"),
    ("boost_shiny_odds", "shiny_odds", int, "1 in N for the boosted type (best 32)"),
    ("boost_type", "boost_type", str, "daily (a new type every day), off, or a type such as FIRE"),
    None,
    ("outbreaks", "outbreaks", bool, "on / off: timed mass outbreaks of a rarer Pokemon somewhere"),
    ("outbreak_every", "event_every", float, "minutes between outbreaks"),
    ("outbreak_length", "event_length", float, "minutes an outbreak lasts"),
    ("outbreak_share", "outbreak_percent", int, "% of the wild encounters there that are the outbreak"),
    ("first_outbreak", "first_event", float, "minutes after the world starts before the first one"),
    None,
    ("legality_checks", "legality", bool, "on / off: turn away hacked Pokemon in battles and trades"),
    ("battles", "battles", bool, "on / off: link battles between players"),
    ("trades", "trades", bool, "on / off: link trades between players"),
]

TRUE, FALSE = ("on", "yes", "true", "1"), ("off", "no", "false", "0")


def parse_bool(text):
    t = text.strip().lower()
    if t in TRUE:
        return True
    if t in FALSE:
        return False
    raise ValueError("expected on or off, not %r" % text)


def read(path):
    """The file's settings as gameserver option names -> values."""
    known = {s[0]: s for s in SETTINGS if s}
    out = {}
    with open(path, encoding="utf-8-sig") as f:
        for n, line in enumerate(f, 1):
            line = line.strip()
            if not line or line.startswith(("#", ";")):
                continue
            if "=" not in line:
                raise ValueError("%s line %d: expected setting = value" % (path, n))
            key, value = (x.strip() for x in line.split("=", 1))
            if key not in known:
                raise ValueError("%s line %d: no setting called %r" % (path, n, key))
            _, option, kind, _ = known[key]
            try:
                out[option] = parse_bool(value) if kind is bool else kind(value)
            except ValueError as e:
                raise ValueError("%s line %d: %s: %s" % (path, n, key, e))
    return out


def write_template(path, args):
    lines = ["# PokeRecomp world settings. Edit, save, and (re)start the world.",
             "# Lines starting with # are notes.", ""]
    for s in SETTINGS:
        if s is None:
            lines.append("")
            continue
        key, option, kind, note = s
        value = getattr(args, option)
        if kind is bool:
            value = "on" if value else "off"
        lines.append("# " + note)
        lines.append("%s = %s" % (key, value))
    with open(path, "w", encoding="utf-8") as f:
        f.write("\n".join(lines) + "\n")
