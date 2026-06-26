#!/usr/bin/env python3
"""
rinex2ubx_ambit.py - Convert RINEX 3 broadcast nav to Suunto Ambit3 GPS orbit binary.

Downloads today's GPS ephemeris from Geoscience Australia (free, no login) and
encodes it as UBX-AID-EPH messages in the format expected by the Ambit3 Sport.

Header bytes 6-9 of the output blob encode [year_lo, year_hi, month, day] as
required by gps_orbit_write() in src/libambit/device_driver_ambit3.c:624.

Usage:
    python3 tools/rinex2ubx_ambit.py [--output ~/.openambit/orbit_cache.uo]
"""

import struct
import math
import gzip
import json
import datetime
import os
import sys
import argparse
import urllib.request
import urllib.error

PI = math.pi
GA_API = "https://data.gnss.ga.gov.au/api/rinexFiles/"
STATION = "ALIC"          # Alice Springs — reliable GA reference station
DEFAULT_OUT = os.path.expanduser("~/.openambit/orbit_cache.uo")


# ---------------------------------------------------------------------------
# RINEX 3 download
# ---------------------------------------------------------------------------

def fetch_rinex_content(date: datetime.date) -> str:
    """Download RINEX 3 mixed nav file from Geoscience Australia for given date.
    Falls back to the previous day if today's file is not yet available."""
    for delta in (0, 1):
        target = date - datetime.timedelta(days=delta)
        date_str = target.strftime("%Y-%m-%d")
        url = (f"{GA_API}?stationId={STATION}&fileType=nav"
               f"&startDate={date_str}&endDate={date_str}&filePeriod=01D")
        try:
            with urllib.request.urlopen(url, timeout=30) as resp:
                entries = json.loads(resp.read())
            if not entries:
                continue
            file_url = entries[0]["fileLocation"]
            print(f"  Downloading nav file for {date_str} ({entries[0]['fileSize']//1024} KB)...")
            with urllib.request.urlopen(file_url, timeout=120) as resp:
                raw = resp.read()
            return gzip.decompress(raw).decode("ascii", errors="replace"), target
        except urllib.error.URLError as e:
            print(f"  Warning: {e}", file=sys.stderr)
    raise RuntimeError("Could not fetch RINEX nav file from Geoscience Australia.")


# ---------------------------------------------------------------------------
# RINEX 3 GPS nav parser
# ---------------------------------------------------------------------------

def _rinex_float(s: str) -> float:
    """Parse RINEX float (uses 'D' as exponent separator)."""
    return float(s.strip().replace("D", "E").replace("d", "e"))


def parse_gps_nav(content: str) -> list:
    """Return list of GPS ephemeris dicts from a RINEX 3 Mixed Nav file."""
    lines = content.splitlines()
    i = 0
    while i < len(lines) and "END OF HEADER" not in lines[i]:
        i += 1
    i += 1

    records = []
    while i < len(lines):
        line = lines[i]
        if len(line) < 4 or line[0] != "G":
            i += 1
            continue
        try:
            prn     = int(line[1:3])
            year    = int(line[4:8])
            month   = int(line[9:11])
            day     = int(line[12:14])
            hour    = int(line[15:17])
            minute  = int(line[18:20])
            second  = _rinex_float(line[21:23])
            af0     = _rinex_float(line[23:42])
            af1     = _rinex_float(line[42:61])
            af2     = _rinex_float(line[61:80])

            def orbit_line(offset):
                return lines[i + offset]

            l1 = orbit_line(1)
            IODE    = _rinex_float(l1[4:23]);  Crs     = _rinex_float(l1[23:42])
            delta_n = _rinex_float(l1[42:61]); M0      = _rinex_float(l1[61:80])

            l2 = orbit_line(2)
            Cuc     = _rinex_float(l2[4:23]);  e       = _rinex_float(l2[23:42])
            Cus     = _rinex_float(l2[42:61]); sqrt_A  = _rinex_float(l2[61:80])

            l3 = orbit_line(3)
            toe     = _rinex_float(l3[4:23]);  Cic     = _rinex_float(l3[23:42])
            Omega0  = _rinex_float(l3[42:61]); Cis     = _rinex_float(l3[61:80])

            l4 = orbit_line(4)
            i0      = _rinex_float(l4[4:23]);  Crc     = _rinex_float(l4[23:42])
            omega   = _rinex_float(l4[42:61]); Omega_dot = _rinex_float(l4[61:80])

            l5 = orbit_line(5)
            IDOT    = _rinex_float(l5[4:23]);  codes_L2  = _rinex_float(l5[23:42])
            gps_week= _rinex_float(l5[42:61]); L2P_flag  = _rinex_float(l5[61:80])

            l6 = orbit_line(6)
            sv_acc  = _rinex_float(l6[4:23]);  sv_health = _rinex_float(l6[23:42])
            TGD     = _rinex_float(l6[42:61]); IODC      = _rinex_float(l6[61:80])

            l7 = orbit_line(7)
            t_tx    = _rinex_float(l7[4:23])
            fit_int = _rinex_float(l7[23:42]) if len(l7.rstrip()) > 23 else 0.0

            records.append(dict(
                prn=prn, year=year, month=month, day=day,
                hour=hour, minute=minute, second=second,
                af0=af0, af1=af1, af2=af2,
                IODE=IODE, Crs=Crs, delta_n=delta_n, M0=M0,
                Cuc=Cuc, e=e, Cus=Cus, sqrt_A=sqrt_A,
                toe=toe, Cic=Cic, Omega0=Omega0, Cis=Cis,
                i0=i0, Crc=Crc, omega=omega, Omega_dot=Omega_dot,
                IDOT=IDOT, codes_L2=codes_L2, gps_week=gps_week,
                L2P_flag=L2P_flag, sv_accuracy=sv_acc, sv_health=sv_health,
                TGD=TGD, IODC=IODC, t_tx=t_tx, fit_interval=fit_int,
            ))
            i += 8
        except (ValueError, IndexError):
            i += 1

    return records


