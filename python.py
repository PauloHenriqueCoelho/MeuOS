import struct
import sys
import os

# Configurações baseadas no seu fs.h
TABLE_SECTOR = 1
DATA_START_SECTOR = 2
MAX_FILES = 10
DISK_PATH = "disk.img"

# Cálculo da sua struct: 
# name[16] (16) + sector(4) + used(1) + padding(3) + size(4) = 28 bytes
ENTRY_SIZE = 28 

def inject_file(os_filename, local_path):
    if not os.path.exists(local_path):
        print(f"Erro: Arquivo '{local_path}' nao encontrado.")
        return

    with open(local_path, "rb") as f:
        content = f.read()
    
    file_size = len(content)
    if file_size > 512:
        print("Erro: O arquivo deve ter no maximo 512 bytes.")
        return

    with open(DISK_PATH, "r+b") as disk:
        # 1. Ler a tabela de arquivos (Setor 1)
        disk.seek(TABLE_SECTOR * 512)
        table_raw = bytearray(disk.read(512))
        
        # 2. Procurar slot livre verificando o campo 'used' (Offset 20 na sua struct)
        free_slot = -1
        for i in range(MAX_FILES):
            offset = i * ENTRY_SIZE
            if table_raw[offset + 20] == 0: # Offset do campo 'used'
                free_slot = i
                break
        
        if free_slot == -1:
            print("Erro: Disco cheio ou sem slots livres!")
            return

        # 3. Escrever o conteúdo no setor de dados
        sector = DATA_START_SECTOR + free_slot
        disk.seek(sector * 512)
        disk.write(content.ljust(512, b'\0'))

        # 4. Preparar a nova entrada usando struct.pack para manter o alinhamento do C
        # Formato: 16s (nome), I (sector), B (used), 3x (padding), I (size)
        new_entry = struct.pack("<16s I B 3x I", 
                                os_filename.encode('ascii')[:15], 
                                sector, 
                                1,     # used = 1
                                file_size)

        # 5. Gravar na tabela
        entry_offset = free_slot * ENTRY_SIZE
        table_raw[entry_offset : entry_offset + ENTRY_SIZE] = new_entry
        
        disk.seek(TABLE_SECTOR * 512)
        disk.write(table_raw)
        print(f"Sucesso: {os_filename} injetado no setor {sector} ({file_size} bytes).")

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Uso: python3 inject.py <nome_no_os> <arquivo_local>")
    else:
        inject_file(sys.argv[1], sys.argv[2])