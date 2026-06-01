# Decodes an LWL circular buffer dump.
# The script first parses the codebase and builds a database of all lwl_enter_record() calls.
# The codebase must match the firmware version that generated the dump, otherwise the database may be incompatible.
# The dump directory must contain an info.txt file with a line:
#     next_entry_index = <number>
# where <number> is the buffer write position / wrap point.

import re
import struct
from pathlib import Path


ROOT_DIR = "."
DUMP_BIN = "./code/debug/test5/dump.bin"
INFO_PATH = Path(DUMP_BIN).parent / "info.txt"
LWL_DEFINES_FILE = "./code/CM7/Core/lwl/lwl.h"


# ============================================================
# Regex
# ============================================================

COMMENT_REGEX = re.compile(
    r'//.*?$|/\*.*?\*/',
    re.DOTALL | re.MULTILINE
)

DECL_REGEX = re.compile(
    r'\b(?:static\s+|inline\s+|extern\s+)*void\s+lwl_enter_record\s*\('
)

DEF_REGEX = re.compile(
    r'\b(?:\#define\s+|)\s+lwl_enter_record\s*\('
)

CALL_START = re.compile(r'\blwl_enter_record\s*\(')

DEFINE_REGEX = re.compile(
    r'^\s*#define\s+(\w+)\s+(.+)$',
    re.MULTILINE
)

TYPE_SIZES = {
    'c': 1,
    's': 2,
    'h': 2,
    'd': 4,
    'u': 4,
    'f': 4,
}


# ============================================================
# Parsing helpers
# ============================================================

def clean_code(text):
    return re.sub(COMMENT_REGEX, '', text)


def parse_defines(filepath):

    defines = {}

    with open(filepath,'r',encoding='utf-8',errors='ignore') as f:
        content = f.read()

    for name,value in DEFINE_REGEX.findall(content):

        value = value.strip()

        # char literal
        if value.startswith("'") and value.endswith("'"):

            defines[name] = eval(value)

        # string literal
        elif value.startswith('"') and value.endswith('"'):

            defines[name] = value[1:-1]

    return defines


def split_args(arg_string):

    args = []

    current = []
    paren_depth = 0
    in_string = False
    in_char = False
    escape = False

    for c in arg_string:

        current.append(c)

        if escape:
            escape = False
            continue

        if c == '\\':
            escape = True
            continue

        if in_string:
            if c == '"':
                in_string = False
            continue

        if in_char:
            if c == "'":
                in_char = False
            continue

        if c == '"':
            in_string = True
            continue

        if c == "'":
            in_char = True
            continue

        if c == '(':
            paren_depth += 1
            continue

        if c == ')':
            paren_depth -= 1
            continue

        if c == ',' and paren_depth == 0:
            current.pop()
            args.append(''.join(current).strip())
            current = []

    final = ''.join(current).strip()

    if final:
        args.append(final)

    return args


def extract_calls(text):

    matches = []

    for m in CALL_START.finditer(text):

        line_start = text.rfind('\n', 0, m.start()) + 1
        line_end = text.find('\n', m.start())

        if line_end == -1:
            line_end = len(text)

        line = text[line_start:line_end]

        if DECL_REGEX.search(line):
            continue

        if DEF_REGEX.search(line):
            continue

        start = m.start()

        depth = 0
        i = m.end() - 1

        while i < len(text):

            if text[i] == '(':
                depth += 1

            elif text[i] == ')':
                depth -= 1

                if depth == 0:
                    matches.append((start, i + 1))
                    break

            i += 1

    return matches


def calculate_size(args, id_defines):

    size = 1      # module char

    function_str = id_defines[args[1]]

    size += len(function_str)

    if len(args) < 3:
        return size

    fmt = args[2][1:-1]

    for ch in fmt:

        if ch not in TYPE_SIZES:
            print(f"WARNING: unknown format char '{ch}'")
            continue

        size += TYPE_SIZES[ch]

    return size


# ============================================================
# Database generation
# ============================================================