def latest_per_prn(records: list) -> dict:
    """Keep only the most recent ephemeris record per PRN."""
    best = {}
    for r in records:
        prn = r["prn"]
        if 1 <= prn <= 32:
            key = (r["hour"], r["minute"])
            if prn not in best or key > (best[prn]["hour"], best[prn]["minute"]):
                best[prn] = r
    return best


# ---------------------------------------------------------------------------
# GPS ICD-200 subframe encoding
# ---------------------------------------------------------------------------

def _ura_index(accuracy_m: float) -> int:
    """Convert URA accuracy in metres to 4-bit URA index (IS-GPS-200 table)."""
    thresholds = [2.4, 3.4, 4.85, 6.85, 9.65, 13.65, 24, 48, 96, 192, 384, 768, 1536, 3072, 6144]
    for idx, t in enumerate(thresholds):
        if accuracy_m <= t:
            return idx
    return 15


def _scale(value: float, pow2: int, bits: int, signed: bool = True) -> int:
    """Scale float to fixed-point integer of `bits` width.

    Returns the two's-complement bit pattern masked to `bits` bits.
    Wrapping (not clamping) is intentional for angular parameters.
    """
    ival = round(value / (2.0 ** pow2))
    if signed:
        return int(ival) & ((1 << bits) - 1)
    else:
        ival = int(ival)
        if ival < 0:
            ival = 0
        return min(ival, (1 << bits) - 1)


def _gps_parity(data24: int, d29s: int, d30s: int):
    """Compute IS-GPS-200 Table 20-XIV parity for 24 data bits.

    Returns (parity_6bit, new_D29, new_D30).
    If d30s == 1 the data bits are complemented before parity computation
    (the complemented bits are what gets stored in the word).
    """
    if d30s:
        data24 ^= 0xFFFFFF
    b = [(data24 >> (23 - k)) & 1 for k in range(24)]
    D25 = d29s^b[0]^b[1]^b[2]^b[4]^b[5]^b[9]^b[10]^b[11]^b[12]^b[13]^b[16]^b[17]^b[19]^b[22]
    D26 = d30s^b[1]^b[2]^b[3]^b[5]^b[6]^b[10]^b[11]^b[12]^b[13]^b[14]^b[17]^b[18]^b[20]^b[23]
    D27 = d29s^b[0]^b[2]^b[3]^b[4]^b[6]^b[7]^b[11]^b[12]^b[13]^b[14]^b[15]^b[18]^b[19]^b[21]
    D28 = d30s^b[1]^b[3]^b[4]^b[5]^b[7]^b[8]^b[12]^b[13]^b[14]^b[15]^b[16]^b[19]^b[20]^b[22]
    D29 = d30s^b[0]^b[2]^b[4]^b[5]^b[6]^b[8]^b[9]^b[13]^b[14]^b[15]^b[16]^b[17]^b[20]^b[21]^b[23]
    D30 = d29s^b[1]^b[3]^b[5]^b[6]^b[7]^b[9]^b[10]^b[12]^b[14]^b[18]^b[21]^b[22]^b[23]
    parity = (D25<<5)|(D26<<4)|(D27<<3)|(D28<<2)|(D29<<1)|D30
    return parity, D29, D30


