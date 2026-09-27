###
# SEL patcher
# Patches an RSO's symbol table file with the symbol offsets of another executable binary.
###

import struct
import sys
from dataclasses import dataclass
from elftools.elf.constants import SHN_INDICES
from elftools.elf.elffile import ELFFile
from elftools.elf.sections import SymbolTableSection

RSO_SYMBOL_ENTRY_SIZE = 0x10

@dataclass
class ELFSymbol:
    name: str
    address: int
    size: int
    section_index: int
    bind: int
    type: int

def read_u32(data: bytearray, offset: int) -> int:
    return struct.unpack_from(">I", data, offset)[0]

def write_u32(data: bytearray, offset: int, value: int) -> None:
    struct.pack_into(">I", data, offset, value)

def read_str(data: bytearray, offset: int) -> str | None:
    end = data.find(b'\0', offset)

    if end == -1:
        return None

    return data[offset:end].decode("ascii")

def get_symbols(elf_stream) -> dict[str, list[ELFSymbol]]:
    elf = ELFFile(elf_stream)

    symbol_table = elf.get_section_by_name(".symtab")
    if not isinstance(symbol_table, SymbolTableSection):
        raise Exception("No symbol table found in ELF")

    symbol_map: dict[str, list[ELFSymbol]] = {}

    for symbol in symbol_table.iter_symbols():
        name = symbol.name
        if not name:
            continue

        entry = symbol.entry
        symbol_map.setdefault(name, []).append(ELFSymbol(
            name =  name,
            address =  entry["st_value"],
            size =  entry["st_size"],
            section_index =  entry["st_shndx"],
            bind =  entry["st_info"]["bind"],
            type =  entry["st_info"]["type"],
        ))

    return symbol_map

def get_symbol(name: str, symbol_map: dict[str, list[ELFSymbol]]) -> ELFSymbol | None:
    symbols = symbol_map.get(name)

    if not symbols:
        return None

    for symbol in symbols:
        if symbol.address != 0 and symbol.section_index != SHN_INDICES.SHN_UNDEF:
            return symbol

    return None

def patch_sel(elf_path, sel_path, out_path):
    with open(elf_path, 'rb') as elf_stream:
        symbol_map = get_symbols(elf_stream)

    section_map = {
        1: symbol_map["_f_init"][0],
        2: symbol_map["_f_text"][0],
        # 3: unmapped (.ctors)
        # 4: unmapped (.dtors)
        5: symbol_map["_f_rodata"][0],
        6: symbol_map["_f_data"][0],
        7: symbol_map["_f_bss"][0],
        8: symbol_map["_f_sdata"][0],
        9: symbol_map["_f_sdata2"][0],
        # 10: unmapped
        11: symbol_map["_f_sbss"][0],
        12: symbol_map["_f_sbss2"][0],
        # 13: unmapped
    }

    with open(sel_path, 'rb') as sel_stream:
        sel_data = bytearray(sel_stream.read())

    export_table_offset = read_u32(sel_data, 0x40)
    export_table_size = read_u32(sel_data, 0x44)
    export_names_offset = read_u32(sel_data, 0x48)

    current_offset = export_table_offset
    end_offset = export_table_offset + export_table_size

    is_error = False

    while current_offset < end_offset:
        name_offset = read_u32(sel_data, current_offset)
        section_index = read_u32(sel_data, current_offset + 0x8)

        name = read_str(sel_data, export_names_offset + name_offset)
        if not name:
            is_error = True
            print(f"[ERR] Symbol name missing")
            current_offset += RSO_SYMBOL_ENTRY_SIZE
            continue

        elf_symbol = get_symbol(name, symbol_map)
        if not elf_symbol:
            is_error = True
            print(f"[ERR] Missing symbol name '{name}'")
            current_offset += RSO_SYMBOL_ENTRY_SIZE
            continue

        if section_index != 65521:
            elf_section_base = section_map.get(section_index)
        else:
            elf_section_base = ELFSymbol("", 0, 0, 0, 0, 0)

        if not elf_section_base:
            is_error = True
            print(f"[ERR] Invalid symbol section '{name}', '{elf_symbol.address:X}', {section_index}")
            current_offset += RSO_SYMBOL_ENTRY_SIZE
            continue

        offset = elf_symbol.address - elf_section_base.address
        print(f"[LOG] Mapped symbol '{name}' to offset 0x{offset:08X}")
        write_u32(sel_data, current_offset + 0x4, offset)
        current_offset += RSO_SYMBOL_ENTRY_SIZE

    if is_error:
        raise Exception(f"[ERR] too many errors")

    with open(out_path, 'wb') as out_stream:
        out_stream.write(sel_data)

if __name__ == "__main__":
    elf_path: str = sys.argv[1]
    sel_path: str = sys.argv[2]
    out_path: str = sys.argv[3]
    patch_sel(elf_path, sel_path, out_path)
