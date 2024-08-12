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
                     r'..\..\SDcard_content\www\index.html', 
                     r'..\..\SDcard_content\www\Logs.html', 
                     r'..\..\SDcard_content\www\Time_Sync.html',
                     r'..\..\SDcard_content\www\Upload.html',
                     r'..\..\SDcard_content\www\ModParams.html',
                     r'..\..\SDcard_content\www\WiFi_scan.html',
                     r'..\..\SDcard_content\www\Styles.css',
                     r'..\..\SDcard_content\www\GenScript.js'
                     ]
output_c_file = '..\..\src\web\WEB_files.c'
output_h_file = '..\..\src\web\WEB_files.h'

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
with open(output_h_file, 'w') as f:
    f.write('#ifndef _COMPRESSED_WEB_FILES_H\n')
    f.write('  #define _COMPRESSED_WEB_FILES_H\n\n')
    f.write('typedef struct {\n')
    f.write('    const char* name;\n')
    f.write('    const uint8_t* data;\n')
    f.write('    uint32_t size;\n')
    f.write('} T_compressed_file_rec ;\n\n')
    f.write(f'#define COMPRESSED_WEB_FILES_NUM {files_cnt}\n\n') 
    f.write(f'extern const T_compressed_file_rec compressed_web_files[COMPRESSED_WEB_FILES_NUM];\n\n') 
    f.write('#endif\n')
    

    # Записать перечисление имен массивов и указателей на них
with open(output_c_file, 'w') as f:
    f.write('#include "App.h"\n\n')

    cnt = 0;
    for file, data in compressed_files_data.items():
        array_name = os.path.splitext(os.path.basename(file))[0]
        array_size = len(data)
        array_name = 'N%d_%s_data' % (cnt, array_name)
        f.write(f'const uint8_t {array_name}[{array_size}] = {{\n')
        
        for i, byte in enumerate(data):
            if i % 12 == 0:
                f.write('\n    ')
            f.write(f'0x{byte:02x}, ')
        cnt = cnt + 1
        f.write('\n};\n\n')

    cnt = 0; 
    f.write(f'const T_compressed_file_rec compressed_web_files[COMPRESSED_WEB_FILES_NUM] = ')
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