def _word(data24: int, d29s: int, d30s: int):
    """Build a 30-bit GPS word (as 32-bit int) and return (word32, new_d29, new_d30)."""
    parity, nd29, nd30 = _gps_parity(data24, d29s, d30s)
    stored = (data24 ^ (0xFFFFFF if d30s else 0)) & 0xFFFFFF
    return (stored << 6) | parity, nd29, nd30


def _encode_subframe1(eph: dict) -> tuple:
    """Return (list_of_8_words, how_word) for GPS subframe 1."""
    wn      = int(eph["gps_week"]) & 0x3FF
    codes   = int(eph["codes_L2"]) & 0x3
    ura     = _ura_index(eph["sv_accuracy"])
    health  = int(eph["sv_health"]) & 0x3F
    IODC    = int(eph["IODC"]) & 0x3FF
    L2P     = int(eph["L2P_flag"]) & 1

    toc_sec = eph["hour"] * 3600 + eph["minute"] * 60 + int(eph["second"])
    TOC     = _scale(toc_sec, 4, 16, signed=False)
    TGD     = _scale(eph["TGD"], -31, 8, signed=True)
    AF2     = _scale(eph["af2"], -55, 8, signed=True)
    AF1     = _scale(eph["af1"], -43, 16, signed=True)
    AF0     = _scale(eph["af0"], -31, 22, signed=True)

    d29, d30 = 0, 0
    words = []

    # Word 3: WN(10) CodesL2(2) URA(4) Health(6) IODC_MSB(2)
    d24 = (wn<<14)|(codes<<12)|(ura<<8)|(health<<2)|((IODC>>8)&0x3)
    w, d29, d30 = _word(d24, d29, d30); words.append(w)

    # Word 4: L2P flag at bit 23
    d24 = (L2P & 1) << 23
    w, d29, d30 = _word(d24, d29, d30); words.append(w)

    # Words 5-6: reserved
    for _ in range(2):
        w, d29, d30 = _word(0, d29, d30); words.append(w)

    # Word 7: TGD(8) at bits 23:16
    d24 = (TGD & 0xFF) << 16
    w, d29, d30 = _word(d24, d29, d30); words.append(w)

    # Word 8: IODC_LSB(8) at 23:16 | TOC(16) at 15:0
    d24 = ((IODC & 0xFF) << 16) | (TOC & 0xFFFF)
    w, d29, d30 = _word(d24, d29, d30); words.append(w)

    # Word 9: AF2(8) at 23:16 | AF1(16) at 15:0
    d24 = ((AF2 & 0xFF) << 16) | (AF1 & 0xFFFF)
    w, d29, d30 = _word(d24, d29, d30); words.append(w)

    # Word 10: AF0(22) at 23:2
    d24 = (AF0 & 0x3FFFFF) << 2
    w, d29, d30 = _word(d24, d29, d30); words.append(w)

    # HOW for SF1: subframe_id=1 in bits 10:8
    how = 1 << 8
    return words, how


def _encode_subframe2(eph: dict) -> list:
    """Return list of 8 words for GPS subframe 2."""
    IODE    = int(eph["IODE"]) & 0xFF
    Crs     = _scale(eph["Crs"],              -5,  16, signed=True)
    delta_n = _scale(eph["delta_n"] / PI,    -43,  16, signed=True)
    M0      = _scale(eph["M0"]      / PI,    -31,  32, signed=True)
    Cuc     = _scale(eph["Cuc"],             -29,  16, signed=True)
    e       = _scale(eph["e"],               -33,  32, signed=False)
    Cus     = _scale(eph["Cus"],             -29,  16, signed=True)
    sqrtA   = _scale(eph["sqrt_A"],          -19,  32, signed=False)
    TOE     = _scale(eph["toe"],               4,  16, signed=False)
    fit     = 1 if eph.get("fit_interval", 0) > 4 else 0

    d29, d30 = 0, 0
    words = []

    # Word 3: IODE(8) | CRS(16)
    d24 = ((IODE & 0xFF) << 16) | (Crs & 0xFFFF)
    w, d29, d30 = _word(d24, d29, d30); words.append(w)

    # Word 4: DELTA_N(16) | M0_MSB(8)
    d24 = ((delta_n & 0xFFFF) << 8) | ((M0 >> 24) & 0xFF)
    w, d29, d30 = _word(d24, d29, d30); words.append(w)

    # Word 5: M0_LSB(24)
    w, d29, d30 = _word(M0 & 0xFFFFFF, d29, d30); words.append(w)

    # Word 6: CUC(16) | E_MSB(8)
    d24 = ((Cuc & 0xFFFF) << 8) | ((e >> 24) & 0xFF)
    w, d29, d30 = _word(d24, d29, d30); words.append(w)

    # Word 7: E_LSB(24)
    w, d29, d30 = _word(e & 0xFFFFFF, d29, d30); words.append(w)

    # Word 8: CUS(16) | SQRTA_MSB(8)
    d24 = ((Cus & 0xFFFF) << 8) | ((sqrtA >> 24) & 0xFF)
    w, d29, d30 = _word(d24, d29, d30); words.append(w)

    # Word 9: SQRTA_LSB(24)
    w, d29, d30 = _word(sqrtA & 0xFFFFFF, d29, d30); words.append(w)

    # Word 10: TOE(16) | fit_flag(1) | AODO=0(5)
    d24 = ((TOE & 0xFFFF) << 8) | ((fit & 1) << 7)
    w, d29, d30 = _word(d24, d29, d30); words.append(w)

    return words