def build_call_database():

    id_defines = parse_defines(LWL_DEFINES_FILE)

    all_calls = []
    seen_keys = {}

    for path in Path(ROOT_DIR).rglob("*"):

        if path.suffix not in (".c", ".h"):
            continue

        content = clean_code(
            path.read_text(encoding='utf-8', errors='ignore')
        )

        for start, end in extract_calls(content):

            line_number = content.count('\n', 0, start) + 1

            call_text = content[start:end]

            arg_text = call_text[
                call_text.find('(')+1:
                call_text.rfind(')')
            ].strip()

            args = split_args(arg_text)

            if len(args) < 2:
                continue

            resolved_key = (
                id_defines[args[0]],
                id_defines[args[1]]
            )

            entry = {
                "args": args,
                "size": calculate_size(args , id_defines),
                "key": resolved_key
            }

            all_calls.append(entry)

            if resolved_key in seen_keys:

                prev_path, prev_line = seen_keys[resolved_key]

                print(
                    f"WARNING: duplicate key {resolved_key}\n"
                    f"    first:  {prev_path}:{prev_line}\n"
                    f"    second: {path}:{line_number}"
                )

            else:
                seen_keys[resolved_key] = (path, line_number)

    return all_calls


def build_decoder_table(all_calls):
    return {entry["key"]: entry for entry in all_calls}


# ============================================================
# Buffer handling
# ============================================================

def load_and_unwrap_buffer(bin_path):

    

    # ---- load binary ----
    with open(bin_path, "rb") as f:
        data = f.read()

    # ---- load next_entry_index ----
    with open(INFO_PATH, 'r', encoding='utf-8') as f:
        info_text = f.read()

    m = re.search(
        r'next_entry_index\s*=\s*(\d+)',
        info_text
    )

    if not m:
        raise RuntimeError(
            f"Could not find next_entry_index in {INFO_PATH}"
        )

    next_entry_index = int(m.group(1))

    # ---- detect "buffer never wrapped" edge case ----
    tail = data[next_entry_index:]

    if all(b == 0 for b in tail):

        unwrap_point = 0

        print(
            "Buffer has not wrapped yet "
            "(tail after next_entry_index is all zeros)."
        )

    else:
        unwrap_point = next_entry_index

    print(f"Using unwrap point: {unwrap_point}")

    # ---- unwrap ----
    unwrapped = (
        data[unwrap_point:]
        + data[:unwrap_point]
    )

    return unwrapped


def choose_decode_point_manually(unwrapped, all_calls):

    largest_call_size = max(
        entry["size"]
        for entry in all_calls
    )

    preview = unwrapped[:largest_call_size]

    print("\nPossible junk region:\n")

    for i in range(largest_call_size):
        print(f"{i:<8}", end="")

    print()

    for b in preview:
        print(f"{b:<8}", end="")

    print("\n")

    decode_point = int(
        input(
            f"Enter decoding point "
            f"(0-{largest_call_size-1}): "
        )
    )

    return unwrapped[decode_point:], decode_point

def try_decode(decoded_buffer, decoder_table):

    offset = 0
    decoded_entries = 0

    while offset < len(decoded_buffer):

        if offset + 1 > len(decoded_buffer):
            return False, decoded_entries, offset

        module_char = chr(decoded_buffer[offset])

        matched_entry = None
        best_len = -1

        for entry in decoder_table.values():

            entry_module, entry_function = entry["key"]

            if entry_module != module_char:
                continue

            function_bytes = entry_function.encode()

            start = offset + 1
            end = start + len(function_bytes)

            if end > len(decoded_buffer):
                continue

            if decoded_buffer[start:end] == function_bytes:

                if len(function_bytes) > best_len:

                    matched_entry = entry
                    best_len = len(function_bytes)

        if matched_entry is None:
            return False, decoded_entries, offset

        size = matched_entry["size"]

        if offset + size > len(decoded_buffer):
            return False, decoded_entries, offset

        offset += size
        decoded_entries += 1

    return True, decoded_entries, offset

def choose_decode_point(unwrapped, all_calls, decoder_table):

    largest_call_size = max(
        entry["size"]
        for entry in all_calls
    )

    candidates = []

    for decode_point in range(largest_call_size):

        candidate_buffer = unwrapped[decode_point:]

        success, entries, bytes_consumed = try_decode(
            candidate_buffer,
            decoder_table
        )

        score = (
            entries * 1000
            + bytes_consumed
            + (100000 if success else 0)
        )

        candidates.append({
            "decode_point": decode_point,
            "success": success,
            "entries": entries,
            "bytes": bytes_consumed,
            "score": score
        })

    best = max(
        candidates,
        key=lambda x: x["score"]
    )

    print()

    for c in candidates:

        print(
            f"point={c['decode_point']:2d}   "
            f"success={c['success']}   "
            f"entries={c['entries']:4d}   "
            f"bytes={c['bytes']:4d}"
        )

    print()

    print(
        f"Selected decode point: "
        f"{best['decode_point']}"
    )

    return (
        unwrapped[best["decode_point"]:],
        best["decode_point"]
    )

