import os
import ctypes

# Загрузка DLL
compress_lib = ctypes.CDLL('.\Compessor.dll')

# Определение типов аргументов и возвращаемого значения функции Compress_buf
compress_lib.Compress_buf.argtypes = [
    ctypes.c_void_p,       # in_buf
    ctypes.c_uint,         # in_buf_sz
    ctypes.c_void_p,       # out_buf
    ctypes.c_uint,         # out_buf_sz
    ctypes.c_ubyte         # alg
]
compress_lib.Compress_buf.restype = ctypes.c_int


# Шаг 1: Прочитать список файлов
files_to_compress = [
                     r'.\CYW4373A0_BT_patch.hcd',
                     r'.\4373A0.bin',
                     r'.\4373A0.clm_blob'
                     ]
output_c_file = '..\..\src\Infineon_fw_blobs\Infineon_BLOBs.c'
output_h_file = '..\..\src\Infineon_fw_blobs\Infineon_BLOBs.h'

files_cnt = 0;
# Шаг 2: Прочитать содержимое каждого файла и сжать его
compressed_files_data = {}
for file in files_to_compress:
    with open(file, 'rb') as f:
        file_data = f.read()

        # Создание выходного буфера
        compressed_data_size = len(file_data) * 2  # Предположим, что выходной буфер будет достаточно большим
        compressed_data = ctypes.create_string_buffer(compressed_data_size)

        # Алгоритм сжатия 
        algorithm = 0 # SIXPACK

        # Вызов функции Compress_buf
        result = compress_lib.Compress_buf(
          ctypes.byref(ctypes.create_string_buffer(file_data)),
          len(file_data),
          ctypes.byref(compressed_data),
          compressed_data_size,
          algorithm
          )
        compressed_files_data[file] = compressed_data.raw[:result]
        files_cnt = files_cnt + 1

full_size = 0;
# Шаг 3: Создать файл на C и записать данные из каждого файла в виде отдельного массива
with open(output_h_file, 'w') as h:
    h.write('#ifndef _INFINEON_BLOBS_H\n')
    h.write('  #define _INFINEON_BLOBS_H\n\n')
    h.write('typedef struct {\n')
    h.write('    const char* name;\n')
    h.write('    const uint8_t* data;\n')
    h.write('    uint32_t size;\n')
    h.write('} T_infineon_blob;\n\n')

    h.write(f'#define INFINEON_BLOBS_ADDR        0x700\n')
    h.write(f'#define INFINEON_BLOBS_COMPRESSED  1\n')
    h.write(f'#define INFINEON_BLOBS_CNT         {files_cnt}\n\n')
    h.write(f'extern const T_infineon_blob infineon_blobs[INFINEON_BLOBS_CNT];\n\n') 

    h.write(f'const T_infineon_blob* _Find_infineon_blob(const char *name);\n\n')
    h.write('#endif\n')
    

    # Записать перечисление имен массивов и указателей на них
with open(output_c_file, 'w') as f:
    f.write('#include "App.h"\n\n')

    cnt = 0;
    for file, data in compressed_files_data.items():
        array_name = os.path.splitext(os.path.basename(file))[0]
        array_size = len(data)
        array_name = 'N%d_%s_data' % (cnt, array_name)
        f.write(f'const uint8_t {array_name}[{array_size}]   @ ".BLOBS" = {{\n')
        
        for i, byte in enumerate(data):
            if i % 12 == 0:
                f.write('\n    ')
            f.write(f'0x{byte:02x}, ')
        cnt = cnt + 1
        f.write('\n};\n\n')

    cnt = 0;
    f.write(f'__root T_infineon_blob const infineon_blobs[INFINEON_BLOBS_CNT] @ 0x700 = ')
    f.write('{\n')
    for file, data in compressed_files_data.items():
        file_name  = os.path.basename(file)
        array_name = os.path.splitext(os.path.basename(file))[0]
        array_size = len(data)
        full_size = full_size + array_size
        array_name = 'N%d_%s_data' % (cnt, array_name)
        file_name = '"%s"' % file_name
        formatted_line = "    {{{:<30}, {:<30}, {:>7}}},\n".format(file_name, array_name, array_size)
        f.write(formatted_line)
        cnt = cnt + 1
    f.write('};')    
    f.write(f'\n// Full size = {full_size}\n\n')


print(f'Файлы сжаты и сохранены в {output_c_file}')