def _encode_subframe3(eph: dict) -> list:
    """Return list of 8 words for GPS subframe 3."""
    IODE      = int(eph["IODE"]) & 0xFF
    Cic       = _scale(eph["Cic"],             -29, 16, signed=True)
    Omega0    = _scale(eph["Omega0"]  / PI,    -31, 32, signed=True)
    Cis       = _scale(eph["Cis"],             -29, 16, signed=True)
    i0        = _scale(eph["i0"]      / PI,    -31, 32, signed=True)
    Crc       = _scale(eph["Crc"],              -5, 16, signed=True)
    omega     = _scale(eph["omega"]   / PI,    -31, 32, signed=True)
    Omega_dot = _scale(eph["Omega_dot"] / PI,  -43, 24, signed=True)
    IDOT      = _scale(eph["IDOT"]    / PI,    -43, 14, signed=True)

    d29, d30 = 0, 0
    words = []

    # Word 3: CIC(16) | OMEGA0_MSB(8)
    d24 = ((Cic & 0xFFFF) << 8) | ((Omega0 >> 24) & 0xFF)
    w, d29, d30 = _word(d24, d29, d30); words.append(w)

    # Word 4: OMEGA0_LSB(24)
    w, d29, d30 = _word(Omega0 & 0xFFFFFF, d29, d30); words.append(w)

    # Word 5: CIS(16) | I0_MSB(8)
    d24 = ((Cis & 0xFFFF) << 8) | ((i0 >> 24) & 0xFF)
    w, d29, d30 = _word(d24, d29, d30); words.append(w)

    # Word 6: I0_LSB(24)
    w, d29, d30 = _word(i0 & 0xFFFFFF, d29, d30); words.append(w)

    # Word 7: CRC(16) | OMEGA_MSB(8)
    d24 = ((Crc & 0xFFFF) << 8) | ((omega >> 24) & 0xFF)
    w, d29, d30 = _word(d24, d29, d30); words.append(w)

    # Word 8: OMEGA_LSB(24)
    w, d29, d30 = _word(omega & 0xFFFFFF, d29, d30); words.append(w)

    # Word 9: OMEGA_DOT(24)
    w, d29, d30 = _word(Omega_dot & 0xFFFFFF, d29, d30); words.append(w)

    # Word 10: IODE(8) | IDOT(14) at bits 15:2
    d24 = ((IODE & 0xFF) << 16) | ((IDOT & 0x3FFF) << 2)
    w, d29, d30 = _word(d24, d29, d30); words.append(w)

    return words


# ---------------------------------------------------------------------------
# UBX message builders
# ---------------------------------------------------------------------------

def _ubx_checksum(data: bytes) -> bytes:
    ck_a = ck_b = 0
    for b in data:
        ck_a = (ck_a + b) & 0xFF
        ck_b = (ck_b + ck_a) & 0xFF
    return bytes([ck_a, ck_b])


def _ubx_frame(cls: int, id_: int, payload: bytes) -> bytes:
    hdr = bytes([0xB5, 0x62, cls, id_]) + struct.pack("<H", len(payload))
    body = hdr + payload
    return body + _ubx_checksum(body[2:])