# ============================================================
# Decoder
# ============================================================

def decode_buffer(decoded_buffer, decoder_table, decode_point):

    print()

    header = (
        f"{'NUM':<6}"
        f"{'BYTES':<12}"
        f"{'MODULE_ID':<32}"
        f"{'FUNCTION_ID':<32}"
        f"ARGUMENTS"
    )

    print(header)
    print("-" * len(header))

    offset = 0
    entry_num = 0

    while offset < len(decoded_buffer):

        if offset + 1 > len(decoded_buffer):

            print(
                f"ERROR: truncated module ID "
                f"at byte {decode_point + offset}"
            )
            return

        module_char = chr(decoded_buffer[offset])

        matched_entry = None
        best_len = -1

        for entry in decoder_table.values():

            entry_module, entry_function = entry["key"]

            if entry_module != module_char:
                continue

            function_bytes = entry_function.encode()

            start = offset + 1
            end = start + len(function_bytes)

            if end > len(decoded_buffer):
                continue

            if decoded_buffer[start:end] == function_bytes:

                if len(function_bytes) > best_len:

                    matched_entry = entry
                    best_len = len(function_bytes)

        if matched_entry is None:

            print(
                f"ERROR: unknown entry "
                f"at byte {decode_point + offset} "
                f"(module='{module_char}')"
            )

            print(
                f"DEBUG offset={offset} "
                f"bytes={decoded_buffer[offset:offset+15]}"
            )

            return

        args = matched_entry["args"]
        size = matched_entry["size"]

        if offset + size > len(decoded_buffer):

            print(
                f"ERROR: truncated entry "
                f"at byte {decode_point + offset}"
            )

            return

        module_id, function_id = matched_entry["key"]

        function_bytes = function_id.encode()

        fmt = args[2][1:-1]

        pos = (
            offset
            + 1
            + len(function_bytes)
        )

        module_str = (
            f"{args[0]} ({module_id})"
        )

        function_str = (
            f'{args[1]} ("{function_id}")'
        )

        arg_output = []

        for fmt_char, name in zip(fmt, args[3:]):

            if fmt_char == 'c':

                value = decoded_buffer[pos]
                pos += 1

            elif fmt_char == 's':

                value = int.from_bytes(
                    decoded_buffer[pos:pos+2],
                    'little',
                    signed=True
                )

                pos += 2

            elif fmt_char == 'h':

                value = int.from_bytes(
                    decoded_buffer[pos:pos+2],
                    'little',
                    signed=False
                )

                pos += 2

            elif fmt_char == 'd':

                value = int.from_bytes(
                    decoded_buffer[pos:pos+4],
                    'little',
                    signed=True
                )

                pos += 4

            elif fmt_char == 'u':

                value = int.from_bytes(
                    decoded_buffer[pos:pos+4],
                    'little',
                    signed=False
                )

                pos += 4

            elif fmt_char == 'f':

                value = struct.unpack(
                    '<f',
                    decoded_buffer[pos:pos+4]
                )[0]

                pos += 4

            else:

                print(
                    f"ERROR: unknown format "
                    f"'{fmt_char}'"
                )

                return

            arg_output.append(
                f"{name} = {value}"
            )

        byte_start = decode_point + offset
        byte_end = byte_start + size - 1

        print(
            f"{entry_num:<6}"
            f"{f'{byte_start}-{byte_end}':<12}"
            f"{module_str:<32}"
            f"{function_str:<32}"
            f"{'    '.join(arg_output)}"
        )

        offset += size
        entry_num += 1


# ============================================================
# Main
# ============================================================

def main():

    all_calls = build_call_database()

    for entry in all_calls:
        print( entry )

    unwrapped = load_and_unwrap_buffer(
        DUMP_BIN
    )

    decoder_table = build_decoder_table(
        all_calls
    )

    decoded_buffer, decode_point = choose_decode_point(
        unwrapped,
        all_calls,
        decoder_table
    )

    decode_buffer(
        decoded_buffer,
        decoder_table,
        decode_point
    )

if __name__ == "__main__":
    main()