def make_aid_eph(eph: dict) -> bytes:
    """Build UBX-AID-EPH (class 0x0B, id 0x31) SET message, 104-byte payload."""
    sf1_words, how = _encode_subframe1(eph)
    sf2_words = _encode_subframe2(eph)
    sf3_words = _encode_subframe3(eph)

    payload = struct.pack("<I", eph["prn"])   # svid
    payload += struct.pack("<I", how)          # how
    for w in sf1_words:
        payload += struct.pack("<I", w)
    for w in sf2_words:
        payload += struct.pack("<I", w)
    for w in sf3_words:
        payload += struct.pack("<I", w)

    assert len(payload) == 104, f"AID-EPH payload length {len(payload)} != 104"
    return _ubx_frame(0x0B, 0x31, payload)


def make_aid_ini(date: datetime.date) -> bytes:
    """Build UBX-AID-INI with approximate GPS week/time, unknown position."""
    gps_epoch = datetime.date(1980, 1, 6)
    days_since = (date - gps_epoch).days
    wno = days_since // 7
    # TOW = day-of-week × seconds/day × ms/s
    tow_ms = (days_since % 7) * 24 * 3600 * 1000

    payload  = struct.pack("<iii",  0, 0, 0)        # ecefXYZ = unknown
    payload += struct.pack("<I",    0xFFFFFFFF)      # posAcc = max (unknown)
    payload += struct.pack("<H",    0x0002)          # tmCfg: GPS time, time valid
    payload += struct.pack("<H",    wno & 0xFFFF)    # wno
    payload += struct.pack("<I",    tow_ms)          # tow (ms)
    payload += struct.pack("<i",    0)               # towNs
    payload += struct.pack("<I",    1000)            # tAccMs (1 second)
    payload += struct.pack("<I",    0)               # tAccNs
    payload += struct.pack("<i",    0)               # clkD
    payload += struct.pack("<I",    0)               # freq
    payload += struct.pack("<I",    0)               # freqAcc
    payload += struct.pack("<I",    0x02)            # flags: timeValid

    return _ubx_frame(0x0B, 0x01, payload)


# ---------------------------------------------------------------------------
# Suunto orbit blob assembly
# ---------------------------------------------------------------------------

def build_orbit_blob(date: datetime.date, ubx_stream: bytes) -> bytes:
    """
    Build Suunto Ambit3 GPS orbit binary blob.

    Layout (from device_driver_ambit3.c:gps_orbit_write()):
      bytes 0-5:   6-byte prefix (zeroed; Suunto does not document this)
      bytes 6-7:   year, little-endian uint16
      byte  8:     month
      byte  9:     day
      bytes 10-13: GPS epoch seconds of midnight (LE uint32)
      bytes 14+:   UBX message stream
    """
    gps_epoch = datetime.date(1980, 1, 6)
    gps_secs = int((date - gps_epoch).days * 86400)

    header  = bytes(6)                                   # prefix (unknown, zeroed)
    header += struct.pack("<H", date.year)               # year LE
    header += struct.pack("<BB", date.month, date.day)   # month, day
    header += struct.pack("<I", gps_secs)                # GPS seconds of midnight

    return header + ubx_stream


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--output", default=DEFAULT_OUT,
                        help=f"Output file path (default: {DEFAULT_OUT})")
    args = parser.parse_args()

    today = datetime.date.today()
    print(f"Fetching GPS broadcast ephemeris (date: {today})...")
    content, nav_date = fetch_rinex_content(today)

    print("Parsing RINEX 3 GPS records...")
    records = parse_gps_nav(content)
    per_prn = latest_per_prn(records)
    print(f"  Found {len(per_prn)} GPS satellites (PRNs: {sorted(per_prn)})")

    ubx_messages = [make_aid_ini(today)]
    ok = failed = 0
    for prn in sorted(per_prn):
        try:
            msg = make_aid_eph(per_prn[prn])
            ubx_messages.append(msg)
            ok += 1
        except Exception as e:
            print(f"  PRN {prn:02d}: encoding failed ({e})", file=sys.stderr)
            failed += 1

    print(f"  Encoded {ok} satellites ({failed} failed)")

    ubx_stream = b"".join(ubx_messages)
    blob = build_orbit_blob(nav_date, ubx_stream)

    out_path = args.output
    os.makedirs(os.path.dirname(os.path.abspath(out_path)), exist_ok=True)
    with open(out_path, "wb") as f:
        f.write(blob)

    print(f"Written {len(blob)} bytes to {out_path}")
    print()
    print("NOTE: src/movescount/movescount.cpp GPS_ORBIT_DATA_MIN_SIZE (30000 bytes)")
    print("      must be reduced to accept this smaller broadcast-ephemeris file.")
    print("      Patch: change GPS_ORBIT_DATA_MIN_SIZE to 1000")


if __name__ == "__main__":
    main()